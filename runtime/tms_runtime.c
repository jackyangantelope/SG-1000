/* SPDX-License-Identifier: BSD-3-Clause */

#include "retro8.h"
#include <stddef.h>
#include "shared.h"
extern uint16 retro8_tms_color(unsigned index);
static uint8_t *rom_store;
static uint8_t *bios_store;
static uint8_t display[256*192];
static r8_audio_fn audio_sink;
static r8_service_fn service_sink;
static bool loaded;
static const char *error_text="";
#if R8_PSRAM_HEAP

/* Only the emulation core accesses this PSRAM arena. */
typedef union r8_block r8_block;
union r8_block {
    struct { size_t size; r8_block *next; bool available; } h;
    max_align_t alignment;
};
static r8_block *heap;
void *r8_core_malloc(size_t size) {
    const size_t align=_Alignof(max_align_t);
    if(!size || size>SIZE_MAX-align) return NULL;
    size=(size+align-1)&~(align-1);
    for(r8_block *b=heap;b;b=b->h.next) if(b->h.available && b->h.size>=size) {
        if(b->h.size-size>=sizeof(r8_block)+align) {
            r8_block *tail=(r8_block *)((uint8_t *)(b+1)+size);
            tail->h.size=b->h.size-size-sizeof(*tail);
            tail->h.next=b->h.next;tail->h.available=true;
            b->h.next=tail;b->h.size=size;
        }
        b->h.available=false;return b+1;
    }
    return NULL;
}
void *r8_core_calloc(size_t count,size_t size) {
    if(size && count>SIZE_MAX/size)return NULL;
    void *p=r8_core_malloc(count*size);
    if(p)memset(p,0,count*size);
    return p;
}
void r8_core_free(void *p) {
    if(!p)return;
    ((r8_block *)p-1)->h.available=true;
    for(r8_block *b=heap;b && b->h.next;) {
        r8_block *n=b->h.next;
        if(b->h.available && n->h.available) {
            b->h.size+=sizeof(*n)+n->h.size;b->h.next=n->h.next;
        } else b=n;
    }
}
#endif
void retro8_service(void) { if(service_sink) service_sink(); }
static bool key(const r8_input *state, unsigned code) {
    for(unsigned i=0;i<6;i++) if(state->keys[i]==code) return true;
    return false;
}
bool r8_init(void *bulk,size_t capacity,r8_audio_fn audio,r8_service_fn service) {
    if(!bulk || capacity<73728 || loaded) return false;
    rom_store=bulk; bios_store=rom_store+65536; audio_sink=audio; service_sink=service;
#if R8_PSRAM_HEAP
    if(capacity<73728+sizeof(r8_block)+65536)return false;
    heap=(r8_block *)(rom_store+73728);
    heap->h.size=capacity-73728-sizeof(*heap);
    heap->h.next=NULL;heap->h.available=true;
#endif
    return true;
}
bool r8_load(const uint8_t *game,size_t size,const uint8_t *boot,size_t boot_size,const char *extension) {
    (void)extension;
    if(!rom_store || !game || size<256 || size>32768) { error_text="Use a 256 B..32 KiB standard cartridge";return false; }
#if R8_COLECO
    if(!boot || boot_size!=8192) { error_text="Missing 8192-byte coleco.rom";return false; }
    if(!((game[0]==0xAA&&game[1]==0x55)||(game[0]==0x55&&game[1]==0xAA))) {
        error_text="Invalid Coleco cartridge header";return false;
    }
#else
    (void)boot; (void)boot_size;
#endif
    if(loaded) {
        system_shutdown(); free(cart.sram); cart.sram=NULL; memset(&snd,0,sizeof snd); loaded=false;
    }
    memset(&smsplus,0,sizeof smsplus); memset(&sms,0,sizeof sms);
    memset(&vdp,0,sizeof vdp); memset(&coleco,0,sizeof coleco);
    memset(rom_store,0xFF,65536); memcpy(rom_store,game,size);
    system_reset_config();option.console=R8_COLECO?6:5;
    option.country=1;option.sndrate=44100;option.overscan=0;
    bitmap.data=display;bitmap.width=256;bitmap.height=192;bitmap.pitch=256;bitmap.granularity=1;
    if(!load_rom(rom_store,32768,(int)size)) { error_text="Cartridge configuration failed";return false; }
#if R8_COLECO
    memcpy(bios_store,boot,8192);coleco.rom=bios_store;
#endif
    system_poweron();loaded=true;error_text="";
    return true;
}
void r8_controls(const r8_input *state) {
    for(unsigned player=0;player<2;player++) {
        unsigned pad=state->pads[player];
        if(player==0) {
            if(key(state,0x52))pad|=R8_UP;if(key(state,0x51))pad|=R8_DOWN;
            if(key(state,0x50))pad|=R8_LEFT;if(key(state,0x4F))pad|=R8_RIGHT;
            if(key(state,0x2C)||key(state,0x1D))pad|=R8_A;
            if(key(state,0x1B))pad|=R8_B;
        }
        input.pad[player]=((pad&R8_UP)?INPUT_UP:0)|((pad&R8_DOWN)?INPUT_DOWN:0)|
            ((pad&R8_LEFT)?INPUT_LEFT:0)|((pad&R8_RIGHT)?INPUT_RIGHT:0)|
            ((pad&R8_A)?INPUT_BUTTON1:0)|((pad&R8_B)?INPUT_BUTTON2:0);
        coleco.keypad[player]=0xFF;
        if(pad&R8_START)coleco.keypad[player]=1;
        if(pad&R8_X)coleco.keypad[player]=2;
        if(pad&R8_Y)coleco.keypad[player]=10;
        if(pad&R8_SELECT)coleco.keypad[player]=11;
    }
    for(unsigned digit=1;digit<=9;digit++)if(key(state,0x1D+digit))coleco.keypad[0]=(uint8_t)digit;
    if(key(state,0x27))coleco.keypad[0]=0;
    if(key(state,0x28))coleco.keypad[0]=1;
    if(key(state,0x55))coleco.keypad[0]=10;
    if(key(state,0x56))coleco.keypad[0]=11;
    input.system=0;
#if !R8_COLECO
    if((state->pads[0]&R8_START)||key(state,0x28))input.pad[0]|=INPUT_BUTTON1;
    if(key(state,0x13))input.system=INPUT_PAUSE;
#endif
}
void r8_frame(void) {
    if(!loaded)return;
    system_frame(0);
    if(audio_sink)for(int i=0;i<snd.sample_count;i++) {

        int l=snd.stream[STREAM_PSG_L][i]*4,r=snd.stream[STREAM_PSG_R][i]*4;
        if(l>32767)l=32767;if(l<-32768)l=-32768;
        if(r>32767)r=32767;if(r<-32768)r=-32768;
        audio_sink((int16_t)l,(int16_t)r);
    }
}
void r8_render(uint16_t *dest) {
    uint16_t colors[16];for(unsigned i=0;i<16;i++)colors[i]=retro8_tms_color(i);
    for(unsigned y=0;y<240;y++)for(unsigned x=0;x<320;x++)
        dest[y*320+x]=colors[display[(y*4/5)*256+x*4/5]&15];
}
const char *r8_error(void){return error_text;}
uint32_t r8_clock(void){return CLOCK_NTSC;}
uint32_t r8_frame_cycles(void){return 228*262;}
uint16_t r8_pc(void){return Z80.pc.w.l;}
uint8_t r8_peek(uint16_t address){return Z80.mem_read?Z80.mem_read(address):0;}

void system_manage_sram(uint8 *sram,int slot,int mode){(void)sram;(void)slot;(void)mode;}
