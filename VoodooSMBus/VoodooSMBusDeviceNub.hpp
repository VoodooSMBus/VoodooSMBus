#ifndef VoodooSMBusDeviceNub_hpp
#define VoodooSMBusDeviceNub_hpp


#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "VoodooSMBusControllerDriver.hpp"


struct VoodooSMBusSlaveDevice {
    UInt8 address;        /* chip address - NOTE: 7bit    */
};


class VoodooSMBusDeviceNub : public IOService {
    OSDeclareDefaultStructors(VoodooSMBusDeviceNub);
    
public:
    bool init() override;

    bool attach(IOService* provider, UInt8 address);
    
    bool start(IOService* provider) override;
    
    void stop(IOService* provider) override;
    
    
private:
    IOWorkLoop* getWorkLoop();

    IOCommandGate* command_gate;
    VoodooSMBusControllerDriver* controller;
    IOWorkLoop* work_loop;
    void releaseResources();
    VoodooSMBusSlaveDevice* slave_device;
};

#endif /* VoodooSMBusDeviceNub_hpp */
