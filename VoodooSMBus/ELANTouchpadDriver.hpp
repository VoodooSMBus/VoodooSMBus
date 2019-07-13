
#ifndef ELANTouchpadDriver_hpp
#define ELANTouchpadDriver_hpp


#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "VoodooSMBusDeviceNub.hpp"


class ELANTouchpadDriver : public IOService {
    OSDeclareDefaultStructors(ELANTouchpadDriver);
    
public:
    
    bool start(IOService* provider) override;
    
    void stop(IOService* provider) override;
    
    ELANTouchpadDriver* probe(IOService* provider, SInt32* score) override;

private:
    void releaseResources();
    VoodooSMBusDeviceNub* device_nub;
};




#endif /* ELANTouchpadDriver_hpp */
