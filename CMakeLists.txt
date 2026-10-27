cmake_minimum_required(VERSION 3.20)
set(PICO_BOARD adafruit_fruit_jam CACHE STRING "Public target" FORCE)
set(PICO_PSRAM_CS_PIN 47 CACHE STRING "Fruit Jam PSRAM CS" FORCE)
set(PICO_PSRAM_SIZE_BYTES 0x800000 CACHE STRING "PSRAM size" FORCE)
if(NOT DEFINED PICO_SDK_PATH OR NOT DEFINED PICO_PIO_USB_PATH)
  message(FATAL_ERROR "Set PICO_SDK_PATH and PICO_PIO_USB_PATH to SDK/Pico-PIO-USB checkouts")
endif()
set(ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(CORE "${ROOT}/deps/smsplus")
set(SHARED "${ROOT}/deps/pico_shared")
set(HDMI "${SHARED}/drivers/pico_hdmi")
set(FATFS "${SHARED}/drivers/pico_fatfs")
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")
project(SG1000 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
set(SAVED_TINYUSB_TOP "${TOP}")
set(TOP "${PICO_SDK_PATH}/lib/tinyusb")
check_and_add_pico_pio_usb_support()
set(TOP "${SAVED_TINYUSB_TOP}")
add_subdirectory("${FATFS}" "${CMAKE_BINARY_DIR}/pico_fatfs")
option(SG_QUEUE_DIAGNOSTICS "Passive PCM queue and SG HALT accounting" OFF)
add_library(sg1000_core STATIC
  runtime/tms_runtime.c runtime/psg_bridge.c deps/emu76489/emu76489.c
  "${CORE}/system.c" "${CORE}/sms.c" "${CORE}/memz80.c" "${CORE}/pio.c"
  "${CORE}/vdp.c" "${CORE}/render.c" "${CORE}/tms.c" "${CORE}/loadrom.c"
  "${CORE}/cpu/z80.c" "${CORE}/sound/sound.c")
target_include_directories(sg1000_core PUBLIC runtime PRIVATE "${CORE}" deps/chips deps/emu76489)
target_compile_definitions(sg1000_core PRIVATE RG_PLATFORM_RP2350=1 R8_COLECO=0
  R8_PSRAM_HEAP=1 R8_COLECO_FAST=1 R8_COLECO_HALT_BATCH=1
  malloc=r8_core_malloc calloc=r8_core_calloc free=r8_core_free)
target_compile_options(sg1000_core PRIVATE -O3 -funsigned-char -ffunction-sections -fdata-sections)
add_library(sg1000_hdmi STATIC "${HDMI}/hstx.c" "${HDMI}/video_output.c"
  "${HDMI}/hstx_data_island_queue.c" "${HDMI}/hstx_packet.c")
target_include_directories(sg1000_hdmi PUBLIC "${HDMI}")
target_compile_definitions(sg1000_hdmi PRIVATE GPIOHSTXD0=15 GPIOHSTXD1=17 GPIOHSTXD2=19
  GPIOHSTXCK=13 GPIOHSTXINVERTED=1 JACK_HSTX_126MHZ=1 JACK_HSTX_DOUBLE_BUFFER=1)
target_compile_options(sg1000_hdmi PRIVATE -O3)
target_link_libraries(sg1000_hdmi PUBLIC pico_stdlib pico_multicore hardware_dma hardware_sync
  hardware_clocks hardware_vreg hardware_pio hardware_spi hardware_i2c hardware_pwm hardware_adc)
add_executable(picoSg1000 firmware/sg1000.c platform/input/jack_input.c
  platform/input/jack_input_devices.c platform/input/jack_tinyusb_input.c platform/input/jack_nespad_input.c)
target_include_directories(picoSg1000 PRIVATE runtime platform/input platform/text "${SHARED}" "${FATFS}/fatfs")
target_compile_definitions(picoSg1000 PRIVATE HW_CONFIG=8 PICO_STACK_SIZE=4096
  R8_TITLE="SEGA SG-1000" R8_DIRECTORY="/roms/SG1000" R8_EXTENSION=".sg" R8_BIOS=""
  R8_PROFILE=1 R8_ASYNC_PAD=1 R8_DEFERRED_PRESENT=1 R8_COLECO_HALT_BATCH=1
  R8_SKIP_48K_AY=0 R8_SRAM_HOT=0 R8_QUEUE_DIAG=$<BOOL:${SG_QUEUE_DIAGNOSTICS}>
  R8_SG_QUEUE_DIAG=$<BOOL:${SG_QUEUE_DIAGNOSTICS}>
  R8_VARIANT="SG_PUBLIC_MIT_PSG" R8_BUILD_VERSION="0.2.0-public-candidate"
  JACK_TINYUSB_KEYBOARD_RECOVERY=1 PICO_PIO_USE_GPIO_BASE=1 CFG_TUH_RPI_PIO_USB=1
  PIO_USB_USE_PIO=2 PICO_DEFAULT_PIO_USB_DP_PIN=1
  SDCARD_PIN_CS=39 SDCARD_PIN_SCK=34 SDCARD_PIN_MOSI=35 SDCARD_PIN_MISO=36 SDCARD_SPI=spi0 SDCARD_PIO=pio0)
target_compile_options(picoSg1000 PRIVATE -O3 -Wall -Wextra -Werror
  -Wno-unused-parameter -Wno-cast-function-type -Wno-missing-field-initializers -Wno-attributes -Wno-sign-compare)
target_link_libraries(picoSg1000 PRIVATE sg1000_core sg1000_hdmi pico_fatfs pico_stdlib pico_multicore
  hardware_dma hardware_pio hardware_spi hardware_clocks hardware_vreg hardware_psram hardware_xip_cache
  tinyusb_host tinyusb_board tinyusb_pico_pio_usb m)
target_link_options(picoSg1000 PRIVATE -Wl,--gc-sections -Wl,--print-memory-usage)
pico_set_program_name(picoSg1000 "picoSg1000")
pico_set_program_version(picoSg1000 "0.2.0-public-candidate")
pico_enable_stdio_uart(picoSg1000 1)
pico_enable_stdio_usb(picoSg1000 0)
include("${ROOT}/cmake/LoaderPartition.cmake")
sg1000_link_for_loader(picoSg1000)
pico_add_extra_outputs(picoSg1000)
