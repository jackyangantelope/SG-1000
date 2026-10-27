/* SPDX-License-Identifier: BSD-3-Clause
 * New, minimal machine-side interface for the chips Z80 adapter.
 * No legacy CPU context or instruction implementation is used here.
 */
#ifndef COLECO_CHIPS_ADAPTER_H
#define COLECO_CHIPS_ADAPTER_H
#include <stdint.h>
/* The PSG historically obtained these fixed-width names via the CPU header. */
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
enum { CLEAR_LINE=0, ASSERT_LINE=1, INPUT_LINE_IRQ0=0, INPUT_LINE_NMI=32 };
typedef struct {
    struct { struct { uint16_t l; } w; } pc;
    uint8_t (*mem_read)(uint16_t);
    void (*mem_write)(uint16_t, uint8_t);
    uint8_t (*port_read)(uint16_t);
    void (*port_write)(uint16_t, uint8_t);
} coleco_cpu_interface;
extern coleco_cpu_interface Z80;
extern int z80_cycle_count;
extern unsigned char *cpu_readmap[64], *cpu_writemap[64];
void z80_init(int index,int clock,const void *config,int (*irq_callback)(int));
void z80_reset(void);
void z80_exit(void);
int z80_execute(int cycles);
void z80_set_irq_line(int line,int state);
void z80_reset_cycle_count(void);
int z80_get_elapsed_cycles(void);
#endif
