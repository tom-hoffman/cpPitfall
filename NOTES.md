# Notes and references for Circuit Pitfall

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

## Music
Sound tables start at 2586 in disassembled code.
The 2600 could make 32 mostly out of tune pitches.
Presumably most of the numbers in the table are the pitches, 
since they're almost all $00 - $1F.  Some that are out of range might be
end codes.
https://garrykitchen.medium.com/how-i-fixed-ataris-awful-music-2d21abd7930d
https://www.randomterrain.com/atari-2600-memories-music-and-sound.html

## Timing:
Based on original with rounding.  Adjust as needed.
* running across screen 5 seconds (probably keep this and adjust other timings)
* running across one cell 0.5 seconds
* original pit closed time 2.5 seconds (our pit is 7 wide to accomodate crocs)
* our pit closed time should start at 4 seconds
* open cycle 1.5 seconds
* pit cycle runs continuously

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

