
#ifndef VoodooSMBusControllerDriver_hpp
#define VoodooSMBusControllerDriver_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/IOFilterInterruptEventSource.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/IOPlatformExpert.h>
#include "helpers.hpp"
#include "i2c_i801.cpp"
#include "VoodooSMBusDeviceNub.hpp"

#define ELAN_TOUCHPAD_ADDRESS 0x15

#define PrintBitFieldExpanded(val) IOLog("Register decoded: 0x%x<BUSY=%d,INTR=%d," \
"DEVERR=%d,BUSERR=%d,FAILED=%d,SMBAL=%d,INUSE=%d,BDONE=%d>\n", val, \
(val & SMBHSTSTS_HOST_BUSY) != 0, (val & SMBHSTSTS_INTR) != 0, \
(val & SMBHSTSTS_DEV_ERR) != 0, (val & SMBHSTSTS_BUS_ERR) != 0, \
(val & SMBHSTSTS_FAILED) != 0, (val & SMBHSTSTS_SMBALERT_STS) != 0, \
(val & SMBHSTSTS_INUSE_STS) != 0, (val & SMBHSTSTS_BYTE_DONE) != 0)

class VoodooSMBusControllerDriver : public IOService {
    OSDeclareDefaultStructors(VoodooSMBusControllerDriver)
public:
    IOPCIDevice* pci_device;
    
    i801_adapter* adapter;
    OSDictionary* device_nubs;
    
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    virtual IOService *probe(IOService *provider, SInt32 *score) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    IOReturn setPowerState(unsigned long whichState, IOService* whatDevice);

    IOWorkLoop* getWorkLoop();
    void handleInterrupt(OSObject* owner, IOInterruptEventSource* src, int intCount);
    bool filterInterrupt(OSObject *owner, IOFilterInterruptEventSource *sender);

    // i2c_smbus_read_block_data
    IOReturn ReadBlockData(VoodooSMBusSlaveDevice *client, u8 command, u8 *values);
    
    /**
     * i2c_smbus_write_byte_data - SMBus "write byte" protocol
     * @client: Handle to slave device
     * @command: Byte interpreted by slave
     * @value: Byte being written
     *
     * This executes the SMBus "write byte" protocol, returning negative errno
     * else zero on success.
     */
    IOReturn WriteByteData(VoodooSMBusSlaveDevice *client, u8 command, u8 value);
    
    /**
     * i2c_smbus_write_byte - SMBus "send byte" protocol
     * @client: Handle to slave device
     * @value: Byte to be sent
     *
     * This executes the SMBus "send byte" protocol, returning negative errno
     * else zero on success.
     */
    IOReturn WriteByte(VoodooSMBusSlaveDevice *client, u8 value);
    
    /**
     * i2c_smbus_write_block_data - SMBus "block write" protocol
     * @client: Handle to slave device
     * @command: Byte interpreted by slave
     * @length: Size of data block; SMBus allows at most 32 bytes
     * @values: Byte array which will be written.
     *
     * This executes the SMBus "block write" protocol, returning negative errno
     * else zero on success.
     */
    IOReturn writeBlockData(VoodooSMBusSlaveDevice *client, u8 command, u8 length, const u8 *values);
    
    
private:
    IOCommandGate* command_gate;
    IOWorkLoop* work_loop;
    IOInterruptEventSource* interrupt_source;

    IOReturn publishNub(UInt8 address);
    void releaseResources();
    
    void enableHostNotify();
    void disableHostNotify();
    
    /**
     * i2c_smbus_xfer - execute SMBus protocol operations
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
    IOReturn transferGated(VoodooSMBusControllerMessage *message, union i2c_smbus_data *data, IOCommandGate* command_gate);

};

#endif /* VoodooSMBusControllerDriver_hpp */
