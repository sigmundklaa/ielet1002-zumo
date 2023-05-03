# ielet1002-zumo
Source code for the IELET1002 project of spring 2023

## Contributing
To contribute, enable auto-formatting before committing code.
This can be done by going to `Settings` > `Settings` > `Workspace` and then
searching for `Format on Save` and tick the checkbox.

## Directory structure
```
ielet1002-zumo
|- zumo # Contains the source code for the Zumo ATMega32U4 MCU
|- server # Contains the source code the webserver
|- esp32 # Contains the source code for the ESP32 sensors
|- charger # Contains the source code for the charging station (ESP32)
|- router # Contains the source code for the ESP-32 proxy/router onboard the Zumo
|- lib # Contains common functionality used across the different projects
|- rc # Contains source code for the remote controller
```