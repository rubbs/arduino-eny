

DOCKER_IMAGE=arduino-builder

DOCKER_CMD=docker run -it --mount src=${PWD},target=/src,type=bind  arduino-builder:latest
ARDUINO_CLI=$(DOCKER_CMD) arduino-cli
BOARD=esp32:esp32:nodemcu-32s
BOARD=esp8266:esp8266:nodemcu
PORT=abc

buildenv:
	docker build -t arduino-builder .

info: buildenv
	$(ARDUINO_CLI) board list
	$(ARDUINO_CLI) core list

devshell: buildenv
	$(DOCKER_CMD) /bin/bash

compile: 
	ls -l
	pwd
	$(ARDUINO_CLI) compile arduino-envy
	
upload:
	$(ARDUINO_CLI) upload $(BOARD) -p $(PORT)