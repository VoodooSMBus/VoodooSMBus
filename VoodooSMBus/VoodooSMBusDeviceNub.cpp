
#include "VoodooSMBusDeviceNub.hpp"

#define super IOService

OSDefineMetaClassAndStructors(VoodooSMBusDeviceNub, IOService);

bool VoodooSMBusDeviceNub::attach(IOService* provider) {
    if (!super::attach(provider))
        return false;
    
    controller = OSDynamicCast(VoodooSMBusControllerDriver, provider);
    if (!controller) {
        IOLog("%s Could not get controller\n", provider->getName());
        return false;
    }
    
    return true;
}

IOWorkLoop* VoodooSMBusDeviceNub::getWorkLoop() {
    // Do we have a work loop already?, if so return it NOW.
    if ((vm_address_t) work_loop >> 1)
        return work_loop;
    
    if (OSCompareAndSwap(0, 1, reinterpret_cast<IOWorkLoop*>(&work_loop))) {
        // Construct the workloop and set the cntrlSync variable
        // to whatever the result is and return
        work_loop = IOWorkLoop::workLoop();
    } else {
        while (reinterpret_cast<IOWorkLoop*>(work_loop) == reinterpret_cast<IOWorkLoop*>(1)) {
            // Spin around the cntrlSync variable until the
            // initialization finishes.
            thread_block(0);
        }
    }
    
    return work_loop;
}


void VoodooSMBusDeviceNub::releaseResources() {
    if (command_gate) {
        work_loop->removeEventSource(command_gate);
        command_gate->release();
        command_gate = NULL;
    }
    
    if (work_loop) {
        work_loop->release();
        work_loop = NULL;
    }
}

bool VoodooSMBusDeviceNub::start(IOService* provider) {
    if (!super::start(provider))
        return false;
    
    work_loop = getWorkLoop();
    
    if (!work_loop) {
        IOLog("%s Could not get work loop\n", getName());
        goto exit;
    }
    
    work_loop->retain();
    
    command_gate = IOCommandGate::commandGate(this);
    if (!command_gate || (work_loop->addEventSource(command_gate) != kIOReturnSuccess)) {
        IOLog("%s Could not open command gate\n", getName());
        goto exit;
    }
    

    registerService();
    return true;
exit:
    releaseResources();
    return false;
}

void VoodooSMBusDeviceNub::stop(IOService* provider) {
    super::stop(provider);
}
