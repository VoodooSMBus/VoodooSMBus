/*
 * VoodooSMBusDeviceNub.cpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */

#include "VoodooSMBusDeviceNub.hpp"

#define super IOService

OSDefineMetaClassAndStructors(VoodooSMBusDeviceNub, IOService);

bool VoodooSMBusDeviceNub::init() {
    bool result = super::init();

    slave_device = reinterpret_cast<VoodooSMBusSlaveDevice*>(IOMalloc(sizeof(VoodooSMBusSlaveDevice)));
    return result;
}

void VoodooSMBusDeviceNub::free(void) {
    IOFree(slave_device, sizeof(VoodooSMBusSlaveDevice));
    super::free();
}

void VoodooSMBusDeviceNub::HandleHostNotify () {
    VoodooSMBusSlaveDeviceDriver* device_driver = OSDynamicCast(VoodooSMBusSlaveDeviceDriver, getClient());
    
    if(device_driver) {
        device_driver->handleHostNotify();
    }
}


bool VoodooSMBusDeviceNub::attach(IOService* provider, UInt8 address) {
    if (!super::attach(provider))
        return false;
    
    controller = OSDynamicCast(VoodooSMBusControllerDriver, provider);
    if (!controller) {
        IOLog("%s Could not get controller\n", provider->getName());
        return false;
    }
    
    setProperty("VoodooSMBUS Slave Device Address", OSNumber::withNumber(address, 8));
    slave_device->addr = address;
    
    return true;
}


void VoodooSMBusDeviceNub::releaseResources() {

}

bool VoodooSMBusDeviceNub::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }
    
    registerService();
    return true;
}

void VoodooSMBusDeviceNub::stop(IOService* provider) {
    super::stop(provider);
}

IOReturn VoodooSMBusDeviceNub::WriteByteData(u8 command, u8 value) {
    return controller->WriteByteData(slave_device, command, value);
}

IOReturn VoodooSMBusDeviceNub::ReadBlockData(u8 command, u8 *values) {
    return controller->ReadBlockData(slave_device, command, values);
}

IOReturn VoodooSMBusDeviceNub::WriteByte(u8 value) {
    return controller->WriteByte(slave_device, value);
}

IOReturn VoodooSMBusDeviceNub::writeBlockData(u8 command, u8 length, const u8 *values) {
    return controller->writeBlockData(slave_device, command, length, values);
}
