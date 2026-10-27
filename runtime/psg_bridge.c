/* SPDX-License-Identifier: BSD-3-Clause */

#include "shared.h"
static SN76489_Context context;
void SN76489_Init(int which,int clock,int rate){
 (void)which;memset(&context,0,sizeof context);SNG*t=SNG_new(clock,rate);if(!t)abort();
 context.chip=*t;SNG_delete(t);context.mute=15;context.feedback=6;SN76489_Reset(0);
}
void SN76489_Reset(int which){
 (void)which;SNG_reset(&context.chip);context.chip.noise_feedback=context.feedback;
 for(unsigned i=0;i<3;i++)context.chip.mute[i]=!(context.mute&(1<<i));
}
void SN76489_Shutdown(void){}
void SN76489_Config(int which,int mute,int boost,int volume,int feedback){
 (void)which;(void)volume;context.mute=mute;context.boost=boost;context.feedback=feedback;
 context.chip.noise_feedback=(uint32_t)feedback;
 for(unsigned i=0;i<3;i++)context.chip.mute[i]=!(mute&(1<<i));
}
uint8_t* SN76489_GetContextPtr(int which){(void)which;return (uint8_t*)&context;}
int SN76489_GetContextSize(void){return sizeof context;}
void SN76489_Write(int which,int data){(void)which;SNG_writeIO(&context.chip,(uint8_t)data);}
void SN76489_GGStereoWrite(int which,int data){(void)which;SNG_writeGGIO(&context.chip,(uint8_t)data);}
void SN76489_Update(int which,int16_t**buffer,int n){
 (void)which;for(int i=0;i<n;i++){int32_t v[2];SNG_calc_stereo(&context.chip,v);
  buffer[0][i]=(int16_t)(v[0]/4);buffer[1][i]=(int16_t)(v[1]/4);}
}
