/* SPDX-License-Identifier: GPL-2.0+ WITH Linux-syscall-note */
/* ------------------------------------------------------------------------- */
/*                                         */
/* i2c.h - definitions for the i2c-bus interface                 */
/*                                         */
/* ------------------------------------------------------------------------- */
/*   Copyright (C) 1995-2000 Simon G. Vogl
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA 02110-1301 USA.                                 */
/* ------------------------------------------------------------------------- */

/* With some changes from Kyösti Mälkki <kmalkki@cc.hut.fi> and
 Frodo Looijaard <frodol@dds.nl> */

#ifndef i2c_smbus_h
#define i2c_smbus_h

#include "helpers.hpp"

/*
 * Data for SMBus Messages
 */
#define I2C_SMBUS_BLOCK_MAX         32    /* As specified in SMBus standard */
union i2c_smbus_data {
    __u8 byte;
    __u16 word;
    __u8 block[I2C_SMBUS_BLOCK_MAX + 2]; /* block[0] is used for length */
    /* and one more for user-space compatibility */
};

/* i2c_smbus_xfer read or write markers */
#define I2C_SMBUS_READ              1
#define I2C_SMBUS_WRITE             0

/* SMBus transaction types (size parameter in the above functions)
 Note: these no longer correspond to the (arbitrary) PIIX4 internal codes! */
#define I2C_SMBUS_QUICK             0
#define I2C_SMBUS_BYTE              1
#define I2C_SMBUS_BYTE_DATA         2
#define I2C_SMBUS_WORD_DATA         3
#define I2C_SMBUS_PROC_CALL         4
#define I2C_SMBUS_BLOCK_DATA        5
#define I2C_SMBUS_I2C_BLOCK_BROKEN  6
#define I2C_SMBUS_BLOCK_PROC_CALL   7        /* SMBus 2.0 */
#define I2C_SMBUS_I2C_BLOCK_DATA    8


/*flags for the client struct: */
#define I2C_CLIENT_PEC              0x04    /* Use Packet Error Checking */
#define I2C_CLIENT_TEN              0x10    /* we have a ten bit chip address */
/* Must equal I2C_M_TEN below */
#define I2C_CLIENT_SLAVE            0x20    /* we are the slave */
#define I2C_CLIENT_HOST_NOTIFY      0x40    /* We want to use I2C host notify */
#define I2C_CLIENT_WAKE             0x80    /* for board_info; true iff can wake */
#define I2C_CLIENT_SCCB             0x9000    /* Use Omnivision SCCB protocol */
/* Must match I2C_M_STOP|IGNORE_NAK */

#define I2C_M_TEN                   0x0010    /* this is a ten bit chip address */

#endif /* i2c_smbus_h */
