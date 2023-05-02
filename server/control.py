
import os
import time
import json
import paho.mqtt.client as mqtt
from typing import Any
from dotenv import load_dotenv


def send_(client: mqtt.Client, c: int, a1: int, a2: int) -> None:
    client.publish('/devicemw/control/1', json.dumps({
        'cmd': c, 'arg1': a1, 'arg2': a2
    }))


def main(*args: Any) -> None:
    client = mqtt.Client()
    client.connect(os.environ['mqtt_host'],
                   int(os.environ.get('mqtt_port', '1883')))

    client.loop_start()

    send_(client, 1, 0, 0)
    time.sleep(1)
    send_(client, 0, 200, 200)
    time.sleep(5)
    send_(client, 1, 1, 0)
    time.sleep(5)


if __name__ == '__main__':
    import sys

    load_dotenv()
    main(*sys.argv[1:])
