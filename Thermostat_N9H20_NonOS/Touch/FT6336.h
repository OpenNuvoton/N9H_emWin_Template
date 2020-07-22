/***************************************************************************
 *                                                                                                               *
 * Copyright (c) Nuvoton Technolog. All rights reserved.                                    *
 *                                                                                                               *
 ***************************************************************************/

#ifndef __FT6336_H
#define __FT6336_H

#include "N9H20.h"

#define I2C_ADDR_FT6336             0x48

#define FT6336_ADDR_DEVICE_MODE     0x00
#define FT6336_ADDR_TD_STATUS       0x02

#define FT6336_ADDR_TOUCH1_EVENT    0x03
#define FT6336_ADDR_TOUCH1_ID       0x05
#define FT6336_ADDR_TOUCH1_X        0x03
#define FT6336_ADDR_TOUCH1_Y        0x05

#define FT6336_ADDR_TOUCH2_EVENT    0x09
#define FT6336_ADDR_TOUCH2_ID       0x0B
#define FT6336_ADDR_TOUCH2_X        0x09
#define FT6336_ADDR_TOUCH2_Y        0x0B

#define FT6336_ADDR_FIRMARE_ID      0xA6

UINT8 FT6336_read_firmware_id(void);
UINT8 FT6336_read_device_mode(void);
UINT8 FT6336_read_td_status(void);
UINT16 FT6336_read_touch1_x(void);
UINT16 FT6336_read_touch1_y(void);
UINT8 FT6336_read_touch1_event(void);
UINT8 FT6336_read_touch1_id(void);

//UINT8 FT6336_read_touch2_event(void);
//UINT8 FT6336_read_touch2_id(void);
//UINT16 FT6336_read_touch2_x(void);
//UINT16 FT6336_read_touch2_y(void);

UINT8 FT6336_write_device_mode(void);


#endif
