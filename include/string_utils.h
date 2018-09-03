#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdlib.h>

// Converts a decimal number into its string representation
// It does not add a null terminator
int number_to_string(char* str, size_t size, int number);

// Returns the index of the first occurrence of a character
// or -1 if the character could not be found
int find_character(char *str, int size, char character);

#endif // STRING_UTILS_H
