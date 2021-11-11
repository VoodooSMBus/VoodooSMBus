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

/* Make sure the SMBus host is ready to start transmitting.
 Return 0 if it is, -EBUSY if it is not. */
static int i801_check_pre(struct i801_adapter *priv)
{
    int status;
    
    status = priv->inb_p(SMBHSTSTS(priv));
    if (status & SMBHSTSTS_HOST_BUSY) {
        IOLogError("SMBus is busy, can't use it! (%02x)", status);
        return -EBUSY;
    }
    
    status &= STATUS_FLAGS;
    if (status) {
        IOLogDebug("Clearing status flags (%02x)",
                   status);
        priv->outb_p(status, SMBHSTSTS(priv));
        status = priv->inb_p(SMBHSTSTS(priv)) & STATUS_FLAGS;
        if (status) {
            IOLogError("Failed clearing status flags (%02x)",
                       status);
            return -EBUSY;
        }
    }
    
    /*
     * Clear CRC status if needed.
     * During normal operation, i801_check_post() takes care
     * of it after every operation.  We do it here only in case
     * the hardware was already in this state when the driver
     * started.
     */
    if (priv->features & FEATURE_SMBUS_PEC) {
        status = priv->inb_p(SMBAUXSTS(priv)) & SMBAUXSTS_CRCE;
        if (status) {
            IOLogDebug("Clearing aux status flags (%02x)", status);
            priv->outb_p(status, SMBAUXSTS(priv));
            status = priv->inb_p(SMBAUXSTS(priv)) & SMBAUXSTS_CRCE;
            if (status) {
                IOLogError("Failed clearing aux status flags (%02x)",
                           status);
                return -EBUSY;
            }
        }
    }
    
    return 0;
}

/*
 * Convert the status register to an error code, and clear it.
 * Note that status only contains the bits we want to clear, not the
 * actual register value.
 */
static int i801_check_post(struct i801_adapter *priv, int status)
{
    int result = 0;
    
    /*
     * If the SMBus is still busy, we give up
     * Note: This timeout condition only happens when using polling
     * transactions.  For interrupt operation, NAK/timeout is indicated by
     * DEV_ERR.
     */
    if (status < 0) {
        IOLogError("Transaction timeout");
        /* try to stop the current command */
        IOLogDebug("Terminating the current operation");
        priv->outb_p(priv->inb_p(SMBHSTCNT(priv)) | SMBHSTCNT_KILL,
               SMBHSTCNT(priv));
        IODelay(1000);
        priv->outb_p(priv->inb_p(SMBHSTCNT(priv)) & (~SMBHSTCNT_KILL),
               SMBHSTCNT(priv));
        
        /* Check if it worked */
        status = priv->inb_p(SMBHSTSTS(priv));
        if ((status & SMBHSTSTS_HOST_BUSY) ||
            !(status & SMBHSTSTS_FAILED))
            IOLogError("Failed terminating the transaction");
        priv->outb_p(STATUS_FLAGS, SMBHSTSTS(priv));
        return -ETIMEDOUT;
    }
    
    if (status & SMBHSTSTS_FAILED) {
        result = -EIO;
        IOLogError("Transaction failed");
    }
    if (status & SMBHSTSTS_DEV_ERR) {
        /*
         * This may be a PEC error, check and clear it.
         *
         * AUXSTS is handled differently from HSTSTS.
         * For HSTSTS, i801_isr() or i801_wait_intr()
         * has already cleared the error bits in hardware,
         * and we are passed a copy of the original value
         * in "status".
         * For AUXSTS, the hardware register is left
         * for us to handle here.
         * This is asymmetric, slightly iffy, but safe,
         * since all this code is serialized and the CRCE
         * bit is harmless as long as it's cleared before
         * the next operation.
         */
        if ((priv->features & FEATURE_SMBUS_PEC) &&
            (priv->inb_p(SMBAUXSTS(priv)) & SMBAUXSTS_CRCE)) {
            priv->outb_p(SMBAUXSTS_CRCE, SMBAUXSTS(priv));
            result = -EBADMSG;
            IOLogDebug("PEC error");
        } else {
            result = -ENXIO;
            IOLogDebug("No response");
        }
    }
    if (status & SMBHSTSTS_BUS_ERR) {
        result = -EAGAIN;
        IOLogDebug("Lost arbitration");
    }
    
    /* Clear status flags except BYTE_DONE, to be cleared by caller */
    priv->outb_p(status, SMBHSTSTS(priv));
    
    return result;
}

/* Wait for BUSY being cleared and either INTR or an error flag being set */
static int i801_wait_intr(struct i801_adapter *priv)
{
    int timeout = 0;
    int status;
    
    /* We will always wait for a fraction of a second! */
    do {
        IODelay(250);
        status = priv->inb_p(SMBHSTSTS(priv));
    } while (((status & SMBHSTSTS_HOST_BUSY) ||
              !(status & (STATUS_ERROR_FLAGS | SMBHSTSTS_INTR))) &&
             (timeout++ < MAX_RETRIES));
    
    if (timeout > MAX_RETRIES) {
        IOLogDebug("INTR Timeout!");
        return -ETIMEDOUT;
    }
    return status & (STATUS_ERROR_FLAGS | SMBHSTSTS_INTR);
}

static int i801_transaction(struct i801_adapter *priv, int xact)
{
    int status;
    int result;
    IOReturn sleep_result;
    AbsoluteTime timeout, curTime;
    
    result = i801_check_pre(priv);
    if (result < 0)
        return result;
    
    if (priv->features & FEATURE_IRQ) {
        priv->outb_p(xact | SMBHSTCNT_INTREN | SMBHSTCNT_START,
               SMBHSTCNT(priv));
        
        clock_get_uptime(&curTime);
        nanoseconds_to_absolutetime(priv->timeout, &timeout);
        sleep_result = priv->command_gate->commandSleep(&priv->status, curTime + timeout, THREAD_ABORTSAFE);
        
        if ( sleep_result == THREAD_TIMED_OUT ) {
            IOLogError("Timeout waiting for bus to accept transfer request");
            return -ETIMEDOUT;
        }
        status = priv->status;
        priv->status = 0;
        return i801_check_post(priv, status);
    }
    
    /* the current contents of SMBHSTCNT can be overwritten, since PEC,
     * SMBSCMD are passed in xact */
    priv->outb_p(xact | SMBHSTCNT_START, SMBHSTCNT(priv));
    
    status = i801_wait_intr(priv);
    return i801_check_post(priv, status);
}

/* Wait for either BYTE_DONE or an error flag being set */
static int i801_wait_byte_done(struct i801_adapter *priv)
{
    int timeout = 0;
    int status;
    
    /* We will always wait for a fraction of a second! */
    do {
        IODelay(250);
        status = priv->inb_p(SMBHSTSTS(priv));
    } while (!(status & (STATUS_ERROR_FLAGS | SMBHSTSTS_BYTE_DONE)) &&
             (timeout++ < MAX_RETRIES));
    
    if (timeout > MAX_RETRIES) {
        IOLogDebug("BYTE_DONE Timeout!");
        return -ETIMEDOUT;
    }
    return status & STATUS_ERROR_FLAGS;
}

/*
 * For "byte-by-byte" block transactions:
 *   I2C write uses cmd=I801_BLOCK_DATA, I2C_EN=1
 *   I2C read uses cmd=I801_I2C_BLOCK_DATA
 */
static int i801_block_transaction_byte_by_byte(struct i801_adapter *priv,
                                               union i2c_smbus_data *data,
                                               char read_write, int command,
                                               int hwpec)
{
    int i, len;
    int smbcmd;
    int status = 0;
    IOReturn result;
    AbsoluteTime curTime, timeout;

    result = i801_check_pre(priv);
    if (result < 0)
        return result;
    
    len = data->block[0];
    
    if (read_write == I2C_SMBUS_WRITE) {
        priv->outb_p(len, SMBHSTDAT0(priv));
        priv->outb_p(data->block[1], SMBBLKDAT(priv));
    }
    
    if (command == I2C_SMBUS_I2C_BLOCK_DATA &&
        read_write == I2C_SMBUS_READ)
        smbcmd = I801_I2C_BLOCK_DATA;
    else
        smbcmd = I801_BLOCK_DATA;
    
    if (priv->features & FEATURE_IRQ) {
        priv->is_read = (read_write == I2C_SMBUS_READ);
        if (len == 1 && priv->is_read)
            smbcmd |= SMBHSTCNT_LAST_BYTE;
        priv->cmd = smbcmd | SMBHSTCNT_INTREN;
        priv->len = len;
        priv->count = 0;
        priv->data = &data->block[1];
        
        priv->outb_p(priv->cmd | SMBHSTCNT_START, SMBHSTCNT(priv));
        
        clock_get_uptime(&curTime);
        nanoseconds_to_absolutetime(priv->timeout, &timeout);
        result = priv->command_gate->commandSleep(&priv->status, curTime + timeout, THREAD_ABORTSAFE);
        
        if ( result == THREAD_TIMED_OUT ) {
            IOLogError("Timeout waiting for bus to accept transfer request");
            return -ETIMEDOUT;
        }
        
        priv->status = 0;
        return i801_check_post(priv, status);
    }
    
    for (i = 1; i <= len; i++) {
        if (i == len && read_write == I2C_SMBUS_READ)
            smbcmd |= SMBHSTCNT_LAST_BYTE;
        priv->outb_p(smbcmd, SMBHSTCNT(priv));
        
        if (i == 1)
            priv->outb_p(priv->inb_p(SMBHSTCNT(priv)) | SMBHSTCNT_START,
                   SMBHSTCNT(priv));
        
        status = i801_wait_byte_done(priv);
        if (status)
            goto exit;
        
        if (i == 1 && read_write == I2C_SMBUS_READ
            && command != I2C_SMBUS_I2C_BLOCK_DATA) {
            len = priv->inb_p(SMBHSTDAT0(priv));
            if (len < 1 || len > I2C_SMBUS_BLOCK_MAX) {
                IOLogError("Illegal SMBus block read size %d",
                        len);
                /* Recover */
                while (priv->inb_p(SMBHSTSTS(priv)) &
                       SMBHSTSTS_HOST_BUSY)
                    priv->outb_p(SMBHSTSTS_BYTE_DONE,
                           SMBHSTSTS(priv));
                priv->outb_p(SMBHSTSTS_INTR, SMBHSTSTS(priv));
                return -EPROTO;
            }
            data->block[0] = len;
        }
        
        /* Retrieve/store value in SMBBLKDAT */
        if (read_write == I2C_SMBUS_READ)
            data->block[i] = priv->inb_p(SMBBLKDAT(priv));
        if (read_write == I2C_SMBUS_WRITE && i+1 <= len)
            priv->outb_p(data->block[i+1], SMBBLKDAT(priv));
        
        /* signals SMBBLKDAT ready */
        priv->outb_p(SMBHSTSTS_BYTE_DONE, SMBHSTSTS(priv));
    }
    
    status = i801_wait_intr(priv);
exit:
    return i801_check_post(priv, status);
}

static int i801_set_block_buffer_mode(struct i801_adapter *priv)
{
    priv->outb_p(priv->inb_p(SMBAUXCTL(priv)) | SMBAUXCTL_E32B, SMBAUXCTL(priv));
    if ((priv->inb_p(SMBAUXCTL(priv)) & SMBAUXCTL_E32B) == 0)
        return -EIO;
    return 0;
}

static int i801_block_transaction_by_block(struct i801_adapter *priv,
                                           union i2c_smbus_data *data,
                                           char read_write, int hwpec)
{
    int i, len;
    int status;
    
    priv->inb_p(SMBHSTCNT(priv)); /* reset the data buffer index */
    
    /* Use 32-byte buffer to process this transaction */
    if (read_write == I2C_SMBUS_WRITE) {
        len = data->block[0];
        priv->outb_p(len, SMBHSTDAT0(priv));
        for (i = 0; i < len; i++)
            priv->outb_p(data->block[i+1], SMBBLKDAT(priv));
    }
    
    status = i801_transaction(priv, I801_BLOCK_DATA |
                              (hwpec ? SMBHSTCNT_PEC_EN : 0));
    if (status)
        return status;
    
    if (read_write == I2C_SMBUS_READ) {
        len = priv->inb_p(SMBHSTDAT0(priv));
        if (len < 1 || len > I2C_SMBUS_BLOCK_MAX)
            return -EPROTO;
        
        data->block[0] = len;
        for (i = 0; i < len; i++)
            data->block[i + 1] = priv->inb_p(SMBBLKDAT(priv));
    }
    return 0;
}

static int i801_block_transaction(struct i801_adapter *priv,
                                  union i2c_smbus_data *data, char read_write,
                                  int command, int hwpec)
{
    int result = 0;
    unsigned char hostc = 0;
    
    if (command == I2C_SMBUS_I2C_BLOCK_DATA) {
        if (read_write == I2C_SMBUS_WRITE) {
            /* set I2C_EN bit in configuration register */
            hostc = priv->inb_p(SMBHSTCFG);
            priv->outb_p(hostc | SMBHSTCFG_I2C_EN, SMBHSTCFG);
            
        } else if (!(priv->features & FEATURE_I2C_BLOCK_READ)) {
            IOLogError("I2C block read is unsupported!");
            return -EOPNOTSUPP;
        }
    }
    
    if (read_write == I2C_SMBUS_WRITE
        || command == I2C_SMBUS_I2C_BLOCK_DATA) {
        if (data->block[0] < 1)
            data->block[0] = 1;
        if (data->block[0] > I2C_SMBUS_BLOCK_MAX)
            data->block[0] = I2C_SMBUS_BLOCK_MAX;
    } else {
        data->block[0] = 32;    /* max for SMBus block reads */
    }
    
    /* Experience has shown that the block buffer can only be used for
     SMBus (not I2C) block transactions, even though the datasheet
     doesn't mention this limitation. */
    if ((priv->features & FEATURE_BLOCK_BUFFER)
        && command != I2C_SMBUS_I2C_BLOCK_DATA
        && i801_set_block_buffer_mode(priv) == 0)
        result = i801_block_transaction_by_block(priv, data,
                                                 read_write, hwpec);
    else
        result = i801_block_transaction_byte_by_byte(priv, data,
                                                     read_write,
                                                     command, hwpec);
    
    if (command == I2C_SMBUS_I2C_BLOCK_DATA
        && read_write == I2C_SMBUS_WRITE) {
        /* restore saved configuration register value */
        priv->outb_p(hostc, SMBHSTCFG);
    }
    return result;
}



/* Return negative errno on error. */
static s32 i801_access(struct i801_adapter *priv, u16 addr,
                       unsigned short flags, char read_write, u8 command,
                       int size, union i2c_smbus_data *data)
{
    int hwpec;
    int block = 0;
    int ret = 0, xact = 0;
    
    hwpec = (priv->features & FEATURE_SMBUS_PEC) && (flags & I2C_CLIENT_PEC)
    && size != I2C_SMBUS_QUICK
    && size != I2C_SMBUS_I2C_BLOCK_DATA;
    
    switch (size) {
        case I2C_SMBUS_QUICK:
            priv->outb_p(((addr & 0x7f) << 1) | (read_write & 0x01),
                         SMBHSTADD(priv));
            xact = I801_QUICK;
            break;
        case I2C_SMBUS_BYTE:
            priv->outb_p(((addr & 0x7f) << 1) | (read_write & 0x01),
                         SMBHSTADD(priv));
            if (read_write == I2C_SMBUS_WRITE)
                priv->outb_p(command, SMBHSTCMD(priv));
            xact = I801_BYTE;
            break;
        case I2C_SMBUS_BYTE_DATA:
            priv->outb_p(((addr & 0x7f) << 1) | (read_write & 0x01),
                         SMBHSTADD(priv));
            priv->outb_p(command, SMBHSTCMD(priv));
            if (read_write == I2C_SMBUS_WRITE)
                priv->outb_p(data->byte, SMBHSTDAT0(priv));
            xact = I801_BYTE_DATA;
            break;
        case I2C_SMBUS_WORD_DATA:
            priv->outb_p(((addr & 0x7f) << 1) | (read_write & 0x01),
                         SMBHSTADD(priv));
            priv->outb_p(command, SMBHSTCMD(priv));
            if (read_write == I2C_SMBUS_WRITE) {
                priv->outb_p(data->word & 0xff, SMBHSTDAT0(priv));
                priv->outb_p((data->word & 0xff00) >> 8, SMBHSTDAT1(priv));
            }
            xact = I801_WORD_DATA;
            break;
        case I2C_SMBUS_BLOCK_DATA:
            priv->outb_p(((addr & 0x7f) << 1) | (read_write & 0x01),
                         SMBHSTADD(priv));
            priv->outb_p(command, SMBHSTCMD(priv));
            block = 1;
            break;
        case I2C_SMBUS_I2C_BLOCK_DATA:
            /*
             * NB: page 240 of ICH5 datasheet shows that the R/#W
             * bit should be cleared here, even when reading.
             * However if SPD Write Disable is set (Lynx Point and later),
             * the read will fail if we don't set the R/#W bit.
             */
            priv->outb_p(((addr & 0x7f) << 1) |
                         ((priv->original_hstcfg & SMBHSTCFG_SPD_WD) ?
                          (read_write & 0x01) : 0),
                         SMBHSTADD(priv));
            if (read_write == I2C_SMBUS_READ) {
                /* NB: page 240 of ICH5 datasheet also shows
                 * that DATA1 is the cmd field when reading */
                priv->outb_p(command, SMBHSTDAT1(priv));
            } else
                priv->outb_p(command, SMBHSTCMD(priv));
            block = 1;
            break;
        default:
            IOLogError("Unsupported transaction %d",
                  size);
            ret = -EOPNOTSUPP;
            goto out;
    }
    
    if (hwpec)    /* enable/disable hardware PEC */
        priv->outb_p(priv->inb_p(SMBAUXCTL(priv)) | SMBAUXCTL_CRC, SMBAUXCTL(priv));
    else
        priv->outb_p(priv->inb_p(SMBAUXCTL(priv)) & (~SMBAUXCTL_CRC),
                     SMBAUXCTL(priv));
    
    if (block)
        ret = i801_block_transaction(priv, data, read_write, size,
                                     hwpec);
    else
        ret = i801_transaction(priv, xact);
    
    /* Some BIOSes don't like it when PEC is enabled at reboot or resume
     time, so we forcibly disable it after every transaction. Turn off
     E32B for the same reason. */
    if (hwpec || block)
        priv->outb_p(priv->inb_p(SMBAUXCTL(priv)) &
                     ~(SMBAUXCTL_CRC | SMBAUXCTL_E32B), SMBAUXCTL(priv));
    
    if (block)
        goto out;
    if (ret)
        goto out;
    if ((read_write == I2C_SMBUS_WRITE) || (xact == I801_QUICK))
        goto out;
    
    switch (xact & 0x7f) {
        case I801_BYTE:    /* Result put in SMBHSTDAT0 */
        case I801_BYTE_DATA:
            data->byte = priv->inb_p(SMBHSTDAT0(priv));
            break;
        case I801_WORD_DATA:
            data->word = priv->inb_p(SMBHSTDAT0(priv)) +
            (priv->inb_p(SMBHSTDAT1(priv)) << 8);
            break;
    }
    
    out:
    return ret;
}

static void i801_isr_byte_done(struct i801_adapter *priv)
{
    if (priv->is_read) {
        /* For SMBus block reads, length is received with first byte */
        if (((priv->cmd & 0x1c) == I801_BLOCK_DATA) &&
            (priv->count == 0)) {
            priv->len = priv->inb_p(SMBHSTDAT0(priv));
            if (priv->len < 1 || priv->len > I2C_SMBUS_BLOCK_MAX) {
                IOLogError("Illegal SMBus block read size %d",
                        priv->len);
                /* FIXME: Recover */
                priv->len = I2C_SMBUS_BLOCK_MAX;
            } else {
                IOLogDebug("SMBus block read size is %d",
                        priv->len);
            }
            priv->data[-1] = priv->len;
        }
        
        /* Read next byte */
        if (priv->count < priv->len)
            priv->data[priv->count++] = priv->inb_p(SMBBLKDAT(priv));
        else
            IOLogDebug("Discarding extra byte on block read");
        
        /* Set LAST_BYTE for last byte of read transaction */
        if (priv->count == priv->len - 1)
            priv->outb_p(priv->cmd | SMBHSTCNT_LAST_BYTE,
                   SMBHSTCNT(priv));
    } else if (priv->count < priv->len - 1) {
        /* Write next byte, except for IRQ after last byte */
        priv->outb_p(priv->data[++priv->count], SMBBLKDAT(priv));
    }
    
    /* Clear BYTE_DONE to continue with next byte */
    priv->outb_p(SMBHSTSTS_BYTE_DONE, SMBHSTSTS(priv));
}


#endif /* i2c_i801_h */
