# Third-party provenance and distribution scope

The combined firmware is distributed under GPL-3.0-only. Individual files
retain their original notices; this does not relicense upstream components.

| Included component | Public baseline | Terms and changes |
|---|---|---|
| SMS Plus machine/video | [ducalex/retro-go](https://github.com/ducalex/retro-go/tree/4ced120669750ca7228fd0414211430c1d923166/retro-core/components/smsplus), `4ced120669750ca7228fd0414211430c1d923166` | Retained machine/video sources carry GPL-2.0-or-later notices, with `deps/smsplus/COPYING`. This is a comparison baseline, not a claim that the modified RP2350 snapshot was originally checked out at that revision. Full retained-source differences are in `provenance/smsplus-changes.diff`. |
| chips Z80 | [floooh/chips](https://github.com/floooh/chips), `ca7d7ddd3ba77b48685d24120cf413ea53786767` | zlib, `deps/chips/LICENSE`. Opt-in SRAM placement attributes; decoder unchanged. Independently written BSD-3-Clause machine callback adapter includes canonical HALT batching. |
| emu76489 | [digital-sound-antiques/emu76489](https://github.com/digital-sound-antiques/emu76489), `c0fa097060e022db237163d79704025435042997` | MIT, `deps/emu76489/LICENSE`. Context zero initialization and configurable noise feedback; BSD-3-Clause SG register bridge. |
| pico_shared | [fhoedemakers/pico_shared](https://github.com/fhoedemakers/pico_shared), `df683bf00ff95e1ceda88f132bbf72f8cc7cadb0` | GPL-3.0-only root license in `deps/pico_shared/LICENSE`, except files with their own terms. Selected shared code only. HDMI has local audio/queue, SRAM placement and double-buffer modifications; `provenance/pico_shared-changes.diff` records differences. |
| SD wrapper | pico_shared selected driver | BSD-2-Clause, `deps/pico_shared/drivers/pico_fatfs/LICENSE`. |
| FatFs | R0.16 retained in selected SD driver | ChaN permissive notice in `ff.c`, `ff.h` and related files. |
| PIO SPI | selected SD driver | Raspberry Pi BSD-3-Clause notices in `pio/spi/pio_spi.c` and `.h`. |
| TinyUSB configuration | selected shared code | Ha Thach MIT notice in `deps/pico_shared/tusb_config.h`. |
| New runtime/platform | this project | BSD-3-Clause file notices; combined distribution remains GPL-3.0-only. |

The old Buchmueller CPU implementation, CPU arithmetic tables and generator,
Maxim PSG implementation, FM implementations and embedded Coleco BIOS are
excluded. Replacements do not inherit their source or license. The historical
SMS Plus `CREDITS` is retained verbatim: its discussion of a later
noncommercial fork is not a grant for excluded code. Included machine sources
are evaluated by their own explicit GPL notices and the public baseline;
the project does not claim all versions of SMS Plus are GPL.

Rendering lookup tables are generated from the retained GPL renderer logic.
`deps/smsplus/generate_tables.py` generates rendering tables only. SG-1000
mode is selected at compile time with `R8_COLECO=0`; the shared source still
contains Coleco paths, but no Coleco BIOS is linked or required for SG games.

External build dependencies are pinned in `dependencies.lock.json`:
Pico SDK 2.3.0, its TinyUSB gitlink, and Pico-PIO-USB. Their own licenses
remain in their public checkouts. No BIOS, game ROM, bench hardware definition,
secret, firmware artifact or SDK checkout is part of this source distribution.

`provenance/upstream-comparison.json` records hashes and whether a file is
identical to its stated public comparison baseline. Local modifications are
not represented as pristine upstream code. The emu76489 data-byte path also
handles latched volume/noise writes without accessing beyond the tone array.
This source audit does not establish physical hardware operation.
