//
//  RMIBus.h
//  VoodooSMBus
//
//  Created by Sheika Slate on 4/30/20.
//  Copyright © 2020 leo-labs. All rights reserved.
//

#ifndef RMIBus_h
#define RMIBus_h
#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include "VoodooSMBusSlaveDeviceDriver.hpp"
#include "VoodooSMBusDeviceNub.hpp"
#include "rmi_smbus.cpp"

class RMIBus : public VoodooSMBusSlaveDeviceDriver {
    OSDeclareDefaultStructors(RMIBus);
    
public:
    RMIBus * probe(IOService *provider, SInt32 *score) override;
    void handleHostNotify () override;
    bool init(OSDictionary *dictionary) override;
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    VoodooSMBusDeviceNub* device_nub;
    
private:
//    VoodooSMBusDeviceNub* device_nub;
};
    
#endif /* RMIBus_h */
