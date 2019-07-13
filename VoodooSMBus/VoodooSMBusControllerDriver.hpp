
#ifndef VoodooSMBusControllerDriver_hpp
#define VoodooSMBusControllerDriver_hpp

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "helpers.hpp"
#include "i2c_i801.h"

/* An SMBus device on a PCI controller */
typedef struct {
    IOACPIPlatformDevice* acpi_device;
    bool awake = true;
    const char* name;
    IOPCIDevice* pci_device;
    // IOMemoryMap* mmap;
    IOService* provider;
    unsigned long smba;
} VoodooSMBusPhysicalDevice;


class VoodooSMBusControllerDriver : public IOService {
    OSDeclareDefaultStructors(VoodooSMBusControllerDriver)
public:
    IOPCIDevice* pciDevice;
    
    VoodooSMBusPhysicalDevice* physical_device;
    OSArray* device_nubs;
    
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    virtual IOService *probe(IOService *provider, SInt32 *score) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    IOReturn publishNub();
};

#endif /* VoodooSMBusControllerDriver_hpp */
