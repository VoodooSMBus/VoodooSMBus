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
    bool attach(IOService* provider) override;
    
    bool start(IOService* provider) override;
    
    void stop(IOService* provider) override;
    
    
    
private:
    IOWorkLoop* getWorkLoop();

    IOCommandGate* command_gate;
    VoodooSMBusControllerDriver* controller;
    IOWorkLoop* work_loop;
    void releaseResources();
};

#endif /* VoodooSMBusDeviceNub_hpp */
