## Building/Flashing
  * GFollow instructions for [ch32v003fun](https://github.com/cnlohr/ch32v003fun)
  * Make sure switch is on 3.3V. Connect 3.3V, SWIO, and GND from WCH-LinkE to the board temporarily using a 3X1 header held in place using pressure from fingers.
  * Unbrick CPU if needed (if already flashed before) by running:
```
> ch32v003/minichlink/minichlink -u
```
  * Flash bootloader if needed (not likely):
```
> ch32v003/minichlink/minichlink -w factory_bootloader.bin bootloader
```
  * Build and flash image:
```
> make
```

## Requirements
  * Have 30-50 boards designed, produced, and flashed in time for team banquet. (6 weeks)
    * 3 weeks for board design "tape in"
    * 2 weeks for manufacturing. Firmware is finalized at the same time
    * 1 week for assembly
  * Under $8 each
  * Inspire students to create their own PCB designs next school year with mentor support
  * If there is power needed, the battery needs to last at least one competition or be easily rechargeable with USB
  * It needs to look good/presentable -- have some kind of art on it.

### Nice-To-Have
  * Be understandable / modifiable by interested students. *While I would argue firmware modification is still possible, Brent disagrees*
  * Have multiple patterns
  * Gray-scale, ideally more than 256 levels. *This complicates charlieplexing a bit. Not so much 3 LEDs though*


## Patterns
  * Breathing like Macbook
  * Pulsing with beats like pendulum.
  * Others? If students want to program one here: https://wokwi.com/projects/399273653422371841, I can translate it to the real code pretty easily.


## Post-Mortem


### Tools
  * Brent suggested using a different tool. At minimum one that has DRC for silkscreen covering pad :-) Does he have a recommendation? KiCad?
  * Better microcontroller?
### Nolan
  * You, like Ledgerwood, have an ability to dream up things that would violate assumptions. Best to get a decent way down the dreamt path to ensure other requirements are met still.
    * For example, grayscale charlieplexing is cool, but even just the firmware implementation requires much more thought in order to be low power and be de-risked. DMA engine from pre-calculated RAM buffer to GPIOs in 256-step frequency, can't use different timer channels for this!
  * You can still have RGB addressable LEDs and low power, just have a nfet on the VCC line to the LEDs and only turn them on for .5ms at a time. Need to test more though (what is their startup time?)






