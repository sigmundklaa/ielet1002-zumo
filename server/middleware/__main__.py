
import os
import sys
from .middleware import main
from dotenv import load_dotenv

load_dotenv()
main(mqtt_host=os.environ['mqtt_host'],
     mqtt_port=int(os.environ.get('mqtt_port', '1883')))
