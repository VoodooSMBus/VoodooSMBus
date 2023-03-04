/*
 * VoodooSMBusControllerDriver.cpp
 * SMBus Controller Driver for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 * some functions are ported from the linux kernel driver at:
 * https://github.com/torvalds/linux/blob/master/drivers/i2c/i2c-core-smbus.c
 * by Frodo Looijaard <frodol@dds.nl>
 * by Mark Studebaker <mdsxyz123@yahoo.com> and
 * Jean Delvare <jdelvare@suse.de>
 */

#include "VoodooSMBusControllerDriver.hpp"

OSDefineMetaClassAndStructors(VoodooSMBusControllerDriver, IOService)

#define super IOService
#define MILLI_TO_NANO 1000000

bool VoodooSMBusControllerDriver::init(OSDictionary *dict) {
    bool result = super::init(dict);
    
    // For now, we support only one slave device
    device_nubs = OSDictionary::withCapacity(1);
    adapter = reinterpret_cast<i801_adapter*>(IOMalloc(sizeof(i801_adapter)));
    awake = true;
    
    return result;
}

void VoodooSMBusControllerDriver::free(void) {
    IOFree(adapter, sizeof(i801_adapter));
    OSSafeReleaseNULL(device_nubs);
    super::free();
}

IOService *VoodooSMBusControllerDriver::probe(IOService *provider, SInt32 *score) {
    IOService *result = super::probe(provider, score);
    return result;
}

bool VoodooSMBusControllerDriver::start(IOService *provider) {
    bool result = super::start(provider);
    
    pci_device = OSDynamicCast(IOPCIDevice, provider);
    
    if (!(pci_device = OSDynamicCast(IOPCIDevice, provider))) {
        IOLogError("Failed to cast provider");
        return false;
    }

    pci_device->setIOEnable(true);
   
    adapter->pci_device = pci_device;
    adapter->name = getMatchedName(provider);
    
    pci_device->retain();
    if (!pci_device->open(this)) {
        IOLogError("%s::%s Could not open provider", getName(), pci_device->getName());
        return false;
    }
    
    uint32_t host_config = pci_device->configRead8(SMBHSTCFG);
    if ((host_config & SMBHSTCFG_HST_EN) == 0) {
        IOLogError("SMBus disabled");
        return false;
    }
    
    adapter->smba = pci_device->configRead16(ICH_SMB_BASE) & 0xFFFE;
    
    if (host_config & SMBHSTCFG_SMB_SMI_EN) {
        IOLogError("No PCI IRQ. Poll mode is not implemented. Unloading.");
        return false;
    }
    
    adapter->original_hstcfg = host_config;
    adapter->original_slvcmd = pci_device->ioRead8(SMBSLVCMD(adapter));
    adapter->features |= FEATURE_I2C_BLOCK_READ;
    adapter->features |= FEATURE_IRQ;
    adapter->features |= FEATURE_SMBUS_PEC;
    adapter->features |= FEATURE_BLOCK_BUFFER;
    adapter->features |= FEATURE_HOST_NOTIFY;
    adapter->retries = 3;
    adapter->timeout = 200 * MILLI_TO_NANO;
    
    work_loop = reinterpret_cast<IOWorkLoop*>(getWorkLoop());
    if (!work_loop) {
        IOLogError("%s Could not get work loop", getName());
        goto exit;
    }
    
    interrupt_source =
    IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooSMBusControllerDriver::handleInterrupt),provider);
    
    if (!interrupt_source || work_loop->addEventSource(interrupt_source) != kIOReturnSuccess) {
        IOLogError("%s Could not add interrupt source to work loop", getName());
        goto exit;
    }
    
    command_gate = IOCommandGate::commandGate(this);
    if (!command_gate || (work_loop->addEventSource(command_gate) != kIOReturnSuccess)) {
        IOLogError("%s Could not open command gate", getName());
        goto exit;
    }
    adapter->command_gate = command_gate;
    work_loop->retain();
    
    PMinit();
    provider->joinPMtree(this);
    registerPowerDriver(this, VoodooSMBusPowerStates, kVoodooSMBusPowerStates);
    pci_device->enablePCIPowerManagement(kPCIPMCSPowerStateD0);

    publishMultipleNubs();
    interrupt_source->enable();
    enableHostNotify();

    registerService();

    return result;
    
exit:
    releaseResources();
    return false;
}

void VoodooSMBusControllerDriver::releaseResources() {
    disableHostNotify();
    pci_device->ioWrite8(SMBHSTCFG, adapter->original_hstcfg);
    
    if (device_nubs) {
        OSCollectionIterator* iterator = OSCollectionIterator::withCollection(device_nubs);
        
        while (VoodooSMBusDeviceNub *device_nub = OSDynamicCast(VoodooSMBusDeviceNub, iterator->getNextObject())) {
            IOLogDebug("Detaching device nub");
            device_nub->detach(this);
        }
        device_nubs->flushCollection();
        OSSafeReleaseNULL(iterator);
    }
    
    if (command_gate) {
        work_loop->removeEventSource(command_gate);
        command_gate->release();
        command_gate = NULL;
    }
    
    if (interrupt_source) {
        interrupt_source->disable();
        work_loop->removeEventSource(interrupt_source);
        interrupt_source->release();
        interrupt_source = NULL;
    }
    
    OSSafeReleaseNULL(work_loop);
    pci_device->close(this);
    OSSafeReleaseNULL(pci_device);
}


void VoodooSMBusControllerDriver::stop(IOService *provider) {
    releaseResources();
    PMstop();
    super::stop(provider);
}

IOReturn VoodooSMBusControllerDriver::setPowerState(unsigned long whichState, IOService* whatDevice) {
    if (whatDevice != this)
        return kIOPMAckImplied;
    
    if (whichState == kIOPMPowerOff) {
        
        disableHostNotify();
        command_gate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooSMBusControllerDriver::disableCommandGate));
        pci_device->ioWrite8(SMBHSTCFG, adapter->original_hstcfg);
        awake = false;

    } else {
        if (!awake) {
            pci_device->enablePCIPowerManagement(kPCIPMCSPowerStateD0);
            command_gate->enable();
            enableHostNotify();
            awake = true;
        }
        
    }
    return kIOPMAckImplied;
}

void VoodooSMBusControllerDriver::disableCommandGate() {
    command_gate->disable();
}

IOReturn VoodooSMBusControllerDriver::publishMultipleNubs() {
    addresses = OSDynamicCast(OSArray, getProperty("Addresses"));
    if (!addresses) {
        return kIOReturnError;
    }
    
    OSIterator *iter = OSCollectionIterator::withCollection(addresses);
    if (!iter) {
        return kIOReturnError;
    }
    
    while (OSNumber *addr = OSDynamicCast(OSNumber, iter->getNextObject()))
    {
        IOReturn res = publishNub(addr->unsigned8BitValue());
        if (res) {
            OSSafeReleaseNULL(iter);
            return res;
        }
    }
    
    OSSafeReleaseNULL(iter);
    
    return kIOReturnSuccess;
}

IOReturn VoodooSMBusControllerDriver::publishNub(UInt8 address) {
    
    VoodooSMBusDeviceNub* device_nub = OSTypeAlloc(VoodooSMBusDeviceNub);
    
    if (!device_nub || !device_nub->init()) {
        IOLogError("%s::%s Could not initialise nub", getName(), adapter->name);
        goto exit;
    }
    
    if (!device_nub->attach(this, address)) {
        IOLogError("%s::%s Could not attach nub", getName(), adapter->name);
        goto exit;
    }
    
    if (!device_nub->start(this)) {
        IOLogError("%s::%s Could not start nub", getName(), adapter->name);
        goto exit;
    }
    
    char key[5];
    addrToDictKey(address, key);
    device_nubs->setObject(key, device_nub);
    IOLogDebug("Publishing nub for slave device at address %#04x", address);

    OSSafeReleaseNULL(device_nub);
    return kIOReturnSuccess;
    
exit:
    OSSafeReleaseNULL(device_nub);
    return kIOReturnError;
}

IOWorkLoop* VoodooSMBusControllerDriver::getWorkLoop() {
    // Do we have a work loop already?, if so return it NOW.
    if ((vm_address_t) work_loop >> 1)
        return work_loop;
    
    if (OSCompareAndSwap(0, 1, reinterpret_cast<IOWorkLoop*>(&work_loop))) {
        // Construct the workloop and set the cntrlSync variable
        // to whatever the result is and return
        work_loop = IOWorkLoop::workLoop();
    } else {
        while (reinterpret_cast<IOWorkLoop*>(work_loop) == reinterpret_cast<IOWorkLoop*>(1)) {
            // Spin around the cntrlSync variable until the
            // initialization finishes.
            thread_block(0);
        }
    }
    
    return work_loop;
}


void VoodooSMBusControllerDriver::handleInterrupt(OSObject* owner, IOInterruptEventSource* src, int intCount) {
    u8 status;

    if (adapter->features & FEATURE_HOST_NOTIFY) {
        status = adapter->inb_p(SMBSLVSTS(adapter));
        if (status & SMBSLVSTS_HST_NTFY_STS) {
            UInt8 addr;
            
            addr = adapter->inb_p(SMBNTFDADD(adapter)) >> 1;
            
            /*
             * With the tested platforms, reading SMBNTFDDAT (22 + (p)->smba)
             * always returns 0. Our current implementation doesn't provide
             * data, so we just ignore it.
             */
            
            char key[5];
            addrToDictKey(addr, key);
            VoodooSMBusDeviceNub* nub = OSDynamicCast(VoodooSMBusDeviceNub, device_nubs->getObject(key));
            if (nub) {
                nub->handleHostNotify();
            } else {
                IOLogError("Received Host Notify Interrupt for unknown device at address %#04x", addr);
            }
            
            /* clear Host Notify bit and return */
            adapter->outb_p(SMBSLVSTS_HST_NTFY_STS, SMBSLVSTS(adapter));
            return;
        }
    }
    
    status = adapter->inb_p(SMBHSTSTS(adapter));

    if (status & SMBHSTSTS_BYTE_DONE) {
        i801_isr_byte_done(adapter);
    }
    
    /*
     * Clear irq sources and report transaction result.
     * ->status must be cleared before the next transaction is started.
     */
    status &= SMBHSTSTS_INTR | STATUS_ERROR_FLAGS;
    if (status) {
        adapter->outb_p(status, SMBHSTSTS(adapter));
        adapter->status = status;
        command_gate->commandWakeup(&adapter->status);
    }
}


void VoodooSMBusControllerDriver::enableHostNotify() {
    
    if(!(adapter->original_slvcmd & SMBSLVCMD_HST_NTFY_INTREN)) {
        pci_device->ioWrite8(SMBSLVCMD(adapter), SMBSLVCMD_HST_NTFY_INTREN | adapter->original_slvcmd);
    }

    /* clear Host Notify bit to allow a new notification */
    pci_device->ioWrite8(SMBSLVSTS(adapter), SMBSLVSTS_HST_NTFY_STS);
}

void VoodooSMBusControllerDriver::disableHostNotify() {
    pci_device->ioWrite8(SMBSLVCMD(adapter), adapter->original_slvcmd);
}

IOReturn VoodooSMBusControllerDriver::readByteData(VoodooSMBusSlaveDevice *client, u8 command) {
    union i2c_smbus_data data;
    IOReturn status;
    
    status = transfer(client, I2C_SMBUS_READ, command, I2C_SMBUS_BYTE_DATA, &data);
    if (status != kIOReturnSuccess)
        return status;
    
    return data.byte;
}

IOReturn VoodooSMBusControllerDriver::readBlockData(VoodooSMBusSlaveDevice *client, u8 command, u8 *values) {
    union i2c_smbus_data data;
    IOReturn status;
    
    status = transfer(client, I2C_SMBUS_READ, command, I2C_SMBUS_BLOCK_DATA, &data);
    if (status != kIOReturnSuccess)
        return status;
    
    memcpy(values, &data.block[1], data.block[0]);
    return data.block[0];
}

IOReturn VoodooSMBusControllerDriver::writeByteData(VoodooSMBusSlaveDevice *client, u8 command, u8 value) {
    union i2c_smbus_data data;
    data.byte = value;
    
    return transfer(client, I2C_SMBUS_WRITE, command, I2C_SMBUS_BYTE_DATA, &data);
}


IOReturn VoodooSMBusControllerDriver::writeByte(VoodooSMBusSlaveDevice *client, u8 value) {
    return transfer(client, I2C_SMBUS_WRITE, value, I2C_SMBUS_BYTE, NULL);
}


IOReturn VoodooSMBusControllerDriver::writeBlockData(VoodooSMBusSlaveDevice *client, u8 command,
                                                     u8 length, const u8 *values) {
    union i2c_smbus_data data;
    
    if (length > I2C_SMBUS_BLOCK_MAX)
        length = I2C_SMBUS_BLOCK_MAX;
    data.block[0] = length;
    memcpy(&data.block[1], values, length);
    return transfer(client, I2C_SMBUS_WRITE, command, I2C_SMBUS_BLOCK_DATA, &data);
}

IOReturn VoodooSMBusControllerDriver::transfer(VoodooSMBusSlaveDevice *client, char  read_write, u8 command, int protocol, union i2c_smbus_data *data) {
    VoodooSMBusControllerMessage message = {
        .slave_device = client,
        .read_write = read_write,
        .command = command,
        .protocol = protocol,
    };
    
    return command_gate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooSMBusControllerDriver::transferGated), &message, data);
}

// __i2c_smbus_xfer
IOReturn VoodooSMBusControllerDriver::transferGated(VoodooSMBusControllerMessage *message, union i2c_smbus_data *data) {
    int _try;
    s32 res;

    VoodooSMBusSlaveDevice* slave_device = message->slave_device;
    slave_device->flags &= I2C_M_TEN | I2C_CLIENT_PEC | I2C_CLIENT_SCCB;
    
    /* Retry automatically on arbitration loss */
    for (res = 0, _try = 0; _try <= adapter->retries; _try++) {
        res = i801_access(adapter, slave_device->addr, slave_device->flags, message->read_write, message->command, message->protocol, data);
        if (res != -EAGAIN)
            break;
    }
    
    return res;
}
