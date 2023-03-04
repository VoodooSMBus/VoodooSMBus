/*
 * VoodooSMBusDeviceNub.cpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */

#include "VoodooSMBusControllerDriver.hpp"
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

void VoodooSMBusDeviceNub::handleHostNotifyThreaded () {
    IOService* device_driver = getClient();
    
    if(device_driver) {
        super::messageClient(kIOMessageVoodooSMBusHostNotify, device_driver);
    }
}

void VoodooSMBusDeviceNub::handleHostNotify() {
    thread_t new_thread;
    kern_return_t ret = kernel_thread_start(OSMemberFunctionCast(thread_continue_t, this, &VoodooSMBusDeviceNub::handleHostNotifyThreaded), this, &new_thread);

    if (ret != KERN_SUCCESS) {
        IOLogDebug(" Thread error while attemping to handle host notify in device nub.");
    } else {
        thread_deallocate(new_thread);
    }
}


bool VoodooSMBusDeviceNub::attach(IOService* provider, UInt8 address) {
    if (!super::attach(provider))
        return false;
    
    controller = OSDynamicCast(VoodooSMBusControllerDriver, provider);
    if (!controller) {
        IOLogError("%s Could not get controller", provider->getName());
        return false;
    }
    
    setProperty("VoodooSMBUS Slave Device Address", address, 8);
    slave_device->addr = address;
    slave_device->flags = 0;
    
    return true;
}


void VoodooSMBusDeviceNub::releaseResources() {

}

IOReturn VoodooSMBusDeviceNub::wakeupController() {
    if (controller) {
       return controller->makeUsable();
    } else {
       return kIOReturnError;
    }
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

void VoodooSMBusDeviceNub::setSlaveDeviceFlags(unsigned short flags) {
    slave_device->flags = flags;
}

IOReturn VoodooSMBusDeviceNub::readByteData(u8 command) {
    return controller->readByteData(slave_device, command);
}

IOReturn VoodooSMBusDeviceNub::readBlockData(u8 command, u8 *values) {
    return controller->readBlockData(slave_device, command, values);
}

IOReturn VoodooSMBusDeviceNub::writeByteData(u8 command, u8 value) {
    return controller->writeByteData(slave_device, command, value);
}

IOReturn VoodooSMBusDeviceNub::writeByte(u8 value) {
    return controller->writeByte(slave_device, value);
}

IOReturn VoodooSMBusDeviceNub::writeBlockData(u8 command, u8 length, const u8 *values) {
    return controller->writeBlockData(slave_device, command, length, values);
}
