/* SPDX-License-Identifier: BSD-3-Clause
 * New API bridge to MIT emu76489. Not the former Maxim implementation. */
#ifndef COLECO_PUBLIC_PSG_H
#define COLECO_PUBLIC_PSG_H
#include "emu76489.h"
enum {MUTE_ALLON=15,BOOST_OFF=0,BOOST_ON=1,VOL_FULL=1,FB_SC3000=6,FB_SEGAVDP=9};
typedef struct {SNG chip;int mute,boost,feedback;} SN76489_Context;
void SN76489_Init(int,int,int);void SN76489_Reset(int);void SN76489_Shutdown(void);
void SN76489_Config(int,int,int,int,int);uint8_t* SN76489_GetContextPtr(int);int SN76489_GetContextSize(void);
void SN76489_Write(int,int);void SN76489_GGStereoWrite(int,int);void SN76489_Update(int,int16_t**,int);
#endif
