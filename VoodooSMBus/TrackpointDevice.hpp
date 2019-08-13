/*
 * TrackpointDevice.hpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */

#ifndef TrackpointDevice_hpp
#define TrackpointDevice_hpp

#include <IOKit/hidsystem/IOHIPointing.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include "helpers.hpp"

class TrackpointDevice : public IOHIPointing {
    typedef IOHIPointing super;
    OSDeclareDefaultStructors(TrackpointDevice);
protected:
    virtual IOItemCount buttonCount() override;
    virtual IOFixed resolution() override;
    
    
public:
    bool start(IOService* provider) override;
    void stop(IOService* provider) override;
    
    virtual UInt32 deviceType() override;
    virtual UInt32 interfaceID() override;
    
    void updateRelativePointer(int dx, int dy, int buttons);
    void updateScrollwheel(short deltaAxis1, short deltaAxis2, short deltaAxis3);

};
#endif /* TrackpointDevice_hpp */
