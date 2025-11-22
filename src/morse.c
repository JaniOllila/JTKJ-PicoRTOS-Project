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
    while (morse_line[i] != '\0' && out_i < out_size - 1) {

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

//From this point on AI (copilot) was only used to figure out what was wrong with functions below. It had issues with
// the looping and indexing of decoded_line_to_buffer. It didn't change any logic but a better structure of the function. 


// This function first finds the end of the current decoded_buffer and continues the new line from there. After adding the new line to the decoded_buffer,
//it adds a new line character and null terminator at the end. 
void decoded_line_to_buffer(char* decoded_buffer, char* decoded_line_buffer) {
    int start_point = 0;
    // Finding the end of the current decoded_buffer
    while (start_point < INPUT_BUFFER_SIZE - 1 && decoded_buffer[start_point] != '\0') start_point++;

    // Appending the new decoded line to the end of decoded_buffer
    for (int i= 0; decoded_line_buffer[i] != '\0' && i<INPUT_BUFFER_SIZE - 1 && start_point < INPUT_BUFFER_SIZE - 1; i++) {
        decoded_buffer[start_point] = decoded_line_buffer[i];
        start_point++;
    }
    // Adding a new line character and null terminator at the end
    decoded_buffer[start_point] = '\n';
    decoded_buffer[start_point + 1] = '\0';

}

// This function translates the entire morse_buffer into text using the functions above. 

void morsebuffer_to_text(const char *morse_buffer, char* decoded_buffer) {
    // Creating temporary buffers for processing lines and a index j for line buffer
    char line_buf[INPUT_BUFFER_SIZE];
    char decoded_line_buf[INPUT_BUFFER_SIZE];
    int j = 0;

    // Looping through the morse_buffer
    for (int i = 0; i < INPUT_BUFFER_SIZE - 1; i++) {
        //Checking if end of morse buffer is reached. If yes, then process the last line and break the loop.
        if (morse_buffer[i] == '\0') {
            line_buf[j] = '\0';
            morse_line_to_text(line_buf, decoded_line_buf, INPUT_BUFFER_SIZE);
            decoded_line_to_buffer(decoded_buffer, decoded_line_buf);
            j = 0;
            line_buf[0] = '\0';
            decoded_line_buf[0] = '\0';
            break;
        }
        // Checking for new line to decode the line. If yes, then process the line buffer and reset the temporary buffers and index j.
        else if (morse_buffer[i] == '\n') {
            line_buf[j] = '\0';
            morse_line_to_text(line_buf, decoded_line_buf, INPUT_BUFFER_SIZE);
            decoded_line_to_buffer(decoded_buffer, decoded_line_buf);
            j = 0;
            line_buf[0] = '\0';
            decoded_line_buf[0] = '\0';
        }
        // Checking if line buffer size is reached. If yes, then process the line buffer and reset the temporary buffers and index j.
        else if (i == INPUT_BUFFER_SIZE - 1){
            line_buf[j] = '\0';
            morse_line_to_text(line_buf, decoded_line_buf, INPUT_BUFFER_SIZE);
            decoded_line_to_buffer(decoded_buffer, decoded_line_buf);
            j = 0;
            line_buf[0] = '\0';
            decoded_line_buf[0] = '\0';
        }
        // If not end or new line, keep adding to line buffer and increase index j by one.
        else{
            line_buf[j] = morse_buffer[i];
            j++;
        }

    }


}