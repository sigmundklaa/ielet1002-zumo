
import re
import struct
import logging
import binascii
import json
import paho.mqtt.client as mqtt
from typing import TypedDict, Dict, Iterable, Union, Any
from pathlib import Path
from dataclasses import dataclass
from functools import cache

logging.basicConfig(level=logging.DEBUG)


@dataclass
class _pack_type:
    fmt: str
    size: int

    def __init__(self, fmt):
        self.fmt = fmt
        self.size = struct.calcsize(fmt)


class _config_entry(TypedDict):
    name: str
    kind: str


_config = Dict[str, Dict[str, Iterable[_config_entry]]]


_PACK_TYPES = {
    'char': _pack_type('c'),
    'i8': _pack_type('b'),
    'u8': _pack_type('B'),
    'i16': _pack_type('h'),
    'u16': _pack_type('H'),
    'i32': _pack_type('i'),
    'u32': _pack_type('I'),
    'i64': _pack_type('q'),
    'u64': _pack_type('Q'),
    'f32': _pack_type('f'),
}

_USE_CRC = False


def _fmt(type_name: str) -> str:
    return _PACK_TYPES[type_name].fmt


@cache
def _build_topic(*parts: str) -> str:
    """
    Build the topic that will be used as the Node-RED endpoint
    """
    def _prepend_slash(x):
        if not x.startswith('/'):
            return '/' + x

        return x

    return ''.join(_prepend_slash(x) for x in parts)


@cache
def _build_header_fmt() -> str:
    """
    The header consists of:
    CRC32 checksum - 32bit

    This function also prepends the necessary specifiers for byte order, size,
    alignment etc.
    """
    if not _USE_CRC:
        return '<'

    return f'<{_fmt("u32")}'


def _build_pack_fmt(*entries: _config_entry) -> str:
    return ''.join(_fmt(x['kind']) for x in entries)


def _build_fmt(*entries: _config_entry) -> str:
    return _build_header_fmt() + _build_pack_fmt(*entries)


def _unpack_payload(payload: Union[bytes, bytearray],
                    fmt: str
                    ) -> Iterable[Union[bytes, int, float]]:
    """
    Unpack the payload, and validate the CRC32 checksum
    """
    try:
        unpacked = struct.unpack(fmt, payload)
    except struct.error as e:
        logging.error(
            f'Unable to decode struct: {str(e)} (fmt: {fmt}, payload: {payload})')
        return None

    if not _USE_CRC:
        return unpacked

    checksum = unpacked[0]

    # Checksum is calculated without the header
    computed_crc = binascii.crc32(payload[_PACK_TYPES['u32'].size:])

    if checksum != computed_crc:
        logging.error(
            f'Invalid checksum: got {checksum}, expected {computed_crc}')

        return None

    return unpacked[1:]


class _handler:
    client: mqtt.Client
    config: _config

    def __init__(self,
                 client: mqtt.Client,
                 config: _config,
                 host: str,
                 port: int
                 ):
        logging.info('Initializing MQTT client')

        self.client = client
        self.config = config
        self.host = host
        self.port = port

        self.client.on_message = self._on_message
        self.client.on_connect = self._on_connect

        self.client.connect(host, port, 60)

    def run(self) -> None:
        self.client.loop_forever()

    def _remote_log(self, msg: mqtt.MQTTMessage) -> None:
        fmt = _build_header_fmt()
        size = struct.calcsize(fmt)

        header = msg.payload[:size]
        logstr = msg.payload[size:]

        if _USE_CRC:
            crc = struct.unpack(fmt, header)
            computed_crc = binascii.crc32(logstr)

            if crc != computed_crc:
                logging.warn(f'Bad remote log crc')

        with open(f'./logs/{msg.topic.removeprefix("/log/")}.txt', 'a') as fp:
            fp.write(logstr.decode('utf-8'))

    def _redirect_red(self, msg: mqtt.MQTTMessage) -> None:
        """
        Redirect a message from a device to the appropriate Node-RED topic
        """
        logging.info('Redirecting to red')

        topic = msg.topic.removeprefix('/redmw').removeprefix('/')

        fields = [_config_entry(x) for x in self.config['red'][topic]]
        data = _unpack_payload(msg.payload, _build_fmt(*fields))

        if data == None:
            logging.error('Validation failed, discarding message')
            return

        self.client.publish(_build_topic('/red', topic), json.dumps({
            field['name']: data[idx]
            for idx, field in enumerate(fields)
        }))

    def _redirect_device(self, msg: mqtt.MQTTMessage) -> None:
        """
        Redirect a message from the Node-RED instance to the appropriate device
        topic
        """
        logging.info('Redirecting to device')

        data = json.loads(msg.payload)
        topic = msg.topic.removeprefix('/devicemw').removeprefix('/')
        config = self.config['devicemw'][topic]

        data_struct = [data[field['name']] for field in config]
        packed_tmp = struct.pack(_build_pack_fmt(*config), *data_struct)

        if _USE_CRC:
            checksum = binascii.crc32(packed_tmp)
            data_struct = [checksum] + data_struct

        self.client.publish(_build_topic('/device', topic),
                            struct.pack(_build_fmt(*config), *data_struct))

    def _on_message(self, client: mqtt.Client, usrdata: Any, msg: mqtt.MQTTMessage) -> None:
        logging.info(f'Message recieved: {str(msg.payload)}')

        if msg.topic.startswith('/devicemw'):
            self._redirect_device(msg)
        elif msg.topic.startswith('/redmw'):
            self._redirect_red(msg)
        elif msg.topic.startswith('/red'):
            logging.info(
                f'Red recieved: {json.dumps(json.loads(msg.payload.decode("utf-8")), indent=4)}')
        elif msg.topic.startswith('/log'):
            self._remote_log(msg)
        else:
            logging.error(f'Invalid topic {msg.topic}')

    def _subscribe(self, topic: str):
        self.client.subscribe(topic)
        logging.info(f'Subscribed to {topic}')

    def _on_connect(self, *args: Any) -> None:
        logging.info(f'Connected to {self.host}:{self.port}')

        # Subscribe to topics for Node-RED -> Device direction
        for dtopic in self.config['devicemw'].keys():
            self._subscribe(_build_topic('/devicemw', dtopic))

        # Subscribe to topic for Device -> Node-RED direction.
        for rtopic in self.config['red'].keys():
            self._subscribe(_build_topic('redmw', rtopic))
            self._subscribe(_build_topic('red', rtopic))

        for ltopic in self.config['log'].keys():
            self._subscribe(_build_topic('log', ltopic))


def main(**kwargs) -> None:
    logging.info('Initializing middleware')

    with open(Path.cwd().joinpath('topics.json')) as fp:
        config = json.load(fp)

    handler = _handler(mqtt.Client(), config,
                       kwargs['mqtt_host'], kwargs.get('mqtt_port', 1883))
    handler.run()
