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
		int caughtPercent = 0;
		if (*string == 0)
			break;

		if (*string == '%') {
			string++;

			if (*string == 'd') {
				string++;
				int x = va_arg(ap, int);
				int temp_index = MAX_PRINT_SIZE - 1;

				if (x < 0) {
					buffer[buffer_index] = '-';
					buffer_index++;
					x = -x;
				}

				do {
					temp_buffer[temp_index] = (x % 10) + '0';
					temp_index--;
					x /= 10;
				} while(x != 0);

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

                for (int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
                    buffer[buffer_index] = temp_buffer[i];
                    buffer_index++;
                }
			} else if (*string == 'c'){
				string++;
                char c = va_arg(ap, int); // char is promoted to int in varargs
                // Handle flags and width
                // ...

                // Example: Format character
                buffer[buffer_index] = c;
                buffer_index++;
			} else if (*string == 'f'){
				string++;
				double float_num = va_arg(ap, double);

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

				for (int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
					buffer[buffer_index] = temp_buffer[i];
					buffer_index++;
				}
				
				// Add decimal point
				buffer[buffer_index] = '.';
				buffer_index++;

				// Convert the fractional part to string (up to the specified precision)
				int precision = 4; // Adjust this for the desired precision
				for (int i = 0; i < precision; i++) {
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
                while (*s != '\0') {
                    buffer[buffer_index] = *s;
                    buffer_index++;
                    s++;
                }
			} else if (*string == '%'){
				// Handle percent sign
                buffer[buffer_index] = '%';
                buffer_index++;
                string++;
			} else {
				int width = 0;
				// THIS IS FOR THE INTERGER FORMAT SPECIFIER WHERE USER HAS WIDTH IMPLEMENT BUT ALSO ADD . INTO THEM
				int hasPrecision = 0;
				if (*string == '.') {
					hasPrecision = 1;
					string++;
				}
				// FIRST DECLARE THE FILL CHARACTER AND CHECK IF THE USER ENTERS 0 FLAG, THEN SET FLAG FOR IT
				char fill = ' ';
				if (*string == '0') {
					fill = '0';
					string++;
				}


				// ELSE IF ALL THE SITUATIONS ABOVE HAS PASTED, WHICH MEANS THE USER WANTS TO ADJUST WIDTH, PRECISION, OR 0 FLAG
				if (*string == '*'){
					width = va_arg(ap, int);
					string++;
				}
				else {
					// THIS IS THE WHILE LOOP TO CALCULATE THE SIZE WIDTH
                	while (*string >= '0' && *string <= '9') {
                    width = width * 10 + (*string - '0');
                    string++;
                }
				}



				// IF AFTER ALL OF THE CHECKINGS WITH FLAG, PRECISION, 0 FLAG, WE THEN GO CHECK IF ANY TYPES THEY WOULD LIKE TO PERFORM
				char specifier = *string;

				// IF THIS IS TYPE d OR D
				if (*string == 'd' || *string == 'D') {
					string++;
					int x = va_arg(ap, int);
					// WE CALCULATE THE RANGE OF VALUE
					int value = x;
					int indexcount = 0;
					do {
						value /= 10;
						indexcount++;
					} while(value != 0);
					// IF THE VALUE IS NEGATIVE, WE HAVE TO LEAVE SPACES FOR THE '-'
					if (x < 0 ) {
						indexcount++;
					}
					// IF VALUE IS 0, RETURN 0
					if (x == 0) {
						buffer[buffer_index] = '0';
						buffer_index++;
					}
					// IF THE PRECISION WAS PREVIOUSLY DETECTED, SET FILL  = 0
					if (hasPrecision == 1) {
						fill = '0';
					}
					// CHANGE WIDTH BASED ON THE INDEX HAS FOUND AND THE RANGE USER DECLARED TO GET FINAL VALUE
					width = width - indexcount;
					if (width < 0) {
						width = 0;
					}
					// FIRST, WE FILL WITH VARIABLE FIRST
					for (int i = 0; i < width; i++) {
						buffer[buffer_index] = fill;
						buffer_index++;
					}
					// THEN IF VALUE IS <0 , INSERT '-'
					if (x < 0) {
						buffer[buffer_index] = '-';
						buffer_index++;
						x = -x;
					}

					int temp_index = MAX_PRINT_SIZE - 1;


					// WE THEN COPY VARIABLES TO TEMP, AND THEN COPY THEM TO BUFFER
					do {
						temp_buffer[temp_index] = (x % 10) + '0';
						temp_index--;
						x /= 10;
					} while(x != 0);

					for(int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
						buffer[buffer_index] = temp_buffer[i];
						buffer_index++;
					}

				}

				// IF THIS IS TYPE OF x OR X
				else if (*string == 'x' || *string == 'X') {
					string++;
					int tempIndex = 0;
					int x = va_arg(ap, int);
					// RETURN 0 IF VALUE  = 0
					if (x == 0) {
						buffer[buffer_index] = '0';
						buffer_index++;
						return;
					}
					
					// THIS PART IS THE SAME AS THE ABOVE SITUATION FOR %X, BUT THIS ONE IS STORE THEM TO TEMP VARIABLES
					char hex_digits[] = "0123456789ABCDEF";
					char hex_string[8]; // Max 8 hexadecimal digits for uint32_t
					int index = sizeof(hex_string) - 1;
					if (x > 0) {
						while (x > 0 && index >= 0) {
							hex_string[index] = hex_digits[x % 16];
							x /= 16;
							index--;
						}
						// Print the hexadecimal string
						for (int i = index + 1; i < sizeof(hex_string); i++) {
							temp_buffer[tempIndex] = hex_string[i];
							tempIndex++;
						}
					}
					else {
						unsigned int n = x;
						while (n > 0 && index >= 0) {
							hex_string[index] = hex_digits[n % 16];
							n /= 16;
							index--;
						}
						// Print the hexadecimal string
						for (int i = index + 1; i < sizeof(hex_string); i++) {
							temp_buffer[tempIndex] = hex_string[i];
							tempIndex++;
						}
					}

					//CHECKING WITH PRECISION WIDTH, THEN FILL THE CHARACTERS FIRST, THEN COPY RIGHT VARIABLES INTO BUFFER
					if (hasPrecision == 1) {
					fill = '0';
					}
					width = width - tempIndex;
					if (width < 0) {
						width = 0;
					}
					for (int i = 0; i < width; i++) {
						buffer[buffer_index] = fill;
						buffer_index++;
					}
					for(int i = 0; i < tempIndex; i++) {
						buffer[buffer_index] = temp_buffer[i];
						buffer_index++;
					}

				}

				// IF THIS IS TYPE OF s OR S
				else if (*string == 's' || *string == 'S')
				{
					string++;
					char *str = va_arg(ap, char*);
					int temp_index = 0;
					//Looping through String to the end.
					// IF THE PRECISION IS NOT INCLUDED, THEN WE CAN FILL THE WIDTH WITH BLANKSPACES OR 0, THEN FILL THE STRING INTO BUFFER
					if (hasPrecision == 0) {
						while (*str != '\0')
						{
							//store each character of string into buffer array. 
							temp_buffer[temp_index] = *str;
							temp_index++;
							*str++;
						}
						width = width - temp_index;
						if (width < 0) {
							width = 0;
						}
						for (int i = 0; i < width; i++) {
						buffer[buffer_index] = fill;
						buffer_index++;
						}

						for (int i = 0; i < temp_index; i++) {
						buffer[buffer_index] = temp_buffer[i];
						buffer_index++;
						}
					}
					// ELSE THEN PRECISION IS INCLUDED, WHICH MEAN THAT THE WIDTH THAT USER DECLARED IS THE SIZE OF THE STRING, THEN ONLY THAT PORTION IS COPIED TO BUFFER
					else {
						while (width != temp_index)
						{
							//store each character of string into buffer array. 
							temp_buffer[temp_index] = *str;
							temp_index++;
							*str++;
						}
						for (int i = 0; i < width; i++) {
						buffer[buffer_index] = temp_buffer[i];
						buffer_index++;
						}
					

					}
				}
				// IF THIS IS TYPE c OR C
				else if (*string == 'c'|| *string == 'C')
				{
					// PRECISION DOES NOT DO ANYTHING WITH CHARACTER, SO DO THEM NORMALLY
					string++;
					char x = (char) va_arg(ap, int);
					// this is width -1 is for character
					for (int i = 0; i < width - 1; i++) {
						buffer[buffer_index] = fill;
						buffer_index++;
					}
					//Store character into buffer array. 
					buffer[buffer_index] = x;
					buffer_index++;
				}
				// IF THIS IS TYPE f OR F
				else if (*string == 'f'|| *string == 'F')  {
					string++;
					double value = va_arg(ap, double); // Get the float argument as a double


					// WIDTH AND PRECISION DOES NOT DO ANYTHING WITH FLOAT, SO DO THEM NORMALLY
					if (value < 0) {
						buffer[buffer_index] = '-';
						buffer_index++;
						value = -value;
					}

					// Split the number into integer and fractional parts
					int integer_part = (int)value;
					int temp_index = MAX_PRINT_SIZE - 1;

					do {
						temp_buffer[temp_index] = (integer_part % 10) + '0';
						temp_index--;
						integer_part /= 10;
					} while(integer_part != 0);

					for(int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
						buffer[buffer_index] = temp_buffer[i];
						buffer_index++;
					}

					double fractional_part = value - (int)value;
					// Print the decimal point
					buffer[buffer_index] = '.';
					buffer_index++;

					// Print up to 6 decimal places
					for (int i = 0; i < sizeof(fractional_part); i++) {
						fractional_part *= 10;
						int digit = (int)fractional_part;
						buffer[buffer_index] = '0' + digit;
						buffer_index++;
						fractional_part -= digit;
					}
				}
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
