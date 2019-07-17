/*
 * VoodooSMBusIntelLpssI2C.cpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 */

#include "VoodooSMBusIntelLpssI2C.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooSMBusIntelLpssI2C, IOService);
