/* SPDX-License-Identifier: BSD-3-Clause
 * Coleco machine adapter for floooh/chips (zlib license).
 * One host loop iteration is one Z80 T-state. Keep the original VDP/PSG.
 */
#include "z80.h"
#include <stdbool.h>
#include <string.h>
#if R8_COLECO_FAST
/* Same decoder and one-T-state bus order. Keep the large decoder and its
 * caller in SRAM without code growth from forced inlining. The SDK copies
 * these sections into SRAM before application entry. */
#define CHIPS_Z80_TICK_ATTR __attribute__((section(".time_critical.cv_z80_tick")))
#define CHIPS_Z80_FETCH_ATTR __attribute__((section(".time_critical.cv_z80_fetch")))
#define CHIPS_Z80_CONST_ATTR __attribute__((section(".time_critical.cv_z80_tables")))
#define CV_EXEC_ATTR __attribute__((section(".time_critical.cv_z80_execute")))
#else
#define CV_EXEC_ATTR
#endif
/* Rename only colliding API names, leaving the chips implementation intact. */
#define z80_init chips_z80_init
#define z80_reset chips_z80_reset
#define CHIPS_IMPL
#include "chips/z80.h"
#undef z80_init
#undef z80_reset

coleco_cpu_interface Z80;
int z80_cycle_count;
unsigned char *cpu_readmap[64], *cpu_writemap[64];
static z80_t cpu;
static uint64_t pins;
static bool nmi_line, nmi_pulse, irq_line;
static int (*acknowledge)(int);
#if R8_COLECO_HALT_BATCH
static uint64_t halt_skipped_ticks;
static uint32_t halt_spans;
void r8_coleco_halt_get(uint64_t *ticks,uint32_t *spans) {
    *ticks=halt_skipped_ticks;*spans=halt_spans;
}
/* At this exact M1/T2 boundary four unmodified ticks fetch HALT and return
 * every CPU/bus field to the same value except R[6:0]. Reads are direct
 * array accesses, refresh has no machine-side effect, and no device runs
 * inside execute(). Never cross this call's scanline/event budget. */
static unsigned halt_span(unsigned left) {
    uint64_t expected=Z80_HALT|Z80_M1|Z80_MREQ|Z80_RD|
        (uint16_t)(cpu.pc-1)|(UINT64_C(0x76)<<16);
    if(pins!=expected || cpu.pins!=pins || cpu.int_bits ||
       nmi_line || nmi_pulse || irq_line || cpu.step!=Z80_M1_T2 ||
       cpu.opcode!=0x76 || cpu.prefix_active || cpu.hlx_idx ||
       cpu.addr!=cpu.hl) return 0;
    uint16_t address=(uint16_t)(cpu.pc-1);
    if(cpu_readmap[address>>10][address&1023]!=0x76) return 0;
    return left&~3u;
}
#endif

void z80_init(int index,int clock,const void *config,int (*irq_callback)(int)) {
    (void)index; (void)clock; (void)config;
    memset(&Z80,0,sizeof Z80);
    acknowledge=irq_callback;
#if R8_COLECO_HALT_BATCH
    halt_skipped_ticks=0;halt_spans=0;
#endif
    z80_reset();
}
void z80_reset(void) {
    pins=chips_z80_reset(&cpu);
    nmi_line=nmi_pulse=irq_line=false;
    Z80.pc.w.l=cpu.pc;
}
void z80_exit(void) { }
void z80_reset_cycle_count(void) { z80_cycle_count=0; }
int z80_get_elapsed_cycles(void) { return z80_cycle_count; }
void z80_set_irq_line(int line,int state) {
    bool active=state!=CLEAR_LINE;
    if(line==INPUT_LINE_NMI) {
        /* Preserve an assert/clear pulse between execute calls. chips latches
         * the edge on the next tick and accepts it at an instruction boundary. */
        if(active && !nmi_line) nmi_pulse=true;
        nmi_line=active;
    } else irq_line=active;
}
CV_EXEC_ATTR int z80_execute(int cycles) {
    if(cycles<=0) return 0;
    for(int tick=0;tick<cycles;tick++) {
#if R8_COLECO_HALT_BATCH
        if(pins&Z80_HALT) {
            unsigned n=halt_span((unsigned)(cycles-tick));
            if(n) {
                cpu.r=(cpu.r&0x80)|((cpu.r+n/4)&0x7f);
                z80_cycle_count+=(int)n;
                halt_skipped_ticks+=n;halt_spans++;
                tick+=(int)n-1;
                continue;
            }
        }
#endif
        pins &= ~(Z80_NMI|Z80_INT);
        if(nmi_line||nmi_pulse) pins|=Z80_NMI;
        if(irq_line) pins|=Z80_INT;
        nmi_pulse=false;
        pins=z80_tick(&cpu,pins);
        ++z80_cycle_count;
        Z80.pc.w.l=cpu.pc;
        uint16_t address=Z80_GET_ADDR(pins);
        if(pins&Z80_MREQ) {
            if(pins&Z80_RD) {
                uint8_t data=cpu_readmap[address>>10][address&1023];
                Z80_SET_DATA(pins,data);
            } else if(pins&Z80_WR) Z80.mem_write(address,Z80_GET_DATA(pins));
        } else if(pins&Z80_IORQ) {
            if(pins&Z80_M1) {
                uint8_t vector=acknowledge?(uint8_t)acknowledge(0):0xFF;
                Z80_SET_DATA(pins,vector);
            } else if(pins&Z80_RD) {
                uint8_t data=Z80.port_read(address);
                Z80_SET_DATA(pins,data);
            } else if(pins&Z80_WR) Z80.port_write(address,Z80_GET_DATA(pins));
        }
    }
    return cycles;
}
