/*
 * VoodooSMBusSlaveDeviceDriver.cpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */
#include "VoodooSMBusSlaveDeviceDriver.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooSMBusSlaveDeviceDriver, IOService);
