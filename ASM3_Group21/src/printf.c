#include "printf.h"
#include "../uart/uart.h"

#define MAX_PRINT_SIZE 256


void printf(char *string,...) {

	va_list ap;
	va_start(ap, string);

	char buffer[MAX_PRINT_SIZE];
	int buffer_index = 0;

	char temp_buffer[MAX_PRINT_SIZE];

	while(1) {
		if (*string == 0)
			break;

		if (*string == '%') {
			string++;

			int zero_pad = 0;
			if (*string == '0') {
				zero_pad = 1;
				string++;
			}

			// Parse width specifier
			int width = 0;
			while (*string >= '0' && *string <= '9') {
				width = width * 10 + (*string - '0');
				string++;
			}

			// Parse width specifier (either from format string or as an argument)
			if (*string == '*') {
				width = va_arg(ap, int);
				string++;
			} else {
				while (*string >= '0' && *string <= '9') {
					width = width * 10 + (*string - '0');
					string++;
				}
			}

			// Parse precision specifier (either from format string or as an argument)
			int precision = -1; // Default precision value
			if (*string == '.') {
				string++;
				if (*string == '*') {
			    	precision = va_arg(ap, int);
					string++;
				} else {
					precision = 0;
					while (*string >= '0' && *string <= '9') {
						precision = precision * 10 + (*string - '0');
						string++;
					}
				}
			}

			if (*string == 'd') {
				string++;
				int x = va_arg(ap, int);
				int temp_index = MAX_PRINT_SIZE - 1;
				int isNegative = 0;
				if (x < 0) {	
					x = -x;	
					isNegative = 1;
				}

				do {
					temp_buffer[temp_index] = (x % 10) + '0';
					temp_index--;
					x /= 10;
				} while(x != 0);
				
				// Print the negative sign if necessary
				if (isNegative) {
					temp_buffer[temp_index] = '-';
					temp_index--; 
				}

				// Determine the padding character
				char padding_char = zero_pad ? '0' : ' ';

				// Apply padding if necessary
				while (width > (MAX_PRINT_SIZE - temp_index - 1)) {
					temp_buffer[temp_index] = padding_char;
					temp_index--;
				}

				// Calculate the number of leading zeros needed for padding
				int num_zeros = precision - (MAX_PRINT_SIZE - temp_index - 1);
				if (num_zeros < 0) {
					num_zeros = 0;
				}	

				// Apply padding with leading zeros
				while (num_zeros > 0) {
					temp_buffer[temp_index] = '0';
					temp_index--;
					num_zeros--;
				}

				for(int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
					buffer[buffer_index] = temp_buffer[i];
					buffer_index++;
				}

			}
			else if (*string == 'x') {
				//WRITE YOUR CODE HERE FOR HEXA FORMAT
				//Then continue with other formats
                // Handle hexadecimal format specifier
                string++;
                unsigned int x = va_arg(ap, unsigned int);
                int temp_index = MAX_PRINT_SIZE - 1;

                do {
                    int remainder = x % 16;
                    temp_buffer[temp_index] = (remainder < 10) ? (remainder + '0') : (remainder - 10 + 'a');
                    temp_index--;
                    x /= 16;
                } while (x != 0);

				// Determine the padding character
				char padding_char = zero_pad ? '0' : ' ';

				// Apply padding if necessary
				while (width > (MAX_PRINT_SIZE - temp_index - 1)) {
					temp_buffer[temp_index] = padding_char;
					temp_index--;
				}

				// Calculate the number of leading zeros needed for padding
				int num_zeros = precision - (MAX_PRINT_SIZE - temp_index - 1);
				if (num_zeros < 0) {
					num_zeros = 0;
				}
				
				// Apply padding with leading zeros
				while (num_zeros > 0) {
					temp_buffer[temp_index] = '0';
					temp_index--;
					num_zeros--;
				}

                for (int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
                    buffer[buffer_index] = temp_buffer[i];
                    buffer_index++;
                }
			} else if (*string == 'c'){
				string++;
                char c = va_arg(ap, int); // char is promoted to int in varargs
				char padding_char = zero_pad ? '0' : ' ';
                // Handle flags and width
                for (int i = 0; i < width - 1; i++) {
					buffer[buffer_index] = padding_char;
					buffer_index++;
				}

                // Example: Format character
                buffer[buffer_index] = c;
                buffer_index++;
			} else if (*string == 'f'){
				string++;
				double float_num = va_arg(ap, double);
				// Determine the padding character
				char padding_char = zero_pad ? '0' : ' ';
				int Precision = (precision >= 0) ? precision : 6; // Default precision is 6
				if (float_num < 0) {
					buffer[buffer_index] = '-';
					buffer_index++;
					float_num = -float_num;
				}

				// Handle floating-point format specifier
				int int_part = (int)float_num;
				double fractional_part = float_num - int_part;

				// Convert the integer part to string
				int temp_index = MAX_PRINT_SIZE - 1;
				do {
					temp_buffer[temp_index] = (int_part % 10) + '0';
					temp_index--;
					int_part /= 10;
				} while (int_part != 0);

				// Apply padding if necessary (before the integer part)
				int padding_count = width - (MAX_PRINT_SIZE - temp_index - 1) - Precision - 1; // Calculate required padding
				while (padding_count > 0) {
					temp_buffer[temp_index] = padding_char;
					temp_index--;
					padding_count--;
				}

				for (int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
					buffer[buffer_index] = temp_buffer[i];
					buffer_index++;
				}
				
				// Add decimal point
				buffer[buffer_index] = '.';
				buffer_index++;

				// Convert the fractional part to string (up to the specified precision)
				for (int i = 0; i < Precision; i++) {
					fractional_part *= 10;
					int digit = (int)fractional_part;
					buffer[buffer_index] = digit + '0';
					buffer_index++;
					fractional_part -= digit;
				}

				// Round the last digit
				if (fractional_part >= 0.5) {
					buffer[buffer_index - 1]++;
				}
			} else if (*string == 's'){
				string++; 
				char *s = va_arg(ap, char *);
				int temp_index = 0;
				int str_length = 0;

				// Calculate the length of the string
				while (s[str_length] != '\0') {
					str_length++;
				}

				// // Determine the padding character
				char padding_char = zero_pad ? '0' : ' ';
				// Calculate the padding count
				// Apply padding if necessary
				// Calculate the padding count
				width = width - str_length;
				for (int i = 0; i < width; i++){
					temp_buffer[temp_index] = padding_char;
					temp_index++;
				}
				
				while (*s != '\0') {
					temp_buffer[temp_index] = *s;
					temp_index++;
                    s++;
                }

				for (int i = 0; i < temp_index; i++) {
                    buffer[buffer_index] = temp_buffer[i];
                    buffer_index++;
                }

			} else if (*string == '%'){
				// Handle percent sign
                buffer[buffer_index] = '%';
                buffer_index++;
                string++;
			} else {
			}
		}else {
			buffer[buffer_index] = *string;
			buffer_index++;
			string++;
		}

		if (buffer_index == MAX_PRINT_SIZE - 1)
			break;
	}
	
	va_end(ap);

	// Null-terminate the buffer
	buffer[buffer_index] = '\0';

	//Print out formated string
	uart_puts(buffer);
}
