FROM python:3-slim-bookworm

RUN apt update && apt install -y curl
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/bin sh
RUN arduino-cli version
RUN arduino-cli core install esp32:esp32
RUN arduino-cli core install esp8266:esp8266 --additional-urls http://arduino.esp8266.com/stable/package_esp8266com_index.json

WORKDIR /src