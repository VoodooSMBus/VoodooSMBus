//
//  RMIBus.c
//  VoodooSMBus
//
//  Created by Sheika Slate on 4/30/20.
//  Copyright © 2020 leo-labs. All rights reserved.
//

#include "RMIBus.hpp"

OSDefineMetaClassAndStructors(RMIBus, VoodooSMBusSlaveDeviceDriver)

#define super IOService

bool RMIBus::init(OSDictionary *dictionary) {
    return super::init(dictionary);
}

RMIBus * RMIBus::probe(IOService *provider, SInt32 *score) {
    IOLog("Probe");
    if (!super::probe(provider, score))
        return NULL;
    
    device_nub = OSDynamicCast(VoodooSMBusDeviceNub, provider);
    
    if (!device_nub) {
        IOLog("%s Could not et VoodooSMBus device nub instance\n", getName());
        return NULL;
    }
    
    return this;
}

bool RMIBus::start(IOService *provider) {
    if (!super::start(provider))
        return false;
    
//    provider->joinPMtree(this);
//    registerPowerDriver(this, , unsigned long numberOfStates);
    
    device_nub->setSlaveDeviceFlags(I2C_CLIENT_HOST_NOTIFY);
    
    int i = 0;
    IOSleep(3000);
    while (i++ < 10) {
        IOLog("Recieving SMBus version: %d\n", rmi_smb_get_version(device_nub));
        IOSleep(300);
    }
    
    registerService();
    return true;
}

void RMIBus::handleHostNotify() {
    IOLog("Notification recieved");
}

void RMIBus::stop(IOService *provider) {
    PMstop();
    super::stop(provider);
}

void RMIBus::initialize() {
    
    rmi_write_block
    
}
