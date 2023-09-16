#include "../uart/uart.h"
#include "printf.h"
#include "mbox.h"
#include "framebf.h"
#include "image.h"
#include "video.h"
#include "timer.h"
#include "Maze.h"
#include "gameElement.h"
#include "Frontier.c"
#define MAX_CMD_SIZE 100
#define MAX_TOKENS 100
#define HISTORY_SIZE 10
#define MAX_REQ_VALUE 10
int widthScreen = 40;
int heightScreen = 20;
char history[HISTORY_SIZE][MAX_CMD_SIZE];
int history_count = 0;
int current_history_index = 0;
char *maze;
static unsigned char our_memory[1024 * 1024]; //reserve 1 MB for malloc
static size_t next_index = 0;
int inGame = 0;
Frontier *myFrontier;

int x_direct = 20;
int y_direct = 0;


void *malloc(size_t sz)
{
    void *mem;

    if(sizeof our_memory - next_index < sz)
        return NULL;

    mem = &our_memory[next_index];
    next_index += sz;
    return mem;
}

void free(void *mem)
{
   //we cheat, and don't free anything.
}

// Function draw image
void draw_image()
{
    // Looping through image array line by line.
    for (int j = 0; j < 425; j++)
    {
        // Looping through image array pixel by pixel of line j.
        for (int i = 0; i < 307; i++)
        {
            // Printing each pixel in correct order of the array and lines, columns.
            drawPixelARGB32(i, j, image[j * 307 + i]);
        }
    }
}

void getNearFrontier(const char *maze, int x, int y) {
    if (y * widthScreen + x - widthScreen < 0) {
        myFrontier->north = 10;
    }
    else {    myFrontier->north = maze[y * widthScreen + x - widthScreen];
    }
    if (y * widthScreen + x + 1 > ((y + 1) * widthScreen)) {
        myFrontier->east = 10;
    }
    else {    myFrontier->east = maze[y * widthScreen + x + 1];
    }
    if (y*widthScreen + x + widthScreen > widthScreen * heightScreen) {
        myFrontier->south = 10;
    }
    else {    myFrontier->south = maze[y*widthScreen + x + widthScreen];
    }
    if (y * widthScreen + x - 1 < y * widthScreen) {
        myFrontier->west = 10;
    }
    else {    myFrontier->west = maze[y * widthScreen + x - 1];
    }
}

int checkDirection(int dir) {
    switch (dir)
    {
    case 3:
        if (myFrontier->north == 0) {
            return 1;
        }
        if (myFrontier->north == 2) {
            return 2;
        }
        break;
    case 4:
        if (myFrontier->east == 0) {
            return 1;
        }
        if (myFrontier->east == 2) {
            return 2;
        }
        break;
    case 5:
        if (myFrontier->south == 0) {
            return 1;
        }
        if (myFrontier->south == 2) {
            return 2;
        }
        break;
    case 6:
        if (myFrontier->west == 0) {
            return 1;
        }
        if (myFrontier->west == 2) {
            return 2;
        }
        break;
    default:
        break;
    }
    return 0;
}

void drawMap(const char *maze, int widthScreen, int heightScreen) {
   int x, y;
   for(y = 0; y < heightScreen; y++) {
      for(x = 0; x < widthScreen; x++) {
         switch(maze[y * widthScreen + x]) {
         case 1:  draw_wall(x * 20, y * 20);  break;
         case 2:  draw_destination(x * 20,  y * 20);  break;
         }
      }
   }
   for (int x = 0; x < widthScreen; x++) {
        draw_wall(x * 20, heightScreen * 20);
   }
   for (int y = 0; y < heightScreen; y++) {
        draw_wall(widthScreen * 20, y * 20);
   }
   while (1) {
    int var = rand_range(0, widthScreen * heightScreen / 2);
    if (maze[var] == 0) {
        int y_index = var / widthScreen;
        int x_index = var % widthScreen;
        printf("This is x_index: %d\n", x_index);
        printf("This is y_index: %d\n", y_index);
        draw_destination(x_index * 20, y_index * 20);
        x_direct = x_index * 20;
        y_direct = y_index * 20;
        getNearFrontier(maze, x_index, y_index);
        break;
    }
   }
}


void draw_wall(int x, int y) {
    for (int j = 0; j < 20; j++) {
        for (int i = 0; i < 20; i++) {
            drawPixelARGB32(i + x, j + y, epd_bitmap_wall[j*20 +i]);
        }
    }
}

void draw_destination(int x, int y) {
    for (int j = 0; j < 20; j++) {
        for (int i = 0; i < 21; i++) {
            drawPixelARGB32(i + x, j + y, epd_bitmap_destination[j*21 +i]);
        }
    }
}

void draw_video() {
    for (int a = 0; a < epd_bitmap_allArray_LEN; a++) {
        for (int j = 0; j < 240; j++) {
            for (int i = 0; i < 426; i++) {
                drawPixelARGB32(i, j, epd_bitmap_allArray[a][j * 426 + i]);
            }
        }
        wait_msec(40000);
    }
}

const char *commands[] = {
    "help", "clear", "setcolor", "showinfo", "video", "smallimg", "game"
    // Add more commands as needed
};

char *strcpy(char *dest, const char *src) {
    char *originalDest = dest;
    
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    
    *dest = '\0'; // Add the null-terminator at the end
    
    return originalDest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *originalDest = dest;

    size_t i;
    for (i = 0; i < n && *src != '\0'; i++) {
        *dest = *src;
        dest++;
        src++;
    }

    // Fill the remaining characters with null terminators if necessary
    for (; i < n; i++) {
        *dest = '\0';
        dest++;
    }

    return originalDest;
}

char* strtok(char* str, const char* delimiter, char** context) {
    if (str != NULL)
        *context = str;

    if (*context == NULL || **context == '\0')
        return NULL;

    char* token_start = *context;
    while (**context != '\0') {
        int is_delimiter = 0;
        for (size_t i = 0; delimiter[i] != '\0'; i++) {
            if (**context == delimiter[i]) {
                is_delimiter = 1;
                break;
            }
        }

        if (is_delimiter) {
            **context = '\0';
            (*context)++;
            if (token_start != *context)
                return token_start;
        } else {
            (*context)++;
            if (**context == '\0') {
                return token_start;
            }
        }
    }

    return token_start;
}

// String length Function 
size_t strlen(const char* str) {
    size_t length = 0;
    while (*str != '\0') {
        length++;
        str++;
    }
    return length;
}

// String Compare Function
int strcmp(const char* str1, const char* str2) {
    while (*str1 != '\0' && *str2 != '\0') {
        if (*str1 != *str2) {
            return (*str1 > *str2) ? 1 : -1;
        }
        str1++;
        str2++;
    }

    if (*str1 == '\0' && *str2 == '\0') {
        return 0;
    } else if (*str1 == '\0') {
        return -1;
    } else {
        return 1;
    }
}

void help_command(const char *cmd) {
    // Implement help command logic here
    // Print the information about the supported commands
    // ...
    uart_puts("Available commands:\n");
    uart_puts("For more information on a specific command, type help <command-name>:\n");
    uart_puts("help                                 Show brief information of all commands\n");
    uart_puts("help <command_name>                  Show full information of the command\n");
    uart_puts("clear                                Clear screen\n");
    uart_puts("setcolor                             Set text color, and/or background color of the console to one of the following colors: BLACK, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE\n");
    uart_puts("showinfo                             Show board revision and board MAC address\n");
}

void help_info(const char *cmd){
    if (strcmp(cmd, "setcolor") == 0) {
        uart_puts("Set text color only:                         setcolor -t <color>.\n");
        uart_puts("Set background color only:                   setcolor -b <color>.\n");
        uart_puts("Set color for both background and text:      setcolor -t <color> -b <color or setcolor -b <color> -t<color>.\n");
        uart_puts("Accepted color and writing format:  Black,  Red, Green, Yellow, Blue, Purple, Cyan, White.\n");
        uart_puts("Examples\nMyBareMetalOS> setcolor -t yellow\nMyBareMetalOS> setcolor -b yellow -t white\n");
    } else if (strcmp(cmd, "clear") == 0) {
        uart_puts("Clear screen (in our terminal it will scroll down to current position of the cursor).\n");
        uart_puts("Example: MyBareMetalOS> clear\n");
    } else if (strcmp(cmd, "showinfo") == 0) {
        uart_puts("Show board revision and board MAC address in correct format/ meaningful information.\n");
        uart_puts("Example: MyBareMetalOS> showinfo\n");
    } else {
        uart_puts("Unrecognized command\n");
    } 
}

void showinfo() {
    mBuf[0] = 11*4; // Message Buffer Size in bytes (8 elements * 4 bytes (32 bit) each)
    mBuf[1] = MBOX_REQUEST; // Message Request Code (this is a request message)

    mBuf[2] = 0x00010002; // TAG Identifier: Get Board Revision
    mBuf[3] = 4; // Value buffer size in bytes (max of request and response lengths)
    mBuf[4] = 0; // REQUEST CODE = 0
    mBuf[5] = 0; // clear output buffer (response data are mBuf[10])
    
    mBuf[6] = 0x00010003; // TAG Identifier: Get Board Mac Address 
    mBuf[7] = 6; // Value buffer size in bytes (max of request and response lengths)
    mBuf[8] = 0; // REQUEST CODE = 0
    mBuf[9] = 0; // clear output buffer (response data are mBuf[9])
    mBuf[10] = 0; 
    mBuf[11] = MBOX_TAG_LAST;

    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        uart_puts("\nDATA: Board Revision = ");
        uart_hex(mBuf[5]);
        uart_puts("\nBoard MAC Address: ");
        //printf("%x", mBuf[10]);
        // printf("%x", mBuf[9]);
        printf("%02x:%02x:%02x:%02x:%02x:%02x",
        (mBuf[10] >> 8) & 0xFF, mBuf[10] & 0xFF,(mBuf[9] >> 24) & 0xFF,
        (mBuf[9] >> 16) & 0xFF, (mBuf[9] >> 8) & 0xFF, mBuf[9] & 0xFF);
        uart_puts("\n");
    } else {
        uart_puts("Unable to query!\n");
    }
}

int strncmp(const char *str1, const char *str2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (str1[i] != str2[i]) {
            return (unsigned char)str1[i] - (unsigned char)str2[i];
        }
        if (str1[i] == '\0') {
            return 0; // Reached end of one or both strings
        }
    }
    return 0; // Both strings are equal up to n characters
}

void clear_command() {
    uart_puts("\x1B[2J\x1B[H");
}

void deleteCommand(char *cmd_buffer, int *index) {
    if (*index > 0) {
        (*index)--;
        cmd_buffer[*index] = '\0';
        uart_puts("\b \b"); // Move the cursor back, write a space to clear the character, and move the cursor back again
    } 
}

char *strcat(char *dest, const char *src) {
    char *originalDest = dest;

    // Move the destination pointer to the end of the string
    while (*dest != '\0') {
        dest++;
    }

    // Copy the source string to the end of the destination string
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    // Add the null-terminator at the end
    *dest = '\0';

    return originalDest;
}

const char *colorOptions[] = {
    "BLACK", "RED", "GREEN", "YELLOW", "BLUE", "PURPLE", "CYAN", "WHITE"
};

const char *textColorCode[] = {
    "\033[30m", "\033[31m", "\033[32m", "\033[33m",
    "\033[34m", "\033[35m", "\033[36m", "\033[37m"
};

const char *backgroundColorCode[] = {
    "\033[40m", "\033[41m", "\033[42m", "\033[43m",
    "\033[44m", "\033[45m", "\033[46m", "\033[47m"
};

void setBackGroundColor(const char *backgroundArray){
    int backgroundColorIndex = -1;
    // If backgroundArray is provided, find the index of the background color
    if (backgroundArray != NULL) {
        for (int i = 0; i < sizeof(colorOptions) / sizeof(colorOptions[0]); i++) {
            if (strcmp(backgroundArray, colorOptions[i]) == 0) {
                backgroundColorIndex = i;
                break;
            }
        }
    }

    if (backgroundColorIndex != -1) {
        uart_puts((char*)backgroundColorCode[backgroundColorIndex]);
    } else {
        uart_puts("Invalid background color. Supported colors: BLACK, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE.\n");
    }
}

void setTextColor(const char *textColor) {
    int textColorIndex = -1;

    // If textColor is provided, find the index of the text color in the colorOptions array
    if (textColor != NULL) {
        for (int i = 0; i < sizeof(colorOptions) / sizeof(colorOptions[0]); i++) {
            if (strcmp(textColor, colorOptions[i]) == 0) {
                textColorIndex = i;
                break;
            }
        }
    }

    if (textColorIndex != -1) {
        uart_puts((char*)textColorCode[textColorIndex]);
    }  else {
        uart_puts("Invalid text color. Supported colors: BLACK, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE.\n");
    }
}

// ... (other code)

void *memset(void *ptr, int value, size_t num) {
    unsigned char *bytePtr = (unsigned char *)ptr;

    for (size_t i = 0; i < num; i++) {
        *bytePtr = (unsigned char)value;
        bytePtr++;
    }

    return ptr;
}

// Function to perform auto-completion
void autocomplete(char *cmd_buffer, int *cmd_index) {
    int matches = 0;
    int match_index = -1;
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if (strncmp(commands[i], cmd_buffer, *cmd_index) == 0) {
            matches++;
            match_index = i;
        }
    }

    if (matches == 1) {
        strcpy(cmd_buffer, commands[match_index]);
        *cmd_index = strlen(commands[match_index]);
        uart_puts("\r");
        uart_puts("\nMyBareMetalOS> ");  // Print the prompt
        uart_puts(cmd_buffer);
    } else if (matches > 1) {
        uart_puts("\nPossible completions:");
        for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
            if (strncmp(commands[i], cmd_buffer, *cmd_index) == 0) {
                uart_puts(" ");
                uart_puts((char *)commands[i]);
            }
        }
        uart_puts("\n");
        *cmd_index = 0;
        uart_puts("MyBareMetalOS> ");
        memset(cmd_buffer, '\0', MAX_CMD_SIZE);
    } else {
        uart_puts("\nNo suggestions found.\n");
        uart_puts("MyBareMetalOS> ");
        uart_puts(cmd_buffer);
    }
}

void add_to_history(const char *cmd) {
    if (history_count < HISTORY_SIZE) {
        strcpy(history[history_count], cmd);
        history_count++;
    } else {
        history_count = 0;
    }
}

void play_game() {
    GameGenerator();
}
void GameGenerator() {
    maze = (char*)malloc(widthScreen * heightScreen * sizeof(char));
    if (maze == NULL) {
        printf("Not enough memory, the game cant be generated!");
    }
    else {
        clearGame(widthScreen, heightScreen);
        GenerateMaze(maze, widthScreen, heightScreen);
        ShowMaze(maze, widthScreen, heightScreen);
        drawMap(maze, widthScreen, heightScreen);
        inGame = 1;
    }
}

void clear_frame(int heightScreen, int widthScreen) {
    for (int j = 0; j < heightScreen; j++) {
        for (int i = 0; i < widthScreen; i++){
            drawPixelARGB32(i + x_direct,j + y_direct, 0x00000000);
        }
    }
}

void clearGame(int widthScreen, int heightScreen) {
    for (int j = 0; j < heightScreen * 20; j++) {
        for (int i = 0; i < widthScreen * 20; i++){
            drawPixelARGB32(i ,j, 0x00000000);
        }
    }
}
void cli()
{
	static char cli_buffer[MAX_CMD_SIZE];
	static int index = 0;

	//read and send back each char
	char c = uart_getc();

    // if (inGame == 1) {
    //     if (c == 'w') {
    //         if (checkDirection(3) == 1) {
    //         clear_frame(20, 21);
    //         y_direct -= 20;
    //         }
    //         if (checkDirection(3) == 2) {
    //             GameGenerator();
    //             return;
    //         }
    //     }
    //     else if (c == 'a') {
    //         if (checkDirection(6) == 1) {
    //         clear_frame(20, 21);
    //         x_direct -= 20;
    //         }
    //         if (checkDirection(6) == 2) {
    //             GameGenerator();
    //             return;
    //         }
    //     }
    //     else if (c == 's') {
    //         if (checkDirection(5) == 1) {
    //         clear_frame(20, 21);
    //         y_direct += 20;
    //         }
    //         if (checkDirection(5) == 2) {
    //             GameGenerator();
    //             return;
    //         }
    //     }
    //     else if (c == 'd') {
    //         if (checkDirection(4) == 1) {
    //         clear_frame(20, 21);
    //         x_direct += 20;
    //         }
    //         if (checkDirection(4) == 2) {
    //             GameGenerator();
    //             return;
    //         }
    //     }
    //     getNearFrontier(maze, x_direct / 20, y_direct / 20);
    //     draw_destination(x_direct, y_direct);
    //     return;
    // }
	uart_sendc(c);


	//put into a buffer until got new line character
    if (c == 127){
        deleteCommand(cli_buffer, &index);
    } else if (c != '\n' && c !='\t'){
		cli_buffer[index] = c; //Store into the buffer
		index++;
	} else if (c == '\n'){
		cli_buffer[index] = '\0';
		uart_puts("\nGot commands: ");
		uart_puts(cli_buffer); uart_puts("\n");
        add_to_history(cli_buffer);
        current_history_index = history_count; //Set the index for history browsing
		/* Compare with supported commands and execute
		* ........................................... */
        char* tokens[MAX_TOKENS]; // Array to store tokens
        char *token;
        char *saveptr = NULL;

        token = strtok(cli_buffer, " ", &saveptr);
        int numTokens = 0;

        while (token != NULL && numTokens < MAX_TOKENS) {
            tokens[numTokens] = token;
            numTokens++;
            token = strtok(NULL, " ", &saveptr);
        }

        if (numTokens > 0 && strcmp(tokens[0], "help") == 0) {
            if (numTokens == 1){
                help_command(cli_buffer);
            }else if (numTokens == 2){
                help_info(tokens[1]);
            } else {
                uart_puts("Unrecognized command: \n");
            }
        } else if (strcmp(tokens[0], "clear") == 0) {
            // Handle clear command
            clear_command();
        } else if (strcmp(tokens[0], "video") == 0) {
            draw_video();
        } else if (strcmp(tokens[0], "smallimg") == 0) {
            draw_image();
        } else if (strcmp(tokens[0], "game") == 0) {
            play_game();
        }
         else if (strcmp(tokens[0], "setcolor") == 0) {
            // Handle setcolor command
            if (numTokens == 1) {
            // Print usage information
            uart_puts("Set text color only:         setcolor -t <color>.\n");
            uart_puts("Set background color only:   setcolor -b <color>.\n");
            uart_puts("Set color for both:          setcolor -t <color> -b <color>.\n");
            uart_puts("Accepted colors: Black, Red, Green, Yellow, Blue, Purple, Cyan, White.\n");
            } else if (numTokens == 3 && strcmp(tokens[1], "-t") == 0) {
                // Handle setcolor -t <color>
                setTextColor(tokens[2]);
            } else if (numTokens == 3 && strcmp(tokens[1], "-b") == 0) {
                // Handle setcolor -b <color>
                // ...
                setBackGroundColor(tokens[2]);
            } else if (numTokens == 5 &&  (strcmp(tokens[1], "-t") == 0 && strcmp(tokens[3], "-b") == 0)) {
                // Handle setcolor -t <color> -b <color>
                // ...
                setTextColor (tokens[2]);
                setBackGroundColor(tokens[4]);
            } else if (numTokens == 5 && (strcmp(tokens[1], "-b") == 0 && strcmp(tokens[3], "-t") == 0)){
                // Handle setcolor -B <color> -t <color>
                setTextColor (tokens[4]);
                setBackGroundColor(tokens[2]);
            } else {
                // Invalid usage
                uart_puts("Invalid usage. Use 'help setcolor' for usage information.\n");
            }
        } else if (strcmp(tokens[0], "showinfo") == 0) {
            // Handle showinfo command
            showinfo();
        } else {
            // Handle unrecognized command
            uart_puts("Unrecognized command: \n");
        }

		//Return to command line
		index = 0;

        uart_puts("MyBareMetalOS> ");
	} else if (c == '\t') {
        cli_buffer[index] = '\0';
        autocomplete(cli_buffer, &index);
    } 

    if (c == '+') { // Use '_' as UP arrow
        if (current_history_index < history_count) {
            current_history_index++;
        }
        if (current_history_index >= history_count) {
            current_history_index = 0;
        }
        memset(cli_buffer, '\0', MAX_CMD_SIZE);
        strcpy(cli_buffer, history[current_history_index]);
        index = strlen(cli_buffer);

        uart_puts("\r");
        uart_puts("\x1B[K");
        uart_puts("MyBareMetalOS> ");
        uart_puts(cli_buffer);
    }

    if (c == '_') { // Use '+' as DOWN arrow
        if (current_history_index == 0) {
            current_history_index = history_count;
        }
        current_history_index --;
        memset(cli_buffer, '\0', MAX_CMD_SIZE);
        strcpy(cli_buffer, history[current_history_index]);
        index = strlen(cli_buffer);

        uart_puts("\r"); // Move cursor to the beginning of the line
        uart_puts("\x1B[K"); // Clear the line
        uart_puts("MyBareMetalOS> ");   
        uart_puts(cli_buffer);
    }
}

void mbox_buffer_setup(unsigned int buffer_addr, unsigned int tag_identifier,
                       unsigned int **res_data, unsigned int res_length,
                       unsigned int req_length, ...);

void getBoardRevision(){
    unsigned int *revision = 0;
    mbox_buffer_setup(ADDR(mBuf), 0x00010002, &revision, 4, 0, 0);
    mbox_call(ADDR(mBuf), MBOX_CH_PROP);
    uart_puts("Board revision: ");
    uart_hex(revision[0]);
    uart_puts("\n");
}

void getfirmwareRevision(){
    unsigned int *firmware = 0;
    mbox_buffer_setup(ADDR(mBuf), 0x00000001, &firmware, 4, 0, 0);
    mbox_call(ADDR(mBuf), MBOX_CH_PROP);
    uart_puts("Firmware revision: ");
    uart_hex(firmware[0]);
    uart_puts("\n");
}

void getUARTclockrate(){
    unsigned int *UART_clockrate = 0;
    mbox_buffer_setup(ADDR(mBuf), 0x00030002, &UART_clockrate, 8, 4, 2);
    mbox_call(ADDR(mBuf), MBOX_CH_PROP);
    uart_puts("UART clock rate = ");
    uart_dec(UART_clockrate[1]);
    uart_puts("\n");
}

void getARMclockrate(){
    unsigned int *ARM_clockrate = 0;
    mbox_buffer_setup(ADDR(mBuf), 0x00030002, &ARM_clockrate, 8, 4, 3);
    mbox_call(ADDR(mBuf), MBOX_CH_PROP);
    uart_puts("ARM clock rate = ");
    uart_dec(ARM_clockrate[1]);
    uart_puts("\n");
}

// void SetPhyWHFrame(){
//     unsigned int *physize = 0; // Pointer to response data
//     mbox_buffer_setup(ADDR(mBuf), MBOX_TAG_SETPHYWH, &physize, 8, 8, 1024, 768);
//     mbox_call(ADDR(mBuf), MBOX_CH_PROP);
//     uart_puts("Got Actual Physical widthScreen: ");
//     uart_dec(physize[0]); 
//     uart_puts("\nGot Actual Physical heightScreen: ");
//     uart_dec(physize[1]);
//     uart_puts("\n");
// }

void main(){
    // set up serial console
    framebf_init();
    // drawRectARGB32(100,100,400,400,0x00AA0000,1); //RED
    // drawRectARGB32(150,150,400,400,0x0000BB00,1); //GREEN
    // drawRectARGB32(200,200,400,400,0x000000CC,1); //BLUE
    // drawRectARGB32(250,250,400,400,0x00FFFF00,1); //YELLOW
    // drawPixelARGB32(300, 300, 0x00FF0000); //RED
    // draw_image();
    

	uart_init();
    uart_puts("\033[31m");
	uart_puts("8888888888 8888888888 8888888888 88888888888  .d8888b.      d8888   .d8888b.   .d8888b.  \n");
    uart_puts("888        888        888            888     d88P  Y88b    d8P888  d88P  Y88b d88P  Y88b \n");
    uart_puts("888        888        888            888            888   d8P 888  888    888 888    888 \n");
    uart_puts("8888888    8888888    8888888        888          .d88P  d8P  888  Y88b. d888 888    888 \n");
    uart_puts("888        888        888            888      .od888P'  d88   888   'Y888P888 888    888 \n");
    uart_puts("888        888        888            888     d88P'      8888888888        888 888    888\n");
    uart_puts("888        888        888            888     888'             888  Y88b  d88P Y88b  d88P \n");
    uart_puts("8888888888 8888888888 8888888888     888     888888888        888   'Y8888P'   'Y8888P'\n");
    uart_puts("\n\n");
    uart_puts("888888b.          d8888 8888888b.  8888888888      .d88888b.   .d8888b. \n");
    uart_puts("888  '88b        d88888 888   Y88b 888            d88P' 'Y88b d88P  Y88b \n");
    uart_puts("888  .88P       d88P888 888    888 888            888     888 Y88b.\n");
    uart_puts("8888888K.      d88P 888 888   d88P 8888888        888     888  'Y888b.\n");
    uart_puts("888  'Y88b    d88P  888 8888888P'  888            888     888     'Y88b.\n");
    uart_puts("888    888   d88P   888 888 T88b   888            888     888       '888 \n");
    uart_puts("888   d88P  d8888888888 888  T88b  888            Y88b. .d88P Y88b  d88P\n");
    uart_puts("8888888P'  d88P     888 888   T88b 8888888888      'Y88888P'   'Y8888P'\n");
    uart_puts("\n\n");
    uart_puts("Developed by Nguyen Giang Huy - s3836454\n");
    int num = 123;
    int nev_num = -123;
    float nev_float = -999.123;
    float float_num = 123.4545;
    char ch = 'A';
    int hex_num = 100;
    char str[] = "Hello World!";
    uart_puts("\033[36m");
    printf("\n///////////////////////////////////////////////////////////////////////////////////////////");
    printf("\n/////////////////   TEST     ALL     CASES     FOR     DECIMAL  ///////////////////////////\n");
    printf("///////////////////////////////////////////////////////////////////////////////////////////\n");
    uart_puts("\033[37m");
    printf("Positive Integer Number                             :%d\n", num);
    printf("Negative Integer Number                             :%d\n", nev_num);
    printf("Preceding Positive Number with blanks               :%10d\n", num);
    printf("Preceding Negative Number with blanks               :%10d\n", nev_num);
    printf("Negative Number with 0Flag                          :%010d\n", nev_num);
    printf("Positive Number with 0Flag                          :%010d\n", num);
    printf("Positive Number with widthScreen                          :%*d\n", 10, num);
    printf("Negative Number with widthScreen                          :%*d\n", 10, nev_num);
    printf("Integer Value with padding size .*                  : %.*d\n", 5, num);
    uart_puts("\033[32m");
    printf("///////////////////////////////////////////////////////////////////////////////////////////");
    printf("\n///////////////    TEST     ALL     CASES     FOR     FLOAT   /////////////////////////////\n");
    printf("///////////////////////////////////////////////////////////////////////////////////////////\n");
    uart_puts("\033[37m");
    printf("Positive Float Number                               :%f\n", float_num);
    printf("Negative Float Number:                              :%f\n", nev_float);
    printf("Float with Precision                                :%.3f\n", float_num);
    printf("widthScreen and Precision test                            :%15.4f\n", 123.45678);
    printf("widthScreen and Precision test                            :%015f\n", 3.12);
    printf("widthScreen and Precision test with padding:              :%010.2f\n", 123.45678);
    uart_puts("\033[33m");
    printf("///////////////////////////////////////////////////////////////////////////////////////////");
    printf("\n////////////////   TEST     ALL     CASES     FOR     STRING     AND     CHARACTER   //////\n");
    printf("///////////////////////////////////////////////////////////////////////////////////////////\n");
    uart_puts("\033[37m");
    printf("Character                                           :%c\n", ch);
    printf("Character with widthScreen                                :<%15c>\n", ch);
    printf("Character Value with .*                             :%.*c\n", 0, ch);
    printf("Character with widthScreen and NO 0 FLAG includes size .* :<%*c>\n", 15, 'a');
    printf("String                                              :%s\n", str);
    printf("Widtd With String and NO 0 FLAG                     :<%15s>\n", "Embedded");
    printf("widthScreen With String and NO 0 FLAG includes size .*    :<%*s>\n", 15, "Embedded");
    printf("widthScreen Display With String and 0 FLAG                :<%015s>\n", "Embedded");
    uart_puts("\033[34m");
    printf("///////////////////////////////////////////////////////////////////////////////////////////");
    printf("\n/////////////////  TEST     ALL     CASES     FOR     HEXADECIMAL    //////////////////////\n");
    printf("///////////////////////////////////////////////////////////////////////////////////////////\n");
    uart_puts("\033[37m");
    printf("Hexadecimal:                                        :%x\n", hex_num);
    printf("Hex Value with padding size .*                      :%.*x\n", 5, hex_num);
    uart_puts("\033[35m");
    printf("///////////////////////////////////////////////////////////////////////////////////////////");
    printf("\n////////////////   TEST     FOR     PERCENTAGE     SYMBOL         /////////////////////////\n");
    printf("///////////////////////////////////////////////////////////////////////////////////////////\n");
    uart_puts("\033[37m");
    printf("This is percentage symbol                           :%%\n");
     uart_puts("\033[30m");
    printf("\n///////////////////////////////////////////////////////////////////////////////////////////");
    printf("\n////////////////   MAIL     BOX     SET     UP         ////////////////////////////////////\n");
    printf("///////////////////////////////////////////////////////////////////////////////////////////\n");
    uart_puts("\033[37m");
    uart_puts("\n");
    getBoardRevision();
    uart_puts("\n");
    getfirmwareRevision();
    uart_puts("\n");
    getARMclockrate();
    uart_puts("\n");
    getUARTclockrate();
    uart_puts("\n");
    // SetPhyWHFrame();

    uart_puts("\n"); 
    uart_puts("MyBareMetalOS> ");              
    
    // run CLI
    while(1) {
    	cli();
    }
}