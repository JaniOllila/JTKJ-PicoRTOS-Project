#ifndef MORSE_H
#define MORSE_H

//Prototypes

/*Converts single morse code symnbol to character
  Returns '?' if symbol doesnt macth any existing letters
*/
char morse_symbol_to_char(const char *symbol);

//Converts full line of morse code into a text string
void morse_line_to_text(const char *morse_line, char *out, int out_size);

//Appends one decoded text line to the end of a bigger decoded buffer
void decoded_line_to_buffer(char* decoded_buffer, char* decoded_line_buffer);

//Converts the whole morse buffer into text
void morsebuffer_to_text(const char *morse_buffer, char* decoded_buffer);

#endif
