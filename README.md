## Requirements
  * Have 30-50 boards designed, produced, and flashed in time for team banquet. (6 weeks)
  * Under $8 each
  * Inspire students to create their own PCB designs next school year with mentor support
  * If there is power needed, the battery needs to last at least one competition or be easily rechargeable with USB
  * It needs to look good/presentable -- have some kind of art on it.

### Nice-To-Have
  * Be understandable / modifiable by interested students. *While I would argue firmware modification is still possible, Brent disagrees*
  * Have multiple patterns

## Post-Mortem



## Learnings for next time
### Nolan
  * You, like Ledgerwood, have an ability to dream up things that would violate assumptions. Best to get a decent way down the dreamt path to ensure other requirements are met still.
    * For example, grayscale charlieplexing is cool, but even just the firmware implementation requires much more thought in order to be low power and be de-risked. DMA engine from pre-calculated RAM buffer to GPIOs in 256-step frequency, can't use different timer channels for this!
  * You can still have RGB addressable LEDs and low power, just have a nfet on the VCC line to the LEDs and only turn them on for .5ms at a time. Need to test more though.




## Patterns
  * Breathing like Macbook
  * Pulsing with beats like pendulum.
  * Others?

https://wokwi.com/projects/399273653422371841
