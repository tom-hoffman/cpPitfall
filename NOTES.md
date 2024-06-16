# Notes and references for Circuit Pitfall

## Gotchas for M0 processors
https://learn.adafruit.com/adafruit-feather-m0-basic-proto/adapting-sketches-to-m0

## Steps to add an object
* add to parseRoom
* maybe need parsing code
* might need update/parsing code
* might need update/flashing code for loop()
* add to drawCell
* add getColor.

## First sequence of rooms:
196 -> 137 -> 18 -> 37 -> 75

Pit layout: .T3210123. (remember it is reversed l/r)

##Memory 
                                    SKETCH        GLOBAL (128)  AVAILABLE
CircuitPlayground library only:     30048 (11%)   4644 (14%)    28124
cpPitfall (3/3/2024):               35744 (13%)   4816 (14%)    27952
                                    5696 (5.6K)   172 bytes   

## Music
Sound tables start at 2586 in disassembled code.
The 2600 could make 32 mostly out of tune pitches.
Presumably most of the numbers in the table are the pitches, 
since they're almost all $00 - $1F.  Some that are out of range might be
end codes.
https://garrykitchen.medium.com/how-i-fixed-ataris-awful-music-2d21abd7930d
https://www.randomterrain.com/atari-2600-memories-music-and-sound.html

## Object spawning behaviors / re-entering rooms
### All ropes and pits share the same changing positions across rooms..
For the pit status we can use some tidy binary math with 64 steps in 128 ms 
increments calculated only in active pit rooms.
Vine location is a slightly slower cycle, and I can't think of how to 
make this work as elegantly from a binary perspective.  
Probably need to constantly track the rope location with
"next step" time and direction bit.
* logs respawn at defined spawn points, even if the room is left for only a moment.

## Using hardware timers
The 2600 used a hardware timer on its "RIOT" chip (PIA 6532). 
This was an 8 bit timer with prescaling to 1, 8, 64 or 1024 cycles at 1.19MHz.
The Circuit Playground's ATSAMD21 ARM Cortex M0 Processor has a complex
clock signal distribution system as descibed nicely at:
https://www.lucadavidian.com/arduino-m0-pro-il-sistema-di-clock/
This is also helpful in understanding the complexity of the timing API of this board:
https://www.lucadavidian.com/hello-world/
So we could build a lot of the time-based actions (log rolling, pits opening, vine swinging)
around multiple counters, although that brings with it complexity in terms of 
understanding SAMD21 chips specifically.  In particular just "Arduino" documentation
does not apply directly.  
The upshot is I'm going to use one hardware timer, plus multiple software timers
based off the millis() timer.
### Other Timing links
https://blog.thea.codes/understanding-the-sam-d21-clocks/
https://emalliab.wordpress.com/2021/04/16/comparing-timers-on-samd21-and-samd51-microcontrollers/

## Timing Observations:
Based on original with rounding.  Adjust as needed.
* running across screen 5 seconds (probably keep this and adjust other timings)
* running across one cell 0.5 seconds
* original pit closed time 2.5 seconds (our pit is 7 wide to accomodate crocs)
* our pit closed time should start at 4 seconds
* open cycle 1.5 seconds
* pit cycle runs continuously
* full vine cycle -> 7+ seconds let's say 7.2 (9 * 8)?
* vine steps = 12 7.2/12 = 0.6, so *average* step takes 600 millis
* treat the vine like a moving object, not a mask.

Write an algorithm that will return the current state of the pit based on any input of millis.
* Harry's speed = 512 milli per cell
* crossing 7 cell pit = 8 * 512 = 4,096 (2^12)
* Pit speed timing:
  * pit closed - 8 * 512 
  * pit open/close = 512 each (4 * 128)
  * pit fully open = 6 * 512
  * TOTAL = 16 * 512 = 8,192 (2 ^ 13)
(the above is a lttle misleading since it is by 512 milli chunks, not 128)
The finest grained check is once every 128 millis.
There are 64 steps.

