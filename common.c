#include "common.h"
#include <stdlib.h>

int extract_string(char* text, size_t length)
{
    int ret = -1;
    size_t i;

    for (i = 0; i < length; ++i) {
        if ('0' <= text[i] && text[i] <= '9') {
            if (ret == -1) {
                ret = (text[i] - '0');
            } else {
                ret = 10 * ret + text[i] - '0';
            }
        } else if (ret != -1) {
            break;
        }
    }

    return ret;
}
