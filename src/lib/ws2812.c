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
#include <avr/interrupt.h>
#include <string.h>
#include <util/delay.h>
#include "ws2812.h"

#define WS_DDR DDRB
#define WS_PIN_DATA PB1
#define WS_PIN_PWR PB0
#define WS_PORT PORTB
#define WS_PORT_ADDR _SFR_IO_ADDR(WS_PORT)

#define WS_PIN_DATA_MASK _BV(WS_PIN_DATA)
#define WS_PIN_DATA_MASK_HIGH (WS_PORT | WS_PIN_DATA_MASK)
#define WS_PIN_DATA_MASK_LOW (WS_PORT & ~WS_PIN_DATA_MASK)
#define WS_PIN_PWR_MASK _BV(WS_PIN_PWR)
#define WS_DDR_MASK (WS_PIN_DATA_MASK | WS_PIN_PWR_MASK)
#define WS_MSB_MASK 0x80

#define WS_DELAY_PWR_CHG 100.0 // us
#define WS_DELAY_RESET 50.0 // us

#define WS_BIT_CLR(_FLAG_, _BIT_) ((_FLAG_) &= ~(_BIT_))
#define WS_BIT_CHK(_FLAG_, _BIT_) ((_FLAG_) & (_BIT_))
#define WS_BIT_SET(_FLAG_, _BIT_) ((_FLAG_) |= (_BIT_))

wserr_t  
_ws2812_write(
	__in ws2812 *cont
	)
{
	uint16_t len;
	wserr_t result = WS_ERR_NONE;
	uint8_t bit, *data = NULL, data_ele, old_sreg;

	if(!cont || !WS_BIT_CHK(cont->status, WS_STATUS_INIT)
			|| !WS_BIT_CHK(cont->status, WS_STATUS_PWR)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}

	data = (uint8_t *) cont->ele;
	len = cont->count * sizeof(wscol_t);
	if(!data || !len) {
		result = WS_ERR_INV_DATA;
		goto exit;
	}

	old_sreg = SREG;
	cli();

	while(len--) {

		// write each byte to LED chain
		data_ele = *data++;
		bit = WS_CHAN_DEPTH;
		
		while(bit--) {
			WS_PORT = WS_PIN_DATA_MASK_HIGH;

			// TODO: perform proper waits

			if(!(data_ele & WS_MSB_MASK)) {
				WS_PORT = WS_PIN_DATA_MASK_LOW;

				// TODO: perform proper waits
			}

			data_ele <<= 1;
			WS_PORT = WS_PIN_DATA_MASK_LOW;
		}
	}

	// write reset
	WS_BIT_CLR(WS_PORT, WS_PIN_DATA_MASK);
	_delay_us(WS_DELAY_RESET);

	sei();
	SREG = old_sreg;

exit:
	return result;
}

wserr_t 
ws2812_init(
	__inout ws2812 *cont,
	__in uint16_t count,
	__in_opt wsinit_cb init,
	__in bool power,
	__in_opt wsupdate_cb update
	)
{
	uint16_t ele_iter = 0;
	wserr_t result = WS_ERR_NONE;

	if(!cont || !count || (count > WS_ELE_MAX_COUNT)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}

	if(WS_BIT_CHK(cont->status, WS_STATUS_INIT)) {
	
		// if context was previously initialized, uninitialize it
		result = ws2812_uninit(cont);
		if(!WS_ERR_SUCCESS(result)) {
			goto exit;
		}
	} else {
		memset(cont, 0, sizeof(ws2812));
	}

	WS_DDR = WS_DDR_MASK;
	WS_PORT = 0;
	cont->status = WS_STATUS_INIT;
	cont->count = count;

	if(init) {
	
		// if initialize callback is supplied, initialize all LEDs
		for(; ele_iter < cont->count; ++ele_iter) {
		
			result = init(&cont->ele[ele_iter], ele_iter);
			if(!WS_ERR_SUCCESS(result)) {
				goto exit;
			}
		}
	}

	if(power) {

		// if power on is requested, power on all LEDs
		result = ws2812_on(cont, update);
		if(!WS_ERR_SUCCESS(result)) {
			
			result = ws2812_uninit(cont);
			if(!WS_ERR_SUCCESS(result)) {
				goto exit;
			}
		}
	}

exit:
	return result;
}

wserr_t 
ws2812_off(
	__inout ws2812 *cont
	)
{
	wserr_t result = WS_ERR_NONE;

	if(!cont || !WS_BIT_CHK(cont->status, WS_STATUS_INIT)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}
	
	// confirm that power is not already off
	if(!WS_BIT_CHK(cont->status, WS_STATUS_PWR)) {
		result = WS_ERR_INV_STATE;
		goto exit;
	}
	
	WS_BIT_CLR(WS_PORT, WS_PIN_PWR_MASK);
	WS_BIT_CLR(cont->status, WS_STATUS_PWR);
	_delay_us(WS_DELAY_PWR_CHG);
	
exit:
	return result;
}
	
wserr_t 
ws2812_on(
	__inout ws2812 *cont,
	__in_opt wsupdate_cb update
	)
{
	wserr_t result = WS_ERR_NONE;

	if(!cont || !WS_BIT_CHK(cont->status, WS_STATUS_INIT)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}

	// confirm that power is not already on
	if(WS_BIT_CHK(cont->status, WS_STATUS_PWR)) {
		result = WS_ERR_INV_STATE;
		goto exit;
	}

	WS_BIT_SET(WS_PORT, WS_PIN_PWR_MASK);
	WS_BIT_SET(cont->status, WS_STATUS_PWR);
	_delay_us(WS_DELAY_PWR_CHG);

	// if update callback is supplied, update all LEDs
	result = ws2812_update(cont, update);
	if(!WS_ERR_SUCCESS(result)) {
		goto exit;
	}

exit:
	return result;
}

wserr_t 
ws2812_uninit(
	__inout ws2812 *cont
	)
{
	wserr_t result = WS_ERR_NONE;

	if(!cont || !WS_BIT_CHK(cont->status, WS_STATUS_INIT)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}

	if(WS_BIT_CHK(cont->status, WS_STATUS_PWR)) {
	
		// if power is on, power off all LEDs
		result = ws2812_off(cont);
		if(!WS_ERR_SUCCESS(result)) {
			goto exit;
		}
	}
	
	WS_PORT = 0;
	WS_DDR = 0;
	memset(cont, 0, sizeof(ws2812));

exit:
	return result;
}

wserr_t 
ws2812_update(
	__inout ws2812 *cont,
	__in_opt wsupdate_cb update
	)
{
	uint16_t ele_iter;
	wserr_t result = WS_ERR_NONE;

	if(!cont || !WS_BIT_CHK(cont->status, WS_STATUS_INIT)) {
		result = WS_ERR_INV_ARG;
		goto exit;
	}
	
	// confirm that power is not off
	if(!WS_BIT_CHK(cont->status, WS_STATUS_PWR)) {
		result = WS_ERR_INV_STATE;
		goto exit;
	}
	
	if(update) {
	
		// if update callback is supplied, update all LEDs
		for(ele_iter = 0; ele_iter < cont->count; ++ele_iter) {
		
			result = update(&cont->ele[ele_iter], ele_iter, cont->iter);
			if(!WS_ERR_SUCCESS(result)) {
				goto exit;
			}
		}
	}

	// write resulting colors to all LEDs
	result = _ws2812_write(cont);
	if(!WS_ERR_SUCCESS(result)) {
		goto exit;
	}

	++cont->iter;

exit:
	return result;
}
