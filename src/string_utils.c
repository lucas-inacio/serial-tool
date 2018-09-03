#include "string_utils.h"


int number_to_string(char* str, size_t size, int number)
{
    int value = number;
    int mod;
    int i = size - 1;

    do
    {
        mod = value % 10;
        value /= 10;
        str[i] = (char)mod + '0';
        --i;
    } while (value > 0 && i >= 0);

    if (i >= 0)
    {
        int j;
        int count = (int)size - i;
        for (j = 0; j < count; ++j) str[j] = str[i + 1 + j];
    }

    return 0;
}

int find_character(char *str, int size, char character)
{
    int result = -1;
    int i = 0;
    for (; i < size; ++i)
    {
        if (str[i] == character)
        {
            result = i;
            break;
        }
    }

    return result;
}