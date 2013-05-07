#include "tests.h"

int main(int argc, char **argv) {
	tests_init(argc, argv);

	static const char *expected =
		"Read 8 ADC channels to test interrupts\r\n"
		"All done. Now reading the 1.1V value in pooling mode\r\n"
		"Read ADC value 017d = 1227 mvolts -- ought to be 1227\r\n";
	tests_assert_uart_receive("atmega64_adc_test.axf", 100000,
				  expected, '0');

	tests_success();
	return 0;
}
