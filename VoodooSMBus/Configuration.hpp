/*
 * Configuration.hpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */

#ifndef Configuration_hpp
#define Configuration_hpp 

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>

class Configuration {
    
public:
    static bool loadBoolConfiguration(IOService* service, const char* configurationKey, bool defaultValue);
    static UInt64 loadUInt64Configuration(IOService* service, const char* configurationKey, UInt64 defaultValue);
    
private:
    Configuration() {}

};

#endif /* Configuration_hpp */
