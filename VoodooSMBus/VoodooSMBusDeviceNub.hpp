#ifndef VoodooSMBusDeviceNub_hpp
#define VoodooSMBusDeviceNub_hpp


#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "VoodooSMBusControllerDriver.hpp"





class VoodooSMBusDeviceNub : public IOService {
    OSDeclareDefaultStructors(VoodooSMBusDeviceNub);
    
public:
    bool init() override;

    bool attach(IOService* provider, UInt8 address);
    
    bool start(IOService* provider) override;
    
    void stop(IOService* provider) override;
    virtual void free(void) override;

    
    IOReturn WriteByteData(u8 command, u8 value);
    IOReturn ReadBlockData(u8 command, u8 *values);
    IOReturn WriteByte(u8 value);

private:

    VoodooSMBusControllerDriver* controller;
    void releaseResources();
    VoodooSMBusSlaveDevice* slave_device;
};

#endif /* VoodooSMBusDeviceNub_hpp */
