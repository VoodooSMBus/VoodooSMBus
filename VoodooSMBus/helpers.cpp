#include "helpers.hpp"

void addrToDictKey(u8 address, char *key) {
    snprintf(key, 5, "0x%x", address);
}

const char* getMatchedName(IOService* provider) {
    OSData *data;
    data = OSDynamicCast(OSData, provider->getProperty("name"));
    return (const char *)(data->getBytesNoCopy());
}
