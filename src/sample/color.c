/**
 * libws2812
 * Copyright (C) 2014 David Jolly
 * ----------------------
 *
 * libws2812 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libws2812 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include "../lib/ws2812.h"

enum {
	SECT_RED = 0,
	SECT_RED_GREEN,
	SECT_GREEN,
	SECT_GREEN_BLUE,
	SECT_BLUE,
	SECT_BLUE_RED,
};

#define SECT_MAX SECT_BLUE_RED

#define PIN_DATA 1 // PB1
#define PIN_POWER 0 // PB0
#define PORT_BANK D // PORTD

#define UPDATE_DELAY 5 // ms

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

int 
main(void)
{
	ws2812 cont;
	wserr_t result = WS_ERR_NONE;

	CLKPR = _BV(CLKPCE);
	CLKPR = 0;

	// initialize libary (one LED, power on)
	result = ws2812_init(&cont, PORT_BANK, PIN_POWER, PIN_DATA, 1, init_led, true, update_led);
	if(!WS_ERR_SUCCESS(result)) {
		goto exit;
	}

	for(;;) {
		_delay_ms(UPDATE_DELAY);

		// update LED color
		result = ws2812_update(&cont, update_led);
		if(!WS_ERR_SUCCESS(result)) {
			goto cleanup;
		}
	}

cleanup:

	// uninitialize libary
	result = ws2812_uninit(&cont);
	if(!WS_ERR_SUCCESS(result)) {
		goto exit;
	}

exit:
	return result;
}
