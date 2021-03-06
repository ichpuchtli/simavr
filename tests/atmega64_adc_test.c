/*
	atmega644_adc_test.c

	Copyright 2008, 2009 Michel Pollet <buserror@gmail.com>

 	This file is part of simavr.

	simavr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	simavr is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with simavr.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

/*
 * This demonstrate how to use the avr_mcu_section.h file
 * The macro adds a section to the ELF file with useful
 * information for the simulator
 */
#include "avr_mcu_section.h"
AVR_MCU(F_CPU, "atmega64");
AVR_MCU_VOLTAGES(3300, 3300, 3300);	// 3.3V VCC, AVCC, VREF

static int uart_putchar(char c, FILE *stream) {
  if (c == '\n')
    uart_putchar('\r', stream);
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
  return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
                                         _FDEV_SETUP_WRITE);

volatile uint16_t adc_val[8];

ISR(ADC_vect)
{
	uint8_t mux = ADMUX, l = ADCL, h = ADCH;
	uint8_t i = mux & 7;
	adc_val[i] = l | (h << 8);
	i = (i + 1) & 7;
	ADMUX = (mux & 0xF0) | i;
	if (i)
		ADCSRA |= (1 << ADSC);		// restart one now on the new channel
}

void adc_init()
{
	// set reference voltage
	ADMUX = (1 << REFS0) | (0 << ADLAR);	// use internal AVCC (3.3)

	// enable ADC, start a conversion, with the interrupt
	// and with a 128 clock prescaler
	ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADIE) | (1 << ADATE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

        // Free running mode
	ADCSRB = (0 << ADTS2) | (0 << ADTS1) | (1 << ADTS0);

}

int main(void)
{
	stdout = &mystdout;

	sei();
	printf("Read 8 ADC channels to test interrupts\n");

	adc_init();

	/*
	 * The interupt reads all 8 ADCs then stop...
	 * so this loop will eventually exits
	 */
	while (ADCSRA & (1 << ADSC)) sleep_cpu();

	printf("All done. Now reading the 1.1V value in pooling mode\n");

        // remove interrupt
	ADCSRA &= ~(1 << ADIE);

	// clear mux, select internal 1.22V reference voltage
	ADMUX = (ADMUX & ~0x1f) | 0x1e;

        // start conversion
	ADCSRA |= (1 << ADSC);

	while (ADCSRA & (1 << ADSC));

	uint16_t v = (ADCH << 8) | ADCL;

        // div 1024
	uint16_t volts = (v * 3300L) >> 10;

	printf("Read ADC value %04x = %d mvolts -- ought to be 1227\n", v, volts);

	ADCSRA &= ~(1 << ADEN);	// disable ADC...

	cli();

	sleep_cpu();

}

