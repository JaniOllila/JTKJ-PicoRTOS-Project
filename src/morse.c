#include <string.h>
#include "morse.h"


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

        //Newline = end of message
        if (ch == '\n') {
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
