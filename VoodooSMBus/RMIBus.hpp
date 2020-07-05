//
//  RMIBus.h
//  VoodooSMBus
//
//  Created by Sheika Slate on 4/30/20.
//  Copyright © 2020 leo-labs. All rights reserved.
//

#ifndef RMIBus_h
#define RMIBus_h
#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include "VoodooSMBusSlaveDeviceDriver.hpp"
#include "VoodooSMBusDeviceNub.hpp"
#include "rmi_smbus.cpp"

#define RMI_F30_QUERY_SIZE            2

/* Defs for Query 0 */
#define RMI_F30_EXTENDED_PATTERNS        0x01
#define RMI_F30_HAS_MAPPABLE_BUTTONS        BIT(1)
#define RMI_F30_HAS_LED                BIT(2)
#define RMI_F30_HAS_GPIO            BIT(3)
#define RMI_F30_HAS_HAPTIC            BIT(4)
#define RMI_F30_HAS_GPIO_DRV_CTL        BIT(5)
#define RMI_F30_HAS_MECH_MOUSE_BTNS        BIT(6)

/* Defs for Query 1 */
#define RMI_F30_GPIO_LED_COUNT            0x1F

/* Defs for Control Registers */
#define RMI_F30_CTRL_1_GPIO_DEBOUNCE        0x01
#define RMI_F30_CTRL_1_HALT            BIT(4)
#define RMI_F30_CTRL_1_HALTED            BIT(5)
#define RMI_F30_CTRL_10_NUM_MECH_MOUSE_BTNS    0x03

#define RMI_F30_CTRL_MAX_REGS        32
#define RMI_F30_CTRL_MAX_BYTES        RMI_F30_CTRL_MAX_REGS / 8
#define RMI_F30_CTRL_MAX_REG_BLOCKS    11

#define RMI_F30_CTRL_REGS_MAX_SIZE (RMI_F30_CTRL_MAX_BYTES        \
                    + 1                \
                    + RMI_F30_CTRL_MAX_BYTES    \
                    + RMI_F30_CTRL_MAX_BYTES    \
                    + RMI_F30_CTRL_MAX_BYTES    \
                    + 6                \
                    + RMI_F30_CTRL_MAX_REGS        \
                    + RMI_F30_CTRL_MAX_REGS        \
                    + RMI_F30_CTRL_MAX_BYTES    \
                    + 1                \
                    + 1)

#define TRACKSTICK_RANGE_START        3
#define TRACKSTICK_RANGE_END        6

struct rmi_f30_ctrl_data {
    int address;
    int length;
    u8 *regs;
};

struct f30_data {
    /* Query Data */
    bool has_extended_pattern;
    bool has_mappable_buttons;
    bool has_led;
    bool has_gpio;
    bool has_haptic;
    bool has_gpio_driver_control;
    bool has_mech_mouse_btns;
    u8 gpioled_count;

    u8 register_count;

    /* Control Register Data */
    struct rmi_f30_ctrl_data ctrl[RMI_F30_CTRL_MAX_REG_BLOCKS];
    u8 ctrl_regs[RMI_F30_CTRL_REGS_MAX_SIZE];
    uint32_t ctrl_regs_size;

    u8 data_regs[RMI_F30_CTRL_MAX_BYTES];
    u16 *gpioled_key_map;

    struct input_dev *input;

    struct rmi_function *f03;
    bool trackstick_buttons;
};

class RMIBus : public VoodooSMBusSlaveDeviceDriver {
    OSDeclareDefaultStructors(RMIBus);
    
public:
    RMIBus * probe(IOService *provider, SInt32 *score) override;
    void handleHostNotify () override;
    bool init(OSDictionary *dictionary) override;
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    VoodooSMBusDeviceNub* device_nub;
    
private:
    void initialize();
//    VoodooSMBusDeviceNub* device_nub;
};
    
#endif /* RMIBus_h */
