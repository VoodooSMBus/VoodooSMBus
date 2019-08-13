/*
 * TrackpointDevice.cpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */

#include "TrackpointDevice.hpp"

OSDefineMetaClassAndStructors(TrackpointDevice, IOHIPointing);

UInt32 TrackpointDevice::deviceType() {
    return NX_EVS_DEVICE_TYPE_MOUSE;
}

UInt32 TrackpointDevice::interfaceID() {
    return NX_EVS_DEVICE_INTERFACE_BUS_ACE;
}

IOItemCount TrackpointDevice::buttonCount() {
    return 3;
};

IOFixed TrackpointDevice::resolution() {
    return (150) << 16;
};

bool TrackpointDevice::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }

    registerService();
    return true;
}

void TrackpointDevice::stop(IOService* provider) {
    super::stop(provider);
}


void TrackpointDevice::updateRelativePointer(int dx, int dy, int buttons) {
    uint64_t now_abs;
    clock_get_uptime(&now_abs);
    dispatchRelativePointerEvent(dx, dy, buttons, now_abs);
};

void TrackpointDevice::updateScrollwheel(short deltaAxis1, short deltaAxis2, short deltaAxis3) {
    uint64_t now_abs;
    clock_get_uptime(&now_abs);
    dispatchScrollWheelEvent(deltaAxis1, deltaAxis2, deltaAxis3, now_abs);
    
}



