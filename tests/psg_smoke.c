/* SPDX-License-Identifier: BSD-3-Clause */
#include <assert.h>
#include <stdlib.h>
#include "emu76489.h"

int main(void) {
    SNG *s = SNG_new(3579545, 44100);
    assert(s);
    SNG_reset(s);
    for (unsigned i = 0; i < 4096; ++i) assert(SNG_calc(s) == 0);
    s->noise_feedback = 6;
    SNG_writeIO(s, 0xe4);
    assert(s->noise_feedback == 6);
    SNG_writeIO(s, 0xf0);
    unsigned active = 0;
    for (unsigned i = 0; i < 44100; ++i) active += SNG_calc(s) != 0;
    assert(active > 100);
    SNG_reset(s);
    SNG_writeIO(s, 0x84);
    SNG_writeIO(s, 0x10);
    SNG_writeIO(s, 0x90);
    unsigned transitions = 0;
    int previous = SNG_calc(s) != 0;
    for (unsigned i = 1; i < 44100; ++i) {
        int current = SNG_calc(s) != 0;
        transitions += current != previous;
        previous = current;
    }
    /* Tone period 0x104: clock/(32*260), two edges per period. */
    assert(transitions > 840 && transitions < 880);
    SNG_reset(s);
    for (unsigned i = 0; i < 4096; ++i) assert(SNG_calc(s) == 0);
    for (unsigned reg = 0; reg < 8; ++reg) {
        SNG_writeIO(s, 0x80 | (reg << 4));
        SNG_writeIO(s, 0x05);
        assert(s->adr == reg);
        if (reg == 1 || reg == 3 || reg == 5)
            assert(s->volume[(reg - 1) / 2] == 5);
        if (reg == 6) assert(s->noise_mode == 1 && s->noise_fref == 0);
        if (reg == 7) assert(s->noise_volume == 5);
    }
    SNG_delete(s);
    return 0;
}
