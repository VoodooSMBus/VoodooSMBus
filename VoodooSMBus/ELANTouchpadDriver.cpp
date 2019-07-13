
#include "ELANTouchpadDriver.hpp"


#define super IOService

OSDefineMetaClassAndStructors(ELANTouchpadDriver, IOService);


void ELANTouchpadDriver::releaseResources() {
    
    OSSafeReleaseNULL(device_nub);
}

bool ELANTouchpadDriver::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }
    
    device_nub->retain();
    
    return true;
exit:
    releaseResources();
    return false;
}

void ELANTouchpadDriver::stop(IOService* provider) {
    releaseResources();
    super::stop(provider);
}

ELANTouchpadDriver* ELANTouchpadDriver::probe(IOService* provider, SInt32* score) {
    IOLog("Touchpad probe");
    if (!super::probe(provider, score)) {
        return NULL;
    }
    
    device_nub = OSDynamicCast(VoodooSMBusDeviceNub, provider);
    if (!device_nub) {
        IOLog("%s Could not get VoodooSMBus device nub instance\n", getName());
        return NULL;
    }
    return this;
}
