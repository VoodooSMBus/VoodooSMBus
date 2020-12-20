// SPDX-License-Identifier: GPL-2.0-or-later
/*
 Copyright (c) 2019 Leonard Kleinhans
 ported to macOS X from linux kernel driver, original source at
 https://github.com/torvalds/linux/blob/master/drivers/i2c/busses/i2c-i801.c
 
 Copyright (c) 1998 - 2002  Frodo Looijaard <frodol@dds.nl>,
 Philip Edelbrock <phil@netroedge.com>, and Mark D. Studebaker
 <mdsxyz123@yahoo.com>
 Copyright (C) 2007 - 2014  Jean Delvare <jdelvare@suse.de>
 Copyright (C) 2010         Intel Corporation,
 David Woodhouse <dwmw2@infradead.org>
 */

#ifndef i2c_i801_h
#define i2c_i801_h

#include "helpers.hpp"
#include "i2c_smbus.h"
#include <IOKit/IOCommandGate.h>

/* I801 SMBus address offsets */
#define SMBHSTSTS(p)    (0 + (p)->smba)
#define SMBHSTCNT(p)    (2 + (p)->smba)
#define SMBHSTCMD(p)    (3 + (p)->smba)
#define SMBHSTADD(p)    (4 + (p)->smba)
#define SMBHSTDAT0(p)   (5 + (p)->smba)
#define SMBHSTDAT1(p)   (6 + (p)->smba)
#define SMBBLKDAT(p)    (7 + (p)->smba)
#define SMBPEC(p)       (8 + (p)->smba)     /* ICH3 and later */
#define SMBAUXSTS(p)    (12 + (p)->smba)    /* ICH4 and later */
#define SMBAUXCTL(p)    (13 + (p)->smba)    /* ICH4 and later */
#define SMBSLVSTS(p)    (16 + (p)->smba)    /* ICH3 and later */
#define SMBSLVCMD(p)    (17 + (p)->smba)    /* ICH3 and later */
#define SMBNTFDADD(p)   (20 + (p)->smba)    /* ICH3 and later */

/* PCI Address Constants */
#define SMBBAR                  4
#define SMBPCICTL               0x004
#define SMBPCISTS               0x006
#define SMBHSTCFG               0x040
#define TCOBASE                 0x050
#define TCOCTL                  0x054

#define ACPIBASE                0x040
#define ACPIBASE_SMI_OFF        0x030
#define ACPICTRL                0x044
#define ACPICTRL_EN             0x080

#define SBREG_BAR               0x10
#define SBREG_SMBCTRL           0xc6000c
#define SBREG_SMBCTRL_DNV       0xcf000c

/* Host status bits for SMBPCISTS */
#define SMBPCISTS_INTS          BIT(3)

/* Control bits for SMBPCICTL */
#define SMBPCICTL_INTDIS        BIT(10)

/* Host configuration bits for SMBHSTCFG */
#define SMBHSTCFG_HST_EN        BIT(0)
#define SMBHSTCFG_SMB_SMI_EN    BIT(1)
#define SMBHSTCFG_I2C_EN        BIT(2)
#define SMBHSTCFG_SPD_WD        BIT(4)

/* TCO configuration bits for TCOCTL */
#define TCOCTL_EN               BIT(8)

/* Auxiliary status register bits, ICH4+ only */
#define SMBAUXSTS_CRCE          BIT(0)
#define SMBAUXSTS_STCO          BIT(1)

/* Auxiliary control register bits, ICH4+ only */
#define SMBAUXCTL_CRC           BIT(0)
#define SMBAUXCTL_E32B          BIT(1)

/* Other settings */
#define MAX_RETRIES             400

/* I801 command constants */
#define I801_QUICK              0x00
#define I801_BYTE               0x04
#define I801_BYTE_DATA          0x08
#define I801_WORD_DATA          0x0C
#define I801_PROC_CALL          0x10    /* unimplemented */
#define I801_BLOCK_DATA         0x14
#define I801_I2C_BLOCK_DATA     0x18    /* ICH5 and later */

/* I801 Host Control register bits */
#define SMBHSTCNT_INTREN        BIT(0)
#define SMBHSTCNT_KILL          BIT(1)
#define SMBHSTCNT_LAST_BYTE     BIT(5)
#define SMBHSTCNT_START         BIT(6)
#define SMBHSTCNT_PEC_EN        BIT(7)    /* ICH3 and later */

/* I801 Hosts Status register bits */
#define SMBHSTSTS_BYTE_DONE     BIT(7)
#define SMBHSTSTS_INUSE_STS     BIT(6)
#define SMBHSTSTS_SMBALERT_STS  BIT(5)
#define SMBHSTSTS_FAILED        BIT(4)
#define SMBHSTSTS_BUS_ERR       BIT(3)
#define SMBHSTSTS_DEV_ERR       BIT(2)
#define SMBHSTSTS_INTR          BIT(1)
#define SMBHSTSTS_HOST_BUSY     BIT(0)

/* Host Notify Status register bits */
#define SMBSLVSTS_HST_NTFY_STS  BIT(0)

/* Host Notify Command register bits */
#define SMBSLVCMD_HST_NTFY_INTREN   BIT(0)

#define STATUS_ERROR_FLAGS    (SMBHSTSTS_FAILED | SMBHSTSTS_BUS_ERR | \
SMBHSTSTS_DEV_ERR)

#define STATUS_FLAGS        (SMBHSTSTS_BYTE_DONE | SMBHSTSTS_INTR | \
STATUS_ERROR_FLAGS)

#define FEATURE_SMBUS_PEC           BIT(0)
#define FEATURE_BLOCK_BUFFER        BIT(1)
#define FEATURE_BLOCK_PROC          BIT(2)
#define FEATURE_I2C_BLOCK_READ      BIT(3)
#define FEATURE_IRQ                 BIT(4)
#define FEATURE_HOST_NOTIFY         BIT(5)
/* Not really a feature, but it's convenient to handle it as such */
#define FEATURE_IDF                 BIT(15)
#define FEATURE_TCO                 BIT(16)

#define ICH_SMB_BASE                0x20

/* An SMBus device on a PCI controller */
/* This is a mix of i2c_adapter and i801_priv */
struct i801_adapter {
    const char* name;
    IOPCIDevice* pci_device;
    IOCommandGate* command_gate;
    unsigned long smba;
    UInt8 original_slvcmd;
    UInt8 original_hstcfg;
    int retries;
    int timeout;                /* in ns */
    unsigned int features;
    u8 status;
    
    /* Command state used by isr for byte-by-byte block transactions */
    u8 cmd;
    bool is_read;
    int count;
    int len;
    u8 *data;
    
    /* helper function to write to PCI device register, behaves like linux' outb_p */
    void outb_p(UInt8 b, UInt16 offset) {
        pci_device->ioWrite8(offset, b);
    }
    
    /* helper function to read from PCI device register, behaves like linux' inb_p */
    UInt8 inb_p(UInt16 offset) {
        return pci_device->ioRead8(offset);
    }
};
struct VoodooSMBusSlaveDevice {
    UInt8 addr;
    UInt8 flags;
};

void i801_isr_byte_done(struct i801_adapter *priv);

s32 i801_access(struct i801_adapter *priv, u16 addr,
                unsigned short flags, char read_write, u8 command,
                int size, union i2c_smbus_data *data);

#endif /* i2c_i801_h */
