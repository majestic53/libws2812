LIBWS2812
========

LIBWS2812 is a lightwight AVR library for controlling the WS2812 family of RGB LEDs.

Table of Contents
===============

1. [Changelog](https://github.com/majestic53/libws2812#changelog)
2. [Usage](https://github.com/majestic53/libws2812#usage)
3. [Architecture](https://github.com/majestic53/libws2812#architecture)
	* [Initialization](https://github.com/majestic53/libws2812#initialization)
	* [Update](https://github.com/majestic53/libws2812#update)
5. [Examples](https://github.com/majestic53/libws2812#examples)
	* [Blink](https://github.com/majestic53/libws2812#blink)
	* [Color](https://github.com/majestic53/libws2812#color)
6. [License](https://github.com/majestic53/libws2812#license)

Changelog
=========

###Version 0.1.1450r1
Updated: 12/13/2014

* Initial release
* Basic support for WS2812 family of RGB LEDs (tested with WS2812B on an attiny85 @ 8MHz)

Usage
=====

NOTE: You will need to install avr-gcc if you don't already have it.

To use LIBWS2812, compile the core library and sample files with the accompanying makefile. The make file will produce a series of object files 
(found in the ./build directory), and a series of hex files (found in the ./bin directory).

See the [Example](https://github.com/majestic53/libws2812#example) section below for the full example.

Architecture
===========

LIBWS2812 is a relatively simple library with only a handful of routines. Once the user has initialized the library, the user has the option to 
power on/off power to the LEDs, as well as update the LEDs colors. Writing new LED colors is handled by the library, and the user can update 
LED colors at any time, so long as the LEDs are powered on. See the state machine below, which describes the library operations in more detail.

![States](https://dl.dropboxusercontent.com/u/6410544/libws2812/states.png)

The user is also responsible for implemented a series of routines which handle LED initialization and updating. These routines will then be called 
by the library during operation.

###Initialization

This routine allows the user to specify the initial LED colors (per LED). This routine is only called during initialization.

```c
wserr_t initialize_led(
	__inout wscol_t *ele,	// LED element
	__in uint16_t ele_idx	// LED element index
	);
```

NOTE: The user is responsible for reporting any errors through the return error code.

###Update

This routine allows the user to specify the updated LED colors (per LED). This routine is called everytime the user asks the library to update 
the LED colors.

```c
wserr_t update_led(
	__inout wscol_t *ele,	// LED element
	__in uint16_t ele_idx,	// LED element index
	__in uint16_t iter	// update iteration
	);
```

NOTE: The user is responsible for reporting any errors through the return error code.

Examples
=======

###Blink

Blink is a simple example which cylces through the primary colors (red, green, blue).

```c
wserr_t 
init_led(
	__inout wscol_t *ele,
	__in uint16_t ele_idx
	)
{
	wserr_t result = WS_ERR_NONE;

	if(!ele || (ele_idx >= WS_ELE_MAX_COUNT)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}

	ele->red = UINT8_MAX;
	ele->green = 0;
	ele->blue = 0;

exit:
	return result;
}

wserr_t 
update_led(
	__inout wscol_t *ele,
	__in uint16_t ele_idx,
	__in uint16_t iter
	)
{
	wserr_t result = WS_ERR_NONE;

	if(!ele || (ele_idx >= WS_ELE_MAX_COUNT)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}

	if(ele->red == UINT8_MAX) {
		ele->red = 0;
		ele->green = UINT8_MAX;
	} else if(ele->green == UINT8_MAX) {
		ele->green = 0;
		ele->blue = UINT8_MAX;
	} else {
		ele->blue = 0;
		ele->red = UINT8_MAX;
	}

exit:
	return result;
}
```

This example can be build with a call to the makefile ```make blink```.

###Color

Color is an example which cylces through all supported colors (all 16581375 of them).

```c
enum {
	RED_SECT = 0,
	RED_GREEN_SECT,
	GREEN_SECT,
	GREEN_BLUE_SECT,
	BLUE_SECT,
	BLUE_RED_SECT,
};

#define SECT_MAX BLUE_RED_SECT

wserr_t 
init_led(
	__inout wscol_t *ele,
	__in uint16_t ele_idx
	)
{
	wserr_t result = WS_ERR_NONE;

	if(!ele || (ele_idx >= WS_ELE_MAX_COUNT)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}

	ele->red = UINT8_MAX;
	ele->green = 0;
	ele->blue = 0;

exit:
	return result;
}

wserr_t 
update_led(
	__inout wscol_t *ele,
	__in uint16_t ele_idx,
	__in uint16_t iter
	)
{
	uint16_t mode;
	wserr_t result = WS_ERR_NONE;

	if(!ele || (ele_idx >= WS_ELE_MAX_COUNT)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}

	mode = (iter % (UINT8_MAX * (SECT_MAX + 1))) / UINT8_MAX;

	switch(mode) { // 255, 0, 0
		case RED_SECT:

			if(ele->green < UINT8_MAX) {
				++ele->green;
			}
			break;
		case RED_GREEN_SECT: // 255, 255, 0

			if(ele->red) {
				--ele->red;
			}
			break;
		case GREEN_SECT: // 0, 255, 0

			if(ele->blue < UINT8_MAX) {
				++ele->blue;
			}
			break;
		case GREEN_BLUE_SECT: // 0, 255, 255

			if(ele->green) {
				--ele->green;
			}
			break;
		case BLUE_SECT: // 0, 0, 255

			if(ele->red < UINT8_MAX) {
				++ele->red;
			}
			break;
		case BLUE_RED_SECT: // 255, 0, 255

			if(ele->blue) {
				--ele->blue;
			}
			break;
	}

exit:
	return result;
}
```

This example can be build with a call to the makefile ```make color```.

License
======

Copyright(C) 2014 David Jolly <majestic53@gmail.com>

libws2812 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

libws2812 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
