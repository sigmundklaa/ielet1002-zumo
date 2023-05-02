
BROKER_EXE=mosquitto
BROKER_FLAGS=-v -c ./mosquitto.conf

PY=python3

.PHONY: broker
broker: mosquitto.conf
	$(BROKER_EXE) $(BROKER_FLAGS)

.PHONY: middleware
middleware:
	$(PY) -m middleware
