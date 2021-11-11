#include "helpers.hpp"

const char* getMatchedName(IOService* provider) {
    OSData *data;
    data = OSDynamicCast(OSData, provider->getProperty("name"));
    return (const char *)(data->getBytesNoCopy());
}
