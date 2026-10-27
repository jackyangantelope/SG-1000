#!/usr/bin/env bash
# Supply your own three .sg cartridges in the argument directory.
set -euo pipefail
root=$(cd -- "$(dirname -- "$0")/.." && pwd)
content=$(cd -- "${1:?usage: test-host.sh CONTENT_DIRECTORY [BUILD_DIRECTORY]}" && pwd)
dest=${2:-"$root/build/host"}
mkdir -p "$dest"
cd "$root"
core=deps/smsplus
flags=(-std=c11 -O3 -g -funsigned-char -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
sources=(runtime/tms_runtime.c runtime/psg_bridge.c deps/emu76489/emu76489.c "$core"/{system,sms,memz80,pio,vdp,render,tms,loadrom}.c "$core/cpu/z80.c" "$core/sound/sound.c")
objects=()
for source in "${sources[@]}"; do
    obj="$dest/$(basename "$source").o"
    gcc "${flags[@]}" -DR8_COLECO=0 -DR8_COLECO_FAST=1 -DR8_COLECO_HALT_BATCH=1 -DRG_PLATFORM_RP2350=1 -DR8_PSRAM_HEAP=1 \
      -Dmalloc=r8_core_malloc -Dcalloc=r8_core_calloc -Dfree=r8_core_free \
      -Iruntime -I"$core" -Ideps/emu76489 -Ideps/chips -c "$source" -o "$obj"
    objects+=("$obj")
done
gcc "${flags[@]}" -Iruntime tests/game_smoke.c "${objects[@]}" -lm -o "$dest/game"
gcc "${flags[@]}" -Ideps/emu76489 tests/psg_smoke.c deps/emu76489/emu76489.c -o "$dest/psg"
export ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1
"$dest/psg"
for game in brinquitos space_attack viboritas; do
    for mode in 0 1 2; do
        "$dest/game" "$content/$game.sg" 1800 "$mode" > "$dest/$game-$mode.log"
        tail -n 1 "$dest/$game-$mode.log"
    done
    diff "$dest/$game-1.log" "$dest/$game-2.log"
done
