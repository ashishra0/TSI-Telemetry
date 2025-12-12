PORT = /dev/cu.usbserial-110
FQBN = esp32:esp32:esp32
SKETCH = firmware/tsi

compile:
	arduino-cli compile --fqbn $(FQBN) $(SKETCH)

upload:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN):UploadSpeed=115200 $(SKETCH)

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=115200

flash: compile upload

all: compile upload monitor

clean:
	rm -rf $(SKETCH)/build/

.PHONY: compile upload monitor flash all clean
