
import os
import time
import json
import paho.mqtt.client as mqtt
from typing import Any
from dotenv import load_dotenv


def main(*args: Any) -> None:
    client = mqtt.Client()
    client.connect(os.environ['mqtt_host'],
                   int(os.environ.get('mqtt_port', '1883')))

    client.loop_start()

    while 1:
        client.publish('/devicemw/sync/1', json.dumps(
            {'x': 2, 'y': 3, 'z': 4, 'u': 5, 'w': 6, }
        ))
        time.sleep(1)


if __name__ == '__main__':
    import sys

    load_dotenv()
    main(*sys.argv[1:])
