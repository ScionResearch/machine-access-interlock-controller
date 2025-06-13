#pragma once

//-------------------------------------------------------------------//
//                                                                   //
// Terminal helper functions                                         //
// March 2024 - Scion Research - J Peake                             //
// Basic terminal functions and escape codes for terminal menus      //
//                                                                   //
//-------------------------------------------------------------------//

// ASCII defines for readability

#define BACKSPACE       0x7F
#define ENTER           0x0D
#define DECIMAL_POINT   0x2E

// ANSI serial terminal colours
#define ANSI_BLACK    30
#define ANSI_RED      31
#define ANSI_GREEN    32
#define ANSI_YELLOW   33
#define ANSI_BLUE     34
#define ANSI_MAGENTA  35
#define ANSI_CYAN     36
#define ANSI_WHITE    37

// Returns ANSI escape code for cursor to requested column
String term_col(uint8_t column) {
  return "\e[" + String(column) + "G";
}

// Returns ANSI escape code for cursor to requested row/column
String term_cursor(uint8_t row, uint8_t column) {
  return "\e[" + String(row) + ";" + String(column) + "f";
}

// Clear screen
String term_cls(void) {
  return "\e[2J" + term_cursor(0, 0);
}

// Clear from cursor to end of line
String term_clr_line(void) {
  return "\e[0K";
}

// Clear from cursor to end of line
String term_clr_line_from(uint8_t row, uint8_t column) {
  return term_cursor(row, column) + "\e[0K";
}

// Clear from cursor to end of screen
String term_clr_from(uint8_t row, uint8_t column) {
  return term_cursor(row, column) + "\e[0J";
}

// Takes a basic string and returns it formatted to the specified colour
String term_colour_str(String str, uint8_t colour) {
  return "\e[" + String(colour) + "m" + str + "\e[39;49m";
}

// Colours string conditional to second arg
String term_cond_colour_str(String str, bool condition, uint8_t false_colour, uint8_t true_colour) {
  return "\e[" + String(condition ? true_colour : false_colour) + "m" + str + "\e[39;49m";
}