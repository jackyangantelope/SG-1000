# Validation, 2026-10-27

| Evidence | Result | Boundary |
|---|---|---|
| Source selection and notices | Passed | 108 intended tracked files; restricted CPU/PSG, ROM and private board paths excluded |
| Public dependency pins | Verified and built | Official SDK 2.3.0 with TinyUSB gitlink; PIO USB pin in lock file |
| Public host game runs | Passed in source tree and clean clone | Three games x three input modes x 1,800 frames under ASan/UBSan; sample count, reload and invalid-length gate passed |
| Earlier SG host regression | Passed | Three games x three input modes x 1,800 frames for old/new PSG: 18 runs, 162 CPU/video/sample checkpoints equal; PCM differs intentionally |
| Earlier SG PSG-specific tests | Passed | Silence/reset, tone transition, noise activity/feedback and latched-data writes |
| RP2350B bench SG firmware | Scoped physical pass, 2026-10-26 | User accepted `brinquitos.sg` experience. 8,510 frames / 144.218165 s = 59.007823 FPS overall; final 3,600 / 60.077490 s = 59.922610 FPS with no new drops or zero-fill. Startup slow; cumulative under27/over26/slow54 and replaced3. Other games, complete controls and long stability unverified. |
| Public Fruit Jam firmware | Clean-clone Release build passed, physical acceptance pending | Flash 546,928 B, static RAM 467,708 B; different board frontend from scoped bench result |

No SD card, board flash or serial port was changed during publication prep.
The public Fruit Jam target still needs physical validation. The complete
bench capture is retained privately because it names the unreleased board;
its SHA-256 is `8e1313844cba6d1b0558decdb34715e874e7ff5bc02ad57218d743f49edd135a`.
