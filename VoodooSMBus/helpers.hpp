
#ifndef smbus_helpers_hpp
#define smbus_helpers_hpp

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>

typedef UInt8 __u8;
typedef __u8 u8;
typedef UInt16 __u16;
typedef __u16 u16;
typedef SInt32 s32;

#define IOLogError(arg...) IOLog("Error: " arg)
#define IOLogDebug(arg...) IOLog("Debug: " arg)

/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
// from https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/errno-base.h

#define EIO              5      /* I/O error */
#define ENXIO            6      /* No such device or address */
#define EAGAIN          11      /* Try again */
#define EBUSY           16      /* Device or resource busy */
#define EPROTO          71      /* Protocol error */
#define EBADMSG         74      /* Not a data message */
#define EOPNOTSUPP      95      /* Operation not supported on transport endpoint */
#define ETIMEDOUT       110     /* Connection timed out */

#endif /* smbus_helpers_hpp */
