
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
    
    int repeat = ETP_RETRY_COUNT;
    int error;
    do {
        error = Initialize();
        IOLog("Initialize result %d", error);
        if (!error)
            break;
        
        IOSleep(30);
    } while (--repeat > 0);
    
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

// elan_smbus_initialize
int ELANTouchpadDriver::Initialize() {
    UInt8 check[ETP_SMBUS_HELLOPACKET_LEN] = { 0x55, 0x55, 0x55, 0x55, 0x55 };
    UInt8 values[I2C_SMBUS_BLOCK_MAX] = {0};
    int len, error;
    
    /* Get hello packet */
    len = device_nub->ReadBlockData(ETP_SMBUS_HELLOPACKET_CMD, values);
    
    if (len != ETP_SMBUS_HELLOPACKET_LEN) {
        IOLog("hello packet length fail: %d\n", len);
        error = len < 0 ? len : -EIO;
        return error;
    }
    
    /* compare hello packet */
    if (memcmp(values, check, ETP_SMBUS_HELLOPACKET_LEN)) {
        IOLog("hello packet fail [%*ph]\n",
                ETP_SMBUS_HELLOPACKET_LEN, values);
        return -ENXIO;
    }
    
    /* enable tp */
    error = device_nub->WriteByte(ETP_SMBUS_ENABLE_TP);
    if (error) {
        IOLog("failed to enable touchpad: %d\n", error);
        return error;
    }
    
    return 0;
}
