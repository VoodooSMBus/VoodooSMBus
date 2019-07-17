/*
 * VoodooSMBusIntelLpssI2C.hpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */


#ifndef VoodooSMBusIntelLpssI2C_hpp
#define VoodooSMBusIntelLpssI2C_hpp

#include "VoodooSMBusSlaveDeviceDriver.hpp"



class VoodooSMBusIntelLpssI2C : public IOService {
    OSDeclareDefaultStructors(VoodooSMBusIntelLpssI2C)
public:
};

#endif /* VoodooSMBusIntelLpssI2C_hpp */
