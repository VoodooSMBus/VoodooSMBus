/*
 * VoodooSMBusDeviceNub.hpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */


#ifndef VoodooSMBusDeviceNub_hpp
#define VoodooSMBusDeviceNub_hpp


#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "VoodooSMBusControllerDriver.hpp"

class VoodooSMBusControllerDriver;

class VoodooSMBusDeviceNub : public IOService {
    OSDeclareDefaultStructors(VoodooSMBusDeviceNub);
    
public:
    bool init() override;
    bool attach(IOService* provider, UInt8 address);
    bool start(IOService* provider) override;
    void stop(IOService* provider) override;
    void free(void) override;

    void handleHostNotify();
    void setSlaveDeviceFlags(unsigned short flags);
    
    IOReturn writeByteData(u8 command, u8 value);
    IOReturn readByteData(u8 command);
    IOReturn readBlockData(u8 command, u8 *values);
    IOReturn writeByte(u8 value);
    IOReturn writeBlockData(u8 command, u8 length, const u8 *values);

private:
    VoodooSMBusControllerDriver* controller;
    void releaseResources();
    VoodooSMBusSlaveDevice* slave_device;
    void handleHostNotifyThreaded();
};

#endif /* VoodooSMBusDeviceNub_hpp */
