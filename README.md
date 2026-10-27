# Sega SG-1000 for RP2350

Independent public-source release targeting Adafruit Fruit Jam.

## Current evidence

On 2026-10-26, the user accepted an RP2350B bench build using this SG-1000
CPU/PSG selection with `brinquitos.sg`: the final 60.077490 seconds ran at
59.922610 emulated FPS without new dropped or zero-filled samples. Startup
was slower, and cumulative underflow/overflow and three replaced frames remain
recorded. This is scoped acceptance, not a claim for every game or control.
The public Fruit Jam target changes the board frontend and has not been
tested on hardware. It builds from pinned dependencies; three external SG
games each passed 1,800 emulated frames in three input modes under
AddressSanitizer and UndefinedBehaviorSanitizer. This is not physical board
acceptance or proof of real-time speed; see [VALIDATION.md](VALIDATION.md).

## Source and licensing

The old noncommercial Z80 implementation and the old Maxim SN76489 source
are excluded. CPU execution uses [floooh/chips](https://github.com/floooh/chips)
at `ca7d7ddd3ba77b48685d24120cf413ea53786767`; machine/video code derives
from [SMS Plus in retro-go](https://github.com/ducalex/retro-go/tree/4ced120669750ca7228fd0414211430c1d923166/retro-core/components/smsplus);
sound uses [emu76489](https://github.com/digital-sound-antiques/emu76489)
at `c0fa097060e022db237163d79704025435042997` with an SG-1000 register bridge. Original
third-party license files and source notices are retained. The combined
distribution uses GPL-3.0-only, preserving each component's original license;
see [THIRD_PARTY.md](THIRD_PARTY.md) and the recorded upstream differences.

No BIOS is needed for standard SG-1000 cartridges. No game ROM or bench
hardware configuration is included. Supply legally obtained `.sg` files.

## Build status

CMake requires explicit `PICO_SDK_PATH` and `PICO_PIO_USB_PATH` dependency
checkouts. All emulator/runtime/shared source paths resolve inside this tree.
Versions are pinned in `dependencies.lock.json`. Fetch dependencies with
`scripts/fetch-dependencies.ps1 -Destination <outside-source-directory>`.
Install CMake 3.20+, Ninja, Python 3, Git and Arm GNU Toolchain 14.2.Rel1.
Then run:

```powershell
./scripts/build.ps1 -PicoSdkPath <pico-sdk> -PicoPioUsbPath <Pico-PIO-USB> `
  -ToolchainPath <arm-toolchain> -NinjaPath <ninja.exe>
```

The SDK can build/fetch its matching pioasm and picotool host tools. For an
offline build, supply `-PioasmDir`, `-PicotoolDir` and optionally `-PythonExe`.
SDK 2.3.0 and its pinned TinyUSB completed the public build. Build and
host-test results are recorded separately from hardware tests.

The firmware links at `0x10080000`, for a compatible application loader.
It is not a standalone boot-at-zero image. Fruit Jam uses SD SPI pins
34/35/36/39, USB D+/D- 1/2 with power GPIO11, and SDK-default UART1 8/9.
Physical operation of that public target is not yet established.

Linux host checks: `bash scripts/test-host.sh <external-content-directory>`.
Supply `brinquitos.sg`, `space_attack.sg` and `viboritas.sg`.
Run `python scripts/audit-source.py` to audit source selection and notices.

This repository publishes source only. The earlier scoped SG test does not
establish Fruit Jam gameplay, audio, controls or sustained performance.
