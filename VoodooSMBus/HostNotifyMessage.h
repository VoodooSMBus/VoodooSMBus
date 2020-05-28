/*
 * HostNotifyMessage.h
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2020 Leonard Kleinhans <leo-labs>
 *
 */

#ifndef HostNotifyMessage_h
#define HostNotifyMessage_h

#define kIOMessageVoodooSMBusHostNotify iokit_vendor_specific_msg(420)

// an empty message that is sent from `VoodooSMBusControllerDriver to a slave device driver indicating that
// a host notify interrupt for the address of the slave device occurred.
struct  HostNotifyMessage {
};

#endif /* HostNotifyMessage_h */
