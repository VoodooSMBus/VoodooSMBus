/*
 * VoodooSMBusSlaveDeviceDriver.hpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */

#ifndef VoodooSMBusSlaveDeviceDriver_hpp
#define VoodooSMBusSlaveDeviceDriver_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>

class VoodooSMBusSlaveDeviceDriver : public IOService {
    OSDeclareDefaultStructors(VoodooSMBusSlaveDeviceDriver);
    
public:
    virtual void handleHostNotify() { }

};

#endif /* VoodooSMBusSLaveDeviceDriver_hpp */
