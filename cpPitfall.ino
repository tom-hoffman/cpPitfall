#include <Adafruit_CircuitPlayground.h>

// cpPitfall -- Pitfall! for Circuit Playground.
// Based on Pitfall! created by David Crane for Activision,
// on Johni Marangon's dissasembly and commentary at:
// https://github.com/johnidm/asm-atari-2600/blob/master/pitfall.asm
// evoniuk's analysis at
// https://evoniuk.github.io/posts/pitfall.html
// and Ben Valdes's map at:
// https://pitfallharry.tripod.com/MapRoom/PitfallMap.png

// This file contains constant and varible declarations only.

//=============================================================================
// C O M P I L E R - S W I T C H E S
//=============================================================================

#define DEBUG 

//=============================================================================
// C O N S T A N T S
//=============================================================================

// initial value for the random number generator:
const uint8_t RAND_SEED    = 0xc4;        // defines the start scene of the game

// color constants
// Based on translating Atari 2600 colors to RGB, then adjusting as needed.
// Sticking to original colors and palette as much as practical.
// https://www.randomterrain.com/atari-2600-memories-tia-color-charts.html

int gammaCorrect(int c) {
  uint8_t red   = CircuitPlayground.gamma8(c >> 16);
  uint8_t green = CircuitPlayground.gamma8((c & 0x00FF00)  >> 8);
  uint8_t blue  = CircuitPlayground.gamma8(c & 0x0000FF);
  return ((red << 16L) | (green << 8L) | blue);
}
// trailing code comment is original color from disassembly
const uint32_t BROWN         = gammaCorrect(0x3A1F00); // $12
const uint32_t YELLOW        = gammaCorrect(0x5D4100); // $1e
const uint32_t ORANGE        = gammaCorrect(0xFEC6BB); // $3e
const uint32_t RED           = gammaCorrect(0xE14585); // $48
const uint32_t GREEN         = gammaCorrect(0x537E00); // $d6
const uint32_t BLUE          = gammaCorrect(0x006957); // $a4
const uint32_t YELLOW_GREEN  = gammaCorrect(0x49B509); // $c8
const uint32_t PINK          = gammaCorrect(0xFE67AA); // $4a
const uint32_t BLACK         = gammaCorrect(0x000000); // $00
const uint32_t GREY          = gammaCorrect(0x5B5B5B); // $06
const uint32_t WHITE         = gammaCorrect(0xFFFFFF); // $0e
const uint32_t DARK_GREEN    = gammaCorrect(0x103600); // GREEN - $04
const uint32_t DARK_RED      = gammaCorrect(0x6F001F); // RED - $06
// non-original colors
const uint32_t VIOLET        = gammaCorrect(0x7D058C);

const uint32_t BG_COLOR      = gammaCorrect(0x001800);
const uint32_t TREE_COLOR    = gammaCorrect(0x181800);        

// Object color designations
const uint32_t LOG_COLOR         = gammaCorrect(0x3B1F00);
const uint32_t SNAKE_COLORS[]    = {GREY, VIOLET};
const uint32_t FIRE_COLORS[]     = {DARK_RED, YELLOW};
// Treasure main colors (with flickering bright white)
const uint32_t MONEY_COLOR       = gammaCorrect(0x086B00);
const uint32_t SILVER_COLOR      = GREY;
const uint32_t GOLD_COLOR        = YELLOW;
const uint32_t RING_COLOR        = DARK_RED;
const uint32_t FLICKER_COLOR     = gammaCorrect(0xAAAAAA);
const uint32_t TREASURE_COLORS[] = {MONEY_COLOR, SILVER_COLOR, GOLD_COLOR, RING_COLOR};

const uint8_t CELL_COUNT       = 10; // In case you want a bigger led string.

// ==================================
// O B J E C T - M A S K S
// ==================================
// These indicate object placement in the 10 LED's using 10 bit binary numbers.
// NOTE: the left/right order is reversed from the gameplay.

const uint16_t BG0_MASK    = 0b1010000101;
const uint16_t BG1_MASK    = 0b0101001010;
const uint16_t BG2_MASK    = 0b0010101010;
const uint16_t BG3_MASK    = 0b0010110100;

const uint16_t BACKGROUND_MASKS[] = {BG0_MASK, BG1_MASK, 
                                     BG2_MASK, BG3_MASK};

const uint16_t CROC_MASK   = 0b0010101000;

const uint16_t HOLE3_MASK  = 0b0001010100;
const uint16_t LADDER_MASK = 0b0000010000;

const uint16_t LOG1_MASK   = 0b0010000000;
const uint16_t LOG2_MASK_A = 0b0110000000;
const uint16_t LOG2_MASK_B = 0b0100100000;
const uint16_t LOG3_MASK   = 0b0100100010;

const uint8_t        TREASURE_SPAWN = 1;

// pit animation masks
// layout: .T3210123. (remember it is reversed l/r)
const uint16_t PIT0_MASK   = 0b0000000000;
const uint16_t PIT1_MASK   = 0b0000010000;
const uint16_t PIT2_MASK   = 0b0000111000;
const uint16_t PIT3_MASK   = 0b0001111100;
const uint16_t PIT4_MASK   = 0b0011111110;
const uint16_t PIT_MASKS[] = {PIT0_MASK, PIT1_MASK, PIT2_MASK, 
                              PIT3_MASK, PIT4_MASK};

// ==================================
// R O O M - C O D E S
// ==================================
// D A N G E R - C O D E S
// bits 2 1 0
const uint8_t MLOG1  = 0;    // 000 - one moving log    - money
const uint8_t MLOG2S = 1;    // 001 - two moving logs   - silver
const uint8_t MLOG2G = 2;    // 010 - two moving logs   - gold
const uint8_t MLOG3  = 3;    // 011 - three moving logs - ring
const uint8_t LOG1   = 4;    // 100 - one log           - money
const uint8_t LOG3   = 5;    // 101 - three logs        - silver
const uint8_t FIRE   = 6;    // 110 - fire              - gold
const uint8_t SNAKE  = 7;    // 111 - snake             - ring
// T R E A S U R E - C O D E S
// bits 1 0
const uint8_t MONEY  = 0;    // 00
const uint8_t SILVER = 1;    // 01
const uint8_t GOLD   = 2;    // 10
const uint8_t RING   = 3;    // 11
// P I T - C O D E S
// bits 5 4 3
const uint8_t HOLE1         = 0;  // 000
const uint8_t HOLE3         = 1;  // 001
const uint8_t STATIC_TAR    = 2;  // 010
const uint8_t STATIC_QS     = 3;  // 011
const uint8_t CROCS         = 4;  // 100
const uint8_t SHIFTING_TART = 5;  // 101 (with treasure)
const uint8_t SHIFTING_TAR  = 6;  // 110 (no treasure)
const uint8_t SHIFTING_QS   = 7;  // 111

//===================================
// C E L L - C O N T E N T S
//===================================
                              // bit -- type
                              // above ground
const uint8_t HARRY_BIT    = 0;  // 0      harry
const uint8_t VINE_BIT     = 1;  // 1      vine
const uint8_t DANGER_BIT   = 2;  // 2      all non-croc dangers 
const uint8_t TREE_BIT     = 3;  // 3      background tree trunk
const uint8_t CROC_BIT     = 4;  // 4      crocodile
const uint8_t HOLE_BIT     = 5;  // 5      pit (all)
const uint8_t LADDER_BIT   = 6;  // 6      hole/ladder
const uint8_t TREASURE_BIT = 7;  // 7      treasure

//===================================
// T I M E R - B I T S
//===================================
                                               // bit --  type
const uint8_t FLASH_BIT           = 0;              // 0   danger flash/move
const uint8_t FLASH_MASK          = bit(FLASH_BIT);
const uint8_t FLICKER_BIT         = 1;              // 1   flicker treasure 
const uint8_t FLICKER_MASK        = bit(FLICKER_BIT);
const uint8_t PIT_TRANSITION_BIT  = 2;              // 2   pit in transition
const uint8_t PIT_TRANSITION_MASK = bit(PIT_TRANSITION_BIT);
const uint8_t PIT_STATUS_MASK     = 0b00011111;     // a five bit value
//==============================================================================
// V A R I A B L E S
//==============================================================================

          uint8_t    lives           = 3;         // 2 bits
          bool       above           = true;      // above/below ground
volatile  bool       jumping         = false;     // interrupt from tap?
          uint8_t    harryX          = 85;        // Harry's position from 0-100 

          uint8_t    timers          = 0;         // packed booleans for timers
          uint8_t    pitStatus       = 0;         // 5 bits of pit status
          uint8_t    room            = RAND_SEED; // original's "random" 
          uint8_t    cells[CELL_COUNT];           // contents of each cell
          uint8_t    bits0to2        = room & 0b111;
          uint8_t    bits3to5        = bits3to5   = (room >> 3) & 0b111;

          uint8_t    treasureArray[32];           // array of rooms with treasures   
bool      roomContainsTreasure    = false;        // current room has treasure?

uint16_t dirtyCells        = 0b1111111111;


//============
// T I M I N G 
//============

// flash is for deadly dangers
const uint8_t FLASH_PERIOD   = 255;
      uint32_t nextFlash     = FLASH_PERIOD;
// flicker is for treasure
const uint8_t SHORT_FLICKER  = 2;
const uint16_t LONG_FLICKER   = 256;
      uint32_t nextFlicker   = LONG_FLICKER;
