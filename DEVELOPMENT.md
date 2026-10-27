# Change record

## 2026-10-26

- Replaced the restricted CPU and unresolved PSG in the SG test selection
  with the chips Z80 and MIT emu76489. The original game-driven PSG writes
  remain intact; no replacement tones or music were added.
- Three SG games in three input modes passed 18 old/new PSG runs of 1,800
  frames. CPU, video and sample totals matched at 162 checkpoints. PCM is
  intentionally different after the PSG replacement.
- User accepted the scoped bench run of `brinquitos.sg`: final 60.077490
  seconds at 59.922610 FPS with no new dropped or zero-filled audio samples.
  Startup slow and cumulative underflow/overflow remain documented.

## 2026-10-27

- Prepared an independent SG-1000 source repository from the public Fruit Jam
  platform already audited for the related ColecoVision port. The shared
  machine/CPU/PSG code selection is the same as the accepted SG test build;
  `R8_COLECO=0` selects SG mode. The frontend target is now Fruit Jam.
- Retained original source and license notices, excluded ROMs, firmware
  artifacts and unreleased board details, and pinned SDK/PIO USB dependencies.
- A one-commit clean Git clone passed the source audit, Release firmware build
  and all nine SG ASan/UBSan game runs. Release footprint is 546,928 B Flash
  and 467,708 B static RAM. An earlier in-place build reused an incomplete
  CMake cache with different Release flags; its size is not release evidence.
- Fruit Jam physical testing is still required; the bench result does not
  transfer to the new board configuration.
