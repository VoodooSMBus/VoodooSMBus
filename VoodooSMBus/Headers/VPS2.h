//
//  VPS2.h
//  VoodooSMBus
//
//  Created by Avery Black on 11/9/21.
//  Copyright © 2021 Avery Black. All rights reserved.
//

#ifndef VPS2_h
#define VPS2_h

#include <IOKit/IOLib.h>

#define kControlNotifications   "PS2,controlNotifications"

enum {
    kPS2C_deviceDiscovered = iokit_vendor_specific_msg(300),    // SMBus device discovered on PS2 side, try to init SMBus side
    kPS2C_wakeCompleted = iokit_vendor_specific_msg(301),       // PS2 Wakeup completed
};

#endif /* VPS2_h */
