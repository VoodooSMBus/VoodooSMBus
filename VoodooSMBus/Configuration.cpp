/*
 * Configuration.cpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */

#include "Configuration.hpp"


bool Configuration::loadBoolConfiguration(IOService* service, const char* configurationKey, bool defaultValue) {
    OSDictionary *configuration;
    configuration = OSDynamicCast(OSDictionary, service->getProperty("Configuration"));
    
    if (configuration) {
        OSBoolean* value = OSDynamicCast(OSBoolean, configuration->getObject(configurationKey));
        if (value != nullptr) {
            return value->getValue();
        }
    }
    
    return defaultValue;
}

UInt64 Configuration::loadUInt64Configuration(IOService* service, const char* configurationKey, UInt64 defaultValue) {
    OSDictionary *configuration;
    configuration = OSDynamicCast(OSDictionary, service->getProperty("Configuration"));
    
    if (configuration) {
        OSNumber* value = OSDynamicCast(OSNumber, configuration->getObject(configurationKey));
        if (value != nullptr) {
            return value->unsigned64BitValue();
        }
    }
    
    return defaultValue;
}
