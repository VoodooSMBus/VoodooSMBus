//
//  deviceList.c
//  VoodooSMBus
//
//  Created by Avery Black on 11/10/21.
//  Copyright © 2021 leo-labs. All rights reserved.
//

#include "DeviceList.h"

DeviceList *createEmptyList() {
    DeviceList *listHead = new DeviceListNode();
    memset(listHead, 0, sizeof(DeviceList));
    return listHead;
}

bool insertDevice(DeviceList *node, VoodooSMBusDeviceNub *nub, u8 addr) {
    DeviceListNode *newNode = new DeviceListNode();
    if (newNode == nullptr) {
        return false;
    }
    
    nub->retain();
    newNode->addr = addr;
    newNode->nub = nub;
    newNode->next = nullptr;
    
    while (node->next != nullptr) {
        node = node->next;
    }
    
    node->next = newNode;
    return true;
}

VoodooSMBusDeviceNub *deleteDevice(DeviceList *node, u8 addr) {
    DeviceListNode *nextNode = node->next;
    while (nextNode != nullptr && nextNode->addr != addr) {
        node = nextNode;
        nextNode = nextNode->next;
    }
    
    if (nextNode == nullptr) {
        return nullptr;
    }
    
    node->next = nextNode->next;
    VoodooSMBusDeviceNub *nub = nextNode->nub;
    delete nextNode;
    return nub;
}

VoodooSMBusDeviceNub *getDevice(DeviceList *node, u8 addr) {
    while (node != nullptr && node->addr != addr) {
        node = node->next;
    }
    
    if (node == nullptr) {
        return nullptr;
    }
    
    return node->nub;
}

void deleteTree(DeviceList *node) {
    if (node == nullptr) {
        return;
    }
    
    deleteTree(node->next);
    
    // Release nub so that it can terminate
    OSSafeReleaseNULL(node->nub);
    delete node;
}
