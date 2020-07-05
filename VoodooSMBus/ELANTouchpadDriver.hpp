/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * ELANTouchpadDriver.hpp
 * Elan I2C/SMBus Touchpad driver port for macOS X
 *
 * Copyright (c) 2019 Leonard Kleinhans <leo-labs>
 *
 * Based on linux driver:
 ********************************************************************************
 *
 * Elan I2C/SMBus Touchpad driver
 *
 * Copyright (c) 2013 ELAN Microelectronics Corp.
 *
 * Author: 林政維 (Duson Lin) <dusonlin@emc.com.tw>
 *
 * Based on cyapa driver:
 * copyright (c) 2011-2012 Cypress Semiconductor, Inc.
 * copyright (c) 2011-2012 Google, Inc.
 *
 * Trademarks are the property of their respective owners.
 */

#ifndef ELANTouchpadDriver_hpp
#define ELANTouchpadDriver_hpp


#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "VoodooSMBusDeviceNub.hpp"
#include "i2c_smbus.h"
#include "helpers.hpp"
#include "TrackpointDevice.hpp"
#include "Configuration.hpp"
#include "../Dependencies/VoodooI2C/Multitouch Support/VoodooI2CMultitouchInterface.hpp"

/* https://github.com/torvalds/linux/blob/master/drivers/input/mouse/elan_i2c.h */
#define ETP_ENABLE_ABS                      0x0001
#define ETP_ENABLE_CALIBRATE                0x0002
#define ETP_DISABLE_CALIBRATE               0x0000
#define ETP_DISABLE_POWER                   0x0001
#define ETP_PRESSURE_OFFSET                 25


/* Elan SMbus commands */
/* from https://github.com/torvalds/linux/blob/master/drivers/input/mouse/elan_i2c_smbus.c */
#define ETP_SMBUS_IAP_CMD                   0x00
#define ETP_SMBUS_ENABLE_TP                 0x20
#define ETP_SMBUS_SLEEP_CMD                 0x21
#define ETP_SMBUS_IAP_PASSWORD_WRITE        0x29
#define ETP_SMBUS_IAP_PASSWORD_READ         0x80
#define ETP_SMBUS_WRITE_FW_BLOCK            0x2A
#define ETP_SMBUS_IAP_RESET_CMD             0x2B
#define ETP_SMBUS_RANGE_CMD                 0xA0
#define ETP_SMBUS_FW_VERSION_CMD            0xA1
#define ETP_SMBUS_XY_TRACENUM_CMD           0xA2
#define ETP_SMBUS_SM_VERSION_CMD            0xA3
#define ETP_SMBUS_UNIQUEID_CMD              0xA3
#define ETP_SMBUS_RESOLUTION_CMD            0xA4
#define ETP_SMBUS_HELLOPACKET_CMD           0xA7
#define ETP_SMBUS_PACKET_QUERY              0xA8
#define ETP_SMBUS_IAP_VERSION_CMD           0xAC
#define ETP_SMBUS_IAP_CTRL_CMD              0xAD
#define ETP_SMBUS_IAP_CHECKSUM_CMD          0xAE
#define ETP_SMBUS_FW_CHECKSUM_CMD           0xAF
#define ETP_SMBUS_MAX_BASELINE_CMD          0xC3
#define ETP_SMBUS_MIN_BASELINE_CMD          0xC4
#define ETP_SMBUS_CALIBRATE_QUERY           0xC5

#define ETP_SMBUS_REPORT_LEN                32
#define ETP_SMBUS_REPORT_OFFSET             2
#define ETP_SMBUS_HELLOPACKET_LEN           5
#define ETP_SMBUS_IAP_PASSWORD              0x1234
#define ETP_SMBUS_IAP_MODE_ON               (1 << 6)

#define ELAN_VENDOR_ID                      0x04f3
#define ETP_MAX_PRESSURE                    255
#define ETP_FWIDTH_REDUCE                   90
#define ETP_FINGER_WIDTH                    15
#define ETP_RETRY_COUNT                     10

#define ETP_MAX_FINGERS                     5
#define ETP_FINGER_DATA_LEN                 5
#define ETP_REPORT_ID                       0x5D
#define ETP_TP_REPORT_ID                    0x5E
#define ETP_REPORT_ID_OFFSET                2
#define ETP_TOUCH_INFO_OFFSET               3
#define ETP_FINGER_DATA_OFFSET              4
#define ETP_HOVER_INFO_OFFSET               30
#define ETP_MAX_REPORT_LEN                  34

struct elan_tp_data {
    unsigned int        max_x;
    unsigned int        max_y;
    unsigned int        width_x;
    unsigned int        width_y;
    unsigned int        x_res;
    unsigned int        y_res;
    
    int                 pressure_adjustment;
};

// Message types defined by ApplePS2Keyboard
enum {
    // from keyboard to mouse/touchpad
    kKeyboardSetTouchStatus = iokit_vendor_specific_msg(100),   // set disable/enable touchpad (data is bool*)
    kKeyboardGetTouchStatus = iokit_vendor_specific_msg(101),   // get disable/enable touchpad (data is bool*)
    kKeyboardKeyPressTime = iokit_vendor_specific_msg(110)      // notify of timestamp a non-modifier key was pressed (data is uint64_t*)
};


class ELANTouchpadDriver : public IOService {
    OSDeclareDefaultStructors(ELANTouchpadDriver);
    
public:
    
    bool start(IOService* provider) override;
    void stop(IOService* provider) override;
    ELANTouchpadDriver* probe(IOService* provider, SInt32* score) override;
    void handleHostNotify();
    bool init(OSDictionary *dict) override;
    void free(void) override;
    IOReturn setPowerState(unsigned long whichState, IOService* whatDevice) override;

private:
    void loadConfiguration();
    VoodooSMBusDeviceNub* device_nub;
    VoodooI2CMultitouchInterface *mt_interface;
    TrackpointDevice *trackpoint;
    OSArray* transducers;
    elan_tp_data* data;
    bool awake;
    bool trackpointScrolling;
    
    static constexpr const char* CONFIG_DISABLE_WHILE_TYPING = "DisableWhileTyping";
    static constexpr const char* CONFIG_DISABLE_WHILE_TRACKPOINT = "DisableWhileTrackpoint";
    static constexpr const char* CONFIG_DISABLE_WHILE_TYPING_TIMEOUT_MS = "DisableWhileTypingTimeoutMs";
    static constexpr const char* CONFIG_DISABLE_WHILE_TRACKPOINT_TIMEOUT_MS = "DisableWhileTrackpointTimeoutMs";
    static constexpr const char* CONFIG_IGNORE_SET_TOUCHPAD_STATUS = "IgnoreSetTouchpadStatus";
    
    bool disable_while_typing;
    bool disable_while_trackpoint;
    bool ignore_set_touchpad_status;
    uint64_t disable_while_typing_timeout;
    uint64_t disable_while_trackpoint_timeout;
    
    bool ignoreall;
    uint64_t ts_last_keyboard = 0;
    uint64_t ts_last_trackpoint = 0;

    void releaseResources();
    void unpublishMultitouchInterface();
    bool publishMultitouchInterface();
    
    void unpublishTrackpoint();
    bool publishTrackpoint();

    /* ELAN device functions */
    int tryInitialize();
    int initialize();
    int getReport(u8 *report);
    void reportTrackpoint(u8 *report);
    static unsigned int convertResolution(u8 val);
    int setMode(u8 mode);
    bool setDeviceParameters();
    void reportContact(VoodooI2CDigitiserTransducer* transducer, bool contact_valid, u8 *finger_data, AbsoluteTime timestamp);
    void reportAbsolute(u8 *packet);
    void sendSleepCommand();
    
    /*
     * Called by ApplePS2Controller to notify of keyboard interactions
     * @type Custom message type in iokit_vendor_specific_msg range
     * @provider Calling IOService
     * @argument Optional argument as defined by message type
     *
     * @return kIOSuccess if the message is processed
     */
    virtual IOReturn message(UInt32 type, IOService* provider, void* argument);
};

#endif /* ELANTouchpadDriver_hpp */
