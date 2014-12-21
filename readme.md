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

###Version 0.1.1451r1
*Updated: 12/20/2014*

* Added port/pin agnostic initialization macro (you can now define your own power/data pins!!!)

###Version 0.1.1450r1
*Updated: 12/13/2014*

* Initial release
* Basic support for WS2812 family of RGB LEDs (tested with WS2812B on an attiny85 @ 8MHz)

![Wiring](https://dl.dropboxusercontent.com/u/6410544/libws2812/wiring.jpg)

Usage
=====

**NOTE:** You will need to install avr-gcc if you don't already have it.

To use LIBWS2812, simply include the header file ```ws2812.h```, found under ```./src/lib```, in your project.

To use the sample projects, compile the core library using the accompanying makefile. The makefile will produce a series of 
object files (found in the ./build directory), and a series of hex files (found in the ./bin directory).

See the [Examples](https://github.com/majestic53/libws2812#examples) section below for the full example.

Architecture
===========

LIBWS2812 is a relatively simple library with only a handful of routines. Once the library has been initialized, you have the option to 
power on/off the LEDs, as well as update the LEDs colors. Writing new LED colors is handled by the library, and you can update 
LED colors at any time so long as the LEDs are powered on. See the state machine below, which describes the library operations in more detail:

![States](https://dl.dropboxusercontent.com/u/6410544/libws2812/states.png)

**NOTE:** You are responsible for implementing the callback routines which handle LED initialization and updating. If callback routines are not
supplied during initialization, the LEDs will be initialized to a default color.

###Initialization

This routine allows the user to specify the initial LED colors (per LED). This routine is only called during initialization.

```c
wserr_t initialize_led(
	__inout wscol_t *ele,	// LED element
	__in uint16_t ele_idx	// LED element index
	);
```

**NOTE:** The user is responsible for reporting any errors through the return error code. Typically, this routine should return 
with ```WS_ERR_NONE```.

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

**NOTE:** The user is responsible for reporting any errors through the return error code. Typically, this routine should return 
with ```WS_ERR_NONE```.

Examples
=======

###Blink

Blink is a simple example which cylces through the primary colors (red, green, blue).

```c
/*
 * Initialization callback routine
 * Called during initialization for all LEDs
 * @param ele current LED
 * @param ele_idx current LED index
 * @return WS_ERR_NONE on success
 */
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

	// initialize blue
	ele->red = 0;
	ele->green = 0;
	ele->blue = UINT8_MAX;

exit:
	return result;
}

/*
 * Update callback routine
 * Called during update for all LEDs
 * @param ele current LED
 * @param ele_idx current LED index
 * @param iter current update iteration
 * @return WS_ERR_NONE on success
 */
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

	if(ele->blue == UINT8_MAX) { // red
		ele->blue = 0;
		ele->red = UINT8_MAX;
	} else if(ele->red == UINT8_MAX) { // green
		ele->red = 0;
		ele->green = UINT8_MAX;
	} else { // blue
		ele->green = 0;
		ele->blue = UINT8_MAX;
	}

exit:
	return result;
}
```

This example can be build with a call to the makefile ```make blink``` (might require ```sudo``` to write to your programmer).

###Color

Color is an example which cylces through all supported colors (all 16,581,375 of them).

```c
enum {
	SECT_RED = 0,
	SECT_RED_GREEN,
	SECT_GREEN,
	SECT_GREEN_BLUE,
	SECT_BLUE,
	SECT_BLUE_RED,
};

#define SECT_MAX SECT_BLUE_RED

/*
 * Initialization callback routine
 * Called during initialization for all LEDs
 * @param ele current LED
 * @param ele_idx current LED index
 * @return WS_ERR_NONE on success
 */
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

	// initialize red
	ele->red = UINT8_MAX;
	ele->green = 0;
	ele->blue = 0;

exit:
	return result;
}

/*
 * Update callback routine
 * Called during update for all LEDs
 * @param ele current LED
 * @param ele_idx current LED index
 * @param iter current update iteration
 * @return WS_ERR_NONE on success
 */
wserr_t 
update_led(
	__inout wscol_t *ele,
	__in uint16_t ele_idx,
	__in uint16_t iter
	)
{
	uint16_t sect;
	wserr_t result = WS_ERR_NONE;

	if(!ele || (ele_idx >= WS_ELE_MAX_COUNT)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}

	sect = (iter % (UINT8_MAX * (SECT_MAX + 1))) / UINT8_MAX;

	switch(sect) { // 255, 0, 0
		case SECT_RED:

			if(ele->green < UINT8_MAX) {
				++ele->green;
			}
			break;
		case SECT_RED_GREEN: // 255, 255, 0

			if(ele->red) {
				--ele->red;
			}
			break;
		case SECT_GREEN: // 0, 255, 0

			if(ele->blue < UINT8_MAX) {
				++ele->blue;
			}
			break;
		case SECT_GREEN_BLUE: // 0, 255, 255

			if(ele->green) {
				--ele->green;
			}
			break;
		case SECT_BLUE: // 0, 0, 255

			if(ele->red < UINT8_MAX) {
				++ele->red;
			}
			break;
		case SECT_BLUE_RED: // 255, 0, 255

			if(ele->blue) {
				--ele->blue;
			}
			break;
		default:
			result = WS_ERR_INV_STATE;
			goto exit;
	}

exit:
	return result;
}
```

This example can be build with a call to the makefile ```make color``` (might require ```sudo``` to write to your programmer).

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
