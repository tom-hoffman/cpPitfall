#include <Adafruit_CircuitPlayground.h>

// cpPitfall -- Pitfall! for Circuit Playground.
// Based on Pitfall! created by David Crane for Activision,
// on Johni Marangon's dissasembly and commentary at:
// https://github.com/johnidm/asm-atari-2600/blob/master/pitfall.asm
// evoniuk's analysis at
// https://evoniuk.github.io/posts/pitfall.html
// and Ben Valdes's map at:
// https://pitfallharry.tripod.com/MapRoom/PitfallMap.png

//=============================================================================
// C O M P I L E R - S W I T C H E S
//=============================================================================

#define DEBUG 

//=============================================================================
// C O N S T A N T S
//=============================================================================

// initial value for the random number generator:
const byte RAND_SEED    = 0xc4;        // defines the start scene of the game

// color constants
// Based on translating Atari 2600 colors to RGB, then adjusting as needed.
// Sticking to original colors and palette as much as practical.
// https://www.randomterrain.com/atari-2600-memories-tia-color-charts.html

int gammaCorrect(int c) {
  byte red   = CircuitPlayground.gamma8(c >> 16);
  byte green = CircuitPlayground.gamma8((c & 0x00FF00)  >> 8);
  byte blue  = CircuitPlayground.gamma8(c & 0x0000FF);
  return ((red << 16L) | (green << 8L) | blue);
}

const int BROWN         = gammaCorrect(0x3A1F00); // $12
const int YELLOW        = gammaCorrect(0x5D4100); // $1e
const int ORANGE        = gammaCorrect(0xFEC6BB); // $3e
const int RED           = gammaCorrect(0xE14585); // $48
const int GREEN         = gammaCorrect(0x537E00); // $d6
const int BLUE          = gammaCorrect(0x006957); // $a4
const int YELLOW_GREEN  = gammaCorrect(0x49B509); // $c8
const int PINK          = gammaCorrect(0xFE67AA); // $4a
const int BLACK         = gammaCorrect(0x000000); // $00
const int GREY          = gammaCorrect(0x5B5B5B); // $06
const int WHITE         = gammaCorrect(0xFFFFFF); // $0e
const int DARK_GREEN    = gammaCorrect(0x103600); // GREEN - $04
const int DARK_RED      = gammaCorrect(0x6F001F); // RED - $06
// non-original colors
const int VIOLET        = gammaCorrect(0x7D058C);

const int BG            = gammaCorrect(0x001800); // (deprecate)
const int BG_COLOR      = gammaCorrect(0x001800);
const int TREE_COLOR    = gammaCorrect(0x181800);        


// Object color designations
const int LOG_COLOR       = gammaCorrect(0x3B1F00);
const int SNAKE_COLORS[]  = {GREY, VIOLET};
const int FIRE_COLORS[]   = {DARK_RED, YELLOW};

const byte CELL_COUNT     = 10; // In case you want a bigger led string.

// ==================================
// O B J E C T - M A S K S
// ==================================
// These indicate object placement in the 10 LED's using 10 bit binary numbers.
// NOTE: the left/right order is reversed from the gameplay.
const unsigned short BG0_MASK    = 0b1010000101;
const unsigned short BG1_MASK    = 0b0101001010;
const unsigned short BG2_MASK    = 0b0010101010;
const unsigned short BG3_MASK    = 0b0010110100;

const unsigned short BACKGROUND_MASKS[] = {BG0_MASK, BG1_MASK, 
                                           BG2_MASK, BG3_MASK};

const unsigned short CROC_MASK   = 0b0010101000;

const unsigned short HOLE3_MASK  = 0b0010101000;
const unsigned short LADDER_MASK = 0b0000100000;

const unsigned short LOG1_MASK   = 0b0000000100;
const unsigned short LOG2_MASK_A = 0b0000000110;
const unsigned short LOG2_MASK_B = 0b0000010010;
const unsigned short LOG3_MASK   = 0b0100010010;

// ==================================
// R O O M - C O D E S
// ==================================

// D A N G E R - C O D E S
// bits 2 1 0
const byte MLOG1  = 0;    // 000 - one moving log    - money
const byte MLOG2S = 1;    // 001 - two moving logs   - silver
const byte MLOG2G = 2;    // 010 - two moving logs   - gold
const byte MLOG3  = 3;    // 011 - three moving logs - ring
const byte LOG1   = 4;    // 100 - one log           - money
const byte LOG3   = 5;    // 101 - three logs        - silver
const byte FIRE   = 6;    // 110 - fire              - gold
const byte SNAKE  = 7;    // 111 - snake             - ring
// T R E A S U R E - C O D E S
// bits 1 0
const byte MONEY  = 0;    // 00
const byte SILVER = 1;    // 01
const byte GOLD   = 2;    // 02
const byte RING   = 3;    // 03

//===================================
// C E L L - C O N T E N T S
//===================================
                              // bit -- type
                              // above ground
const byte HARRY_BIT    = 0;  // 0      harry
const byte VINE_BIT     = 1;  // 1      vine
const byte DANGER_BIT   = 2;  // 2      all non-croc dangers 
const byte TREE_BIT     = 3;  // 3      background tree trunk
const byte CROC_BIT     = 4;  // 4      crocodile
const byte PIT_BIT      = 5;  // 5      pit (all)
const byte HOLE_BIT     = 6;  // 6      hole/ladder
const byte TREASURE_BIT = 7;  // 7      treasure
//===================================
// T I M E R - B I T S
//===================================
                                          // bit -- type
const byte FLASH_BIT    = 0;              // 0      danger flash/move
const byte FLASH_MASK   = bit(FLASH_BIT);

//==============================================================================
// V A R I A B L E S
//==============================================================================

          byte    lives   = 3;        // 2 bits
          boolean above   = true;     // above/below ground
volatile  boolean jumping = false;    // directly changed by button interrupt
          byte    harryX  = 85;       // Harry's position on a 0-100 scale

          byte    timers  = 0;        // packed booleans for bool timers

          byte    room    = RAND_SEED; // in place of original's "random" 
          byte    cells[CELL_COUNT];   // indicates contents of each cell
          byte    dangers  = room & 0b111;



//============
// T I M I N G 
//============

// flash is for deadly dangers
const byte flashPeriod   = 255;
      long nextFlash      = flashPeriod;




