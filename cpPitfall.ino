#include <Adafruit_CircuitPlayground.h>

// cpPitfall -- Pitfall! for Circuit Playground.
// Based on Pitfall! created by David Crane for Activision,
// and on Johni Marangon's dissasembly and commentary at:
// https://github.com/johnidm/asm-atari-2600/blob/master/pitfall.asm

//=============================================================================
// C O M P I L E R - S W I T C H E S
//=============================================================================

#define DEBUG

//=============================================================================
// C O N S T A N T S
//=============================================================================

// initial value for the random number generator:
const byte RAND_SEED = 0xc4;        // defines the start scene of the game

// color constants
// Based on translating Atari 2600 colors to RGB, then adjusting as needed.
// https://www.randomterrain.com/atari-2600-memories-tia-color-charts.html

const int BROWN         = 0x3A1F00; // $12
const int YELLOW        = 0xFEFA40; // $1e
const int ORANGE        = 0xFEC6BB; // $3e
const int RED           = 0xE14585; // $48
const int GREEN         = 0x537E00; // $d6
const int BLUE          = 0x006957; // $a4
const int YELLOW_GREEN  = 0x49B509; // $c8
const int PINK          = 0xFE67AA; // $4a
const int BLACK         = 0x000000; // $00
const int GREY          = 0x5B5B5B; // $06
const int WHITE         = 0xFFFFFF; // $0e
const int DARK_GREEN    = 0x103600; // GREEN - $04
const int DARK_RED      = 0x6F001F; // RED - $06
const int COLOR_LST[]   = {BROWN, YELLOW, ORANGE, RED, GREEN, BLUE, 
                           YELLOW_GREEN, PINK, BLACK, GREY, WHITE, DARK_GREEN,
                           DARK_RED}; // variable in the original

//==============================================================================
// Z P - V A R I A B L E S
//==============================================================================

byte lives = 3;
byte room = RAND_SEED;              // in place of "random" in the original

void setup() {
  CircuitPlayground.begin();
  #ifdef DEBUG
    Serial.begin(9600);
    CircuitPlayground.setPixelColor(0, BROWN);
    CircuitPlayground.setPixelColor(1, YELLOW);
    CircuitPlayground.setPixelColor(2, ORANGE);
    CircuitPlayground.setPixelColor(3, RED);
    CircuitPlayground.setPixelColor(4, GREEN);
    CircuitPlayground.setPixelColor(5, BLUE);
    CircuitPlayground.setPixelColor(6, YELLOW_GREEN);
    CircuitPlayground.setPixelColor(7, PINK);
    CircuitPlayground.setPixelColor(8, DARK_GREEN);
    CircuitPlayground.setPixelColor(9, DARK_RED);
  #endif
}

void loop() {

}
