#ifndef __N9H26TOUCHPANEL_H__
#define __N9H26TOUCHPANEL_H__

#include "LCDConf.h"

int Init_TouchPanel(void);
int Read_TouchPanel(int *x, int *y);
int Uninit_TouchPanel(void);
#endif
