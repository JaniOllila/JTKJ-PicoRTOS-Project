#include <string.h>
#include "morse.h"
#define INPUT_BUFFER_SIZE 256


//Linking morse code symbol with letter
struct MorseRow {
    const char *morse;
    char letter;
};


//Letter mappings
static struct MorseRow morse_chars[] = {
    {".-",     'a'},
    {"-...",   'b'},
    {"-.-.",  'c'},
    {"-..",    'd'},
    {".",      'e'},
    {"..-.",   'f'},
    {"--.",    'g'},
    {"....",   'h'},
    {"..",     'i'},
    {".---",   'j'},
    {"-.-",    'k'},
    {".-..",   'l'},
    {"--",     'm'},
    {"-.",     'n'},
    {"---",    'o'},
    {".--.",   'p'},
    {"--.-",   'q'},
    {".-.",    'r'},
    {"...",    's'},
    {"-",      't'},
    {"..-",    'u'},
    {"...-",   'v'},
    {".--",    'w'},
    {"-..-",   'x'},
    {"-.--",   'y'},
    {"--..",   'z'}
};

static const int morse_chars_len = sizeof(morse_chars) / sizeof(morse_chars[0]);
// Prototypes
char morse_symbol_to_char(const char *symbol);
void morse_line_to_text(const char *morse_line, char *out, int out_size);
void decoded_line_to_buffer(char* decoded_buffer, char* decoded_line_buffer);
void morsebuffer_to_text(const char *morse_buffer, char* decoded_buffer);

//Converts single morse code symbol to character
char morse_symbol_to_char(const char *symbol)
{
    int i;
    for (i = 0; i < morse_chars_len; i++) {
        if (strcmp(symbol, morse_chars[i].morse) == 0) {
            return morse_chars[i].letter;
        }
    }

    return '?';
}


//Parts of the logic were planned with assistance of AI

//Converts morse code line to text
void morse_line_to_text(const char *morse_line, char *out, int out_size)
{
    char token[8];          //Stores one morse letter
    int token_i = 0;        //Index inside token
    int out_i = 0;          //Index inside output string
    int space_count = 0;    //Counts spaces
    int i = 0;


    //Check input character by character
    while (morse_line[i] != '\0' && out_i <= out_size - 1) {

        char ch = morse_line[i];

        //New line = end of line
        if (ch == '\n') {
            //Convert last unfinished morse symbol
            if (token_i > 0) {
                token[token_i] = '\0';
                out[out_i] = morse_symbol_to_char(token);
                out_i++;
                token_i = 0;
            }
            //End processing
            break;
        }

        if (ch == ' ') {
            space_count++;

            //Reached end of one morse symbol -> convert it
            if (token_i > 0) {
                token[token_i] = '\0';
                out[out_i] = morse_symbol_to_char(token);
                out_i++;
                token_i = 0;
            } else {
                //Check for word break (two spaces)
                if (space_count == 2) {
                    out[out_i] = ' ';
                    out_i++;
                    space_count = 0;
                }
            }

        } else {
            //Dot or dash continues current morse symbol
            space_count = 0;
            if (token_i < (int)sizeof(token) - 1) {
                token[token_i] = ch;
                token_i++;
            }
        }

        i++;
    }

    //Convert last unfinished morse symbol
    if (token_i > 0 && out_i < out_size - 1) {
        token[token_i] = '\0';
        out[out_i] = morse_symbol_to_char(token);
        out_i++;
    }

    out[out_i] = '\0';
}

void decoded_line_to_buffer(char* decoded_buffer, char* decoded_line_buffer) {
    int start_point = 0;
    while (start_point < INPUT_BUFFER_SIZE - 1 && decoded_buffer[start_point] != '\0') start_point++;

    for (int i= 0; decoded_line_buffer[i] != '\0' && i<INPUT_BUFFER_SIZE - 1 && start_point < INPUT_BUFFER_SIZE - 1; i++) {
        decoded_buffer[start_point] = decoded_line_buffer[i];
        start_point++;
    }

    decoded_buffer[start_point] = '\n';
    decoded_buffer[start_point + 1] = '\0';

}

void morsebuffer_to_text(const char *morse_buffer, char* decoded_buffer) {
    char line_buf[INPUT_BUFFER_SIZE];
    char decoded_line_buf[INPUT_BUFFER_SIZE];
    int j = 0;

    for (int i = 0; i < INPUT_BUFFER_SIZE - 1; i++) {
        //Checking if end of morse buffer is reached
        if (morse_buffer[i] == '\0') {
            line_buf[j] = '\0';
            morse_line_to_text(line_buf, decoded_line_buf, INPUT_BUFFER_SIZE);
            decoded_line_to_buffer(decoded_buffer, decoded_line_buf);
            j = 0;
            line_buf[0] = '\0';
            decoded_line_buf[0] = '\0';
            break;
        }
        // Checking for new line to decode the line
        else if (morse_buffer[i] == '\n') {
            line_buf[j] = '\0';
            morse_line_to_text(line_buf, decoded_line_buf, INPUT_BUFFER_SIZE);
            decoded_line_to_buffer(decoded_buffer, decoded_line_buf);
            j = 0;
            line_buf[0] = '\0';
            decoded_line_buf[0] = '\0';
        }
        // Checking if line buffer size is reached
        else if (i == INPUT_BUFFER_SIZE - 1){
            line_buf[j] = '\0';
            morse_line_to_text(line_buf, decoded_line_buf, INPUT_BUFFER_SIZE);
            decoded_line_to_buffer(decoded_buffer, decoded_line_buf);
            j = 0;
            line_buf[0] = '\0';
            decoded_line_buf[0] = '\0';
        }
        // If not end or new line, keep adding to line buffer
        else{
            line_buf[j] = morse_buffer[i];
            j++;
        }

    }


}