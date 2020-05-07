//
//  rmi_smbus.c
//  VoodooSMBus
//
//  Created by Gwy on 5/4/20.
//  Copyright © 2020 leo-labs. All rights reserved.
//

#include "rmi_smbus.hpp"

#define SMB_PROTOCOL_VERSION_ADDRESS 0xfd

static int rmi_smb_get_version(VoodooSMBusDeviceNub* dev)
{
    int retval;
    
    /* Check if for SMBus new version device by reading version byte. */
    retval = dev->readByteData(SMB_PROTOCOL_VERSION_ADDRESS);
    if (retval < 0) {
        IOLog("Failed to get SMBus version number!\n");
        return retval;
    }
    
    return retval + 1;
}

static int smb_block_write(VoodooSMBusDeviceNub* dev, u8 command, u8 *buf, size_t len)
{
    int retval;
    
    retval = dev->writeBlockData(command, len, buf);
    if (retval < 0) {
        IOLog("Failed to write block to SMBus");
//        return retval;
    }
    
    return retval;
}
