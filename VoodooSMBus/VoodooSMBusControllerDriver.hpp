/*
 * VoodooSMBusControllerDriver.hpp
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

#ifndef VoodooSMBusControllerDriver_hpp
#define VoodooSMBusControllerDriver_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/IOFilterInterruptEventSource.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/IOPlatformExpert.h>
#include "i2c_i801.cpp"
#include "VoodooSMBusDeviceNub.hpp"
#include "HostNotifyMessage.h"

#ifndef __ACIDANTHERA_MAC_SDK
#error "No Acidanthera SDK"
#endif

/* Helper struct so we are able to pass more than 4 arguments to `transferGated(..)` */
typedef struct  {
    VoodooSMBusSlaveDevice* slave_device;
    char read_write;
    u8 command;
    int protocol;
} VoodooSMBusControllerMessage;


class VoodooSMBusControllerDriver : public IOService {
    OSDeclareDefaultStructors(VoodooSMBusControllerDriver)
public:
    IOPCIDevice* pci_device;
    i801_adapter* adapter;
    OSDictionary* device_nubs;
    OSArray* addresses;
    
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    virtual IOService *probe(IOService *provider, SInt32 *score) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    IOReturn setPowerState(unsigned long whichState, IOService* whatDevice) override;

    IOWorkLoop* getWorkLoop();
    void handleInterrupt(OSObject* owner, IOInterruptEventSource* src, int intCount);

    /**
     * readByteData - SMBus "read byte" protocol
     * @client: Handle to slave device
     * @command: Byte interpreted by slave
     *
     * This executes the SMBus "read byte" protocol, returning negative errno
     * else the data byte received from the device.
     */
    IOReturn readByteData(VoodooSMBusSlaveDevice *client, u8 command);
    
    /**
     * readBlockData - SMBus "block read" protocol
     * @client: Handle to slave device
     * @command: Byte interpreted by slave
     * @values: Byte array into which data will be read; big enough to hold
     *    the data returned by the slave.  SMBus allows at most 32 bytes.
     *
     * This executes the SMBus "block read" protocol, returning negative errno
     * else the number of data bytes in the slave's response.
     *
     * Note that using this function requires that the client's adapter support
     * the I2C_FUNC_SMBUS_READ_BLOCK_DATA functionality.  Not all adapter drivers
     * support this; its emulation through I2C messaging relies on a specific
     * mechanism (I2C_M_RECV_LEN) which may not be implemented.
     */
    IOReturn readBlockData(VoodooSMBusSlaveDevice *client, u8 command, u8 *values);
    
    /**
     * writeByteData - SMBus "write byte" protocol
     * @client: Handle to slave device
     * @command: Byte interpreted by slave
     * @value: Byte being written
     *
     * This executes the SMBus "write byte" protocol, returning negative errno
     * else zero on success.
     */
    
    IOReturn writeByteData(VoodooSMBusSlaveDevice *client, u8 command, u8 value);
    
    /**
     * writeByte - SMBus "send byte" protocol
     * @client: Handle to slave device
     * @value: Byte to be sent
     *
     * This executes the SMBus "send byte" protocol, returning negative errno
     * else zero on success.
     */
    IOReturn writeByte(VoodooSMBusSlaveDevice *client, u8 value);
    
    /**
     * writeBlockData - SMBus "block write" protocol
     * @client: Handle to slave device
     * @command: Byte interpreted by slave
     * @length: Size of data block; SMBus allows at most 32 bytes
     * @values: Byte array which will be written.
     *
     * This executes the SMBus "block write" protocol, returning negative errno
     * else zero on success.
     */
    IOReturn writeBlockData(VoodooSMBusSlaveDevice *client, u8 command, u8 length, const u8 *values);
    
    /**
     * transfer - execute SMBus protocol operations
     * @adapter: Handle to I2C bus
     * @addr: Address of SMBus slave on that bus
     * @flags: I2C_CLIENT_* flags (usually zero or I2C_CLIENT_PEC)
     * @read_write: I2C_SMBUS_READ or I2C_SMBUS_WRITE
     * @command: Byte interpreted by slave, for protocols which use such bytes
     * @protocol: SMBus protocol operation to execute, such as I2C_SMBUS_PROC_CALL
     * @data: Data to be read or written
     *
     * This executes an SMBus protocol operation, and returns a negative
     * errno code else zero on success.
     */
    IOReturn transfer(VoodooSMBusSlaveDevice *client, char read_write, u8 command, int protocol, union i2c_smbus_data *data);
    
    
private:
    IOCommandGate* command_gate;
    IOWorkLoop* work_loop;
    IOInterruptEventSource* interrupt_source;
    bool awake;
    
    IOReturn publishNub(UInt8 address);
    IOReturn publishMultipleNubs();
    void releaseResources();
    
    void enableHostNotify();
    void disableHostNotify();
    
    void disableCommandGate();
    
    IOReturn transferGated(VoodooSMBusControllerMessage *message, union i2c_smbus_data *data);

};

#endif /* VoodooSMBusControllerDriver_hpp */
