

DOCKER_IMAGE=arduino-builder

DOCKER_CMD=docker run -it --mount src=${PWD},target=/src,type=bind --device=/dev/ttyUSB0 arduino-builder:latest
ARDUINO_CLI=$(DOCKER_CMD) arduino-cli



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
	$(ARDUINO_CLI) compile envy
	
upload:
	$(ARDUINO_CLI) upload envy