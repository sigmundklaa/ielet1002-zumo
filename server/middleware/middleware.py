
import struct
import logging
import binascii
import json
import paho.mqtt.client as mqtt
from typing import TypedDict, Dict, Iterable, Union, Any
from pathlib import Path
from dataclasses import dataclass
from functools import cache

MQTT_HOST = ""
MQTT_PORT = 0


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


def _fmt(type_name: str) -> str:
    return _PACK_TYPES[type_name].fmt


@cache
def _build_topic(prefix: str, topic: str) -> str:
    """
    Build the topic that will be used as the Node-RED endpoint
    """
    def _prepend_slash(x):
        if not x.startswith('/'):
            return '/' + x

        return x

    return _prepend_slash(prefix) + _prepend_slash(topic)


@cache
def _build_header_fmt() -> str:
    """
    The header consists of:
    CRC32 checksum - 32bit

    This function also prepends the necessary specifiers for byte order, size,
    alignment etc.
    """
    return f'<{_fmt("u32")}'


@cache
def _build_pack_fmt(*entries: _config_entry) -> str:
    return ''.join(_fmt(x.kind) for x in entries)


@cache
def _build_fmt(*entries: _config_entry) -> str:
    return _build_header_fmt() + _build_pack_fmt(*entries)


def _unpack_payload(payload: Union[bytes, bytearray],
                    fmt: str
                    ) -> Iterable[Union[bytes, int, float]]:
    """
    Unpack the payload, and validate the CRC32 checksum
    """
    unpacked = struct.unpack(fmt, payload)
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

        self.client.on_message = self._on_message
        self.client.on_connect = self._on_connect

        self.client.connect(host, port, 60)

    def run(self) -> None:
        self.client.loop_forever()

    def _redirect_red(self, msg: mqtt.MQTTMessage) -> None:
        """
        Redirect a message from a device to the appropriate Node-RED topic
        """
        logging.info('Redirecting to red')

        topic = msg.topic.lstrip('/device')

        fields = self.config[topic].fields
        data = _unpack_payload(msg.payload, _build_fmt(*fields))

        if data == None:
            logging.info('Validation failed, discarding message')
            return

        self.client.publish(_build_topic('/red', topic), json.dumps({
            field.name: data[idx]
            for idx, field in enumerate(fields)
        }))

    def _redirect_device(self, msg: mqtt.MQTTMessage) -> None:
        """
        Redirect a message from the Node-RED instance to the appropriate device
        topic
        """
        logging.info('Redirecting to device')

        data = json.loads(msg.payload)
        config = self.config['device'][msg.topic]

        data_struct = [data[field.name] for field in config]
        packed_tmp = struct.pack(
            _build_pack_fmt(_build_pack_fmt(*config)), data_struct)

        checksum = binascii.crc32(packed_tmp)
        data_struct = [checksum] + data_struct

        self.client.publish(_build_topic('/device', msg.topic),
                            struct.pack(_build_fmt(*config), data_struct))

    def _on_message(self, client: mqtt.Client, msg: mqtt.MQTTMessage) -> None:
        logging.info(f'Message recieved: {msg}')

        if msg.topic.startswith('/device'):
            self._redirect_device(msg)
        else:
            self._redirect_red(msg)

    def _on_connect(self, *args: Any) -> None:
        logging.info(f'Connected')

        # Subscribe to topics for Node-RED -> Device direction
        for dtopic in self.config['device'].keys():
            self.client.subscribe(_build_topic('/device', dtopic))

        # Subscribe to topic for Device -> Node-RED direction. This is not
        # prepended by a particular endpoint like /device.
        for rtopic in self.config['red'].keys():
            self.client.subscribe(rtopic)


def main(*args) -> None:
    logging.info('Initializing middleware')

    with open(Path.cwd().joinpath('topics.json')) as fp:
        config = json.load(fp)

    handler = _handler(mqtt.Client(), config, MQTT_HOST, MQTT_PORT)
    handler.run()