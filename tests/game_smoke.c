/* SPDX-License-Identifier: BSD-3-Clause */
#include "retro8.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint64_t samples,nonzero;
static uint32_t pcm=2166136261u;
static uint16_t pixels[320*240];
static void audio(int16_t l,int16_t r){samples++;nonzero+=l!=0||r!=0;pcm=(pcm^(uint16_t)l)*16777619u;}
static uint8_t* read_file(const char*p,size_t*n){FILE*f=fopen(p,"rb");assert(f);fseek(f,0,SEEK_END);*n=ftell(f);rewind(f);uint8_t*b=malloc(*n);assert(b);assert(fread(b,1,*n,f)==*n);fclose(f);return b;}
static uint32_t hash(void*p,size_t n){uint8_t*b=p;uint32_t h=2166136261u;while(n--)h=(h^*b++)*16777619u;return h;}
int main(int argc,char**argv){
 assert(argc==4);size_t gn;uint8_t*g=read_file(argv[1],&gn);
 unsigned frames=atoi(argv[2]),mode=atoi(argv[3]);void*bulk=calloc(1,1048576);assert(bulk);
 assert(r8_init(bulk,1048576,audio,NULL)&&r8_load(g,gn,NULL,0,"sg"));
 for(unsigned f=0;f<frames;f++){
  r8_input in={0};if(mode){if(f%120>=60&&f%120<70)in.pads[0]|=R8_START;
   if(f%90>=30&&f%90<40)in.pads[0]|=R8_A;if(f>360)in.pads[0]|=(f/90)%2?R8_LEFT:R8_RIGHT;}
  if(mode==2){unsigned pad=in.pads[0],k=0;if(pad&R8_START)in.keys[k++]=0x28;
   if(pad&R8_A)in.keys[k++]=0x2c;if(pad&R8_LEFT)in.keys[k++]=0x50;if(pad&R8_RIGHT)in.keys[k++]=0x4f;in.pads[0]=0;}
  r8_controls(&in);r8_frame();if(f%100==99){r8_render(pixels);
   printf("FRAME %u pc=%04x pixels=%08x samples=%llu nonzero=%llu pcm=%08x\n",f+1,r8_pc(),hash(pixels,sizeof pixels),(unsigned long long)samples,(unsigned long long)nonzero,pcm);}
 }
 assert(samples==(uint64_t)frames*r8_frame_cycles()*44100/r8_clock());assert(nonzero>100);
 for(unsigned i=0;i<3;i++){assert(r8_load(g,gn,NULL,0,"sg"));r8_frame();}
 assert(!r8_load(g,1,NULL,0,"sg"));puts("PASS original game execution, sample count, reload and invalid-length gate");
 free(bulk);free(g);return 0;
}
