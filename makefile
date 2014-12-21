#
# libws2812
# Copyright (C) 2014 David Jolly
# ----------------------
#
# libws2812 is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# libws2812 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

CC=avr-gcc
EX0=blink
EX1=color
WS=ws2812

BIN=./bin/
BUILD=./build/

LIB=./src/lib/
SAMPLE=./src/sample/

# Change these to match your device
# NOTE: frequencies less than 8MHz might now work!
F_CPU=8000000
DEV=attiny85
DEV_SRT=t85

CC_FLG=-Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(DEV)

all: blink

# Blink command will attempt to build and flash blink sample
blink: clean blink_build flash_blink

blink_build:
	@echo ""
	@echo "============================================"
	@echo "BUILDING BLINK SAMPLE"
	@echo "============================================"
	mkdir $(BIN)
	mkdir $(BUILD)
	$(CC) $(CC_FLG) -c $(LIB)$(WS).c -o $(BUILD)$(WS).o -Wa,-ahl=$(BUILD)$(WS).s
	$(CC) $(CC_FLG) -c $(SAMPLE)$(EX0).c -o $(BUILD)$(EX0).o -Wa,-ahl=$(BUILD)$(EX0).s
	$(CC) $(CC_FLG) -o $(BUILD)$(EX0).elf $(BUILD)$(EX0).o $(BUILD)$(WS).o
	avr-objcopy -j .text -j .data -O ihex $(BUILD)$(EX0).elf $(BIN)$(EX0).hex
	avr-size --format=avr --mcu=$(DEV) $(BUILD)$(EX0).elf

# Color command will attempt to build and flash color sample
color: clean color_build flash_color

color_build:
	@echo ""
	@echo "============================================"
	@echo "BUILDING COLOR SAMPLE"
	@echo "============================================"
	mkdir $(BIN)
	mkdir $(BUILD)
	$(CC) $(CC_FLG) -c $(LIB)$(WS).c -o $(BUILD)$(WS).o -Wa,-ahl=$(BUILD)$(WS).s
	$(CC) $(CC_FLG) -c $(SAMPLE)$(EX1).c -o $(BUILD)$(EX1).o -Wa,-ahl=$(BUILD)$(EX1).s
	$(CC) $(CC_FLG) -o $(BUILD)$(EX1).elf $(BUILD)$(EX1).o $(BUILD)$(WS).o
	avr-objcopy -j .text -j .data -O ihex $(BUILD)$(EX1).elf $(BIN)$(EX1).hex
	avr-size --format=avr --mcu=$(DEV) $(BUILD)$(EX1).elf

clean:
	rm -rf $(BIN)
	rm -rf $(BUILD)

flash_blink:
	@echo ""
	@echo "============================================"
	@echo "FLASHING BLINK SAMPLE"
	@echo "============================================"
	avrdude -p $(DEV_SRT) -P usb -c usbtiny -U flash:w:$(BIN)$(EX0).hex

flash_color:
	@echo ""
	@echo "============================================"
	@echo "FLASHING COLOR SAMPLE"
	@echo "============================================"
	avrdude -p $(DEV_SRT) -P usb -c usbtiny -U flash:w:$(BIN)$(EX1).hex
