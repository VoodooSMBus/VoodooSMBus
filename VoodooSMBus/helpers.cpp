#include "helpers.hpp"

char * addrToDictKey(u8 address) {
    char key[5];
    snprintf(key, 5, "0x%x", address);
    return key;
}
