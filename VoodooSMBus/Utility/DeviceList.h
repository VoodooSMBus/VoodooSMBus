//
//  deviceList.h
//  VoodooSMBus
//
//  Created by Avery Black on 11/10/21.
//  Copyright © 2021 leo-labs. All rights reserved.
//

#ifndef deviceList_h
#define deviceList_h

#include "../VoodooSMBusDeviceNub.hpp"

struct DeviceListNode {
    VoodooSMBusDeviceNub *nub;
    DeviceListNode *next;
    u8 addr;
};

typedef DeviceListNode DeviceList;

/*
 * Create empty linked list to store nubs
 */
DeviceList *createEmptyList();

/*
 * Insert nub into the tree
 */
bool insertDevice(DeviceList *list, VoodooSMBusDeviceNub *nub, u8 addr);

/*
 * Remove nub from the list
 * Caller must free nub
 */
VoodooSMBusDeviceNub *deleteDevice(DeviceList *list, u8 addr);

/*
 * Get nub from address
 */
VoodooSMBusDeviceNub *getDevice(DeviceList *root, u8 addr);

/*
 * Free tree resources, as well as any nubs contained
 */
void deleteTree(DeviceList *root);

#endif /* deviceList_h */
