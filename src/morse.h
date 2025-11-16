#ifndef MORSE_H
#define MORSE_H

/*Converts single morse code symnbol to character
  Returns '?' if symbol doesnt macth any existing letters
*/
char morse_symbol_to_char(const char *symbol);

//Converts full line of morse code into a text string
void morse_line_to_text(const char *morse_line, char *out, int out_size);

#endif
