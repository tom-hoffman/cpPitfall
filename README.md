# Circuit Pitfall
Pitfall! for Adafruit's Circuit Playground

Based on Pitfall! created by David Crane for Activision,

on Johni Marangon's dissasembly and commentary at:

https://github.com/johnidm/asm-atari-2600/blob/master/pitfall.asm

evoniuk's analysis at

https://evoniuk.github.io/posts/pitfall.html

and Ben Valdes's map at:

https://pitfallharry.tripod.com/MapRoom/PitfallMap.png

## Status
Pre-alpha development.  Committed code should compile and run on a Circuit Playground express, but definitely is not a game (yet).

## Goals

* A completely playable Pitfall game running on a stock Circuit Playground Express.
* The linear room scrolling gameplay is represented as circular on the CP LED's.
* Every object, room and level in the game represented in its proper location.
* Coded using only base Arduino C and the CircuitPlayground library.
* When practical, use binary numbers and operators.
* Keep the overall code lean, simple and fast, especially using minimal memory to store game state.
* Yes, I am using a bunch of global variables, but this is intentional and intended to be close in style to Pitfall!'s 6502 assembly code.
