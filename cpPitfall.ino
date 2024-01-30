#include <Adafruit_CircuitPlayground.h>

// cpPitfall -- Pitfall! for Circuit Playground.
// Based on Pitfall! created by David Crane for Activision,
// on Johni Marangon's dissasembly and commentary at:
// https://github.com/johnidm/asm-atari-2600/blob/master/pitfall.asm
// and evoniuk's analysis at
// https://evoniuk.github.io/posts/pitfall.html

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


// Above ground background colors are shades of pure green.
// Anything not pure green is not background.
// In the original game these are different tree patterns.
const int BG_0            = gammaCorrect(0x001800);          
const int BG_1            = gammaCorrect(0x002400);          
const int BG_2            = gammaCorrect(0x003000);          
const int BG_3            = gammaCorrect(0x003C00); // first room
const int TREES[]         = {BG_0, BG_1, BG_2, BG_3};

// Object color designations
const int LOG_COLOR    = gammaCorrect(0x3B1F00);
const int SNAKE_COLORS[]  = {GREY, VIOLET};
const int FIRE_COLORS[]   = {DARK_RED, YELLOW};

const byte CELL_COUNT     = 10; // In case you want a bigger led string.

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
const byte VINE_BIT     = 1;  // 1      vine**
const byte MDANGER_BIT  = 2;  // 2      mobile danger (log/scorpion)**
const byte DANGER_BIT   = 3;  // 3      stationary danger (cobra/fire/log)
const byte CROC_BIT     = 4;  // 4      crocodile**
const byte PIT_BIT      = 5;  // 5      pit (all)
const byte HOLE_BIT     = 6;  // 6      hole/ladder
const byte TREASURE_BIT = 7;  // 7      treasure
                              // ** needs overlay graphic
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

void initCells() {
  for (byte i = 0; i < CELL_COUNT; i++) {
    cells[i] = 0;
  }
}

//============
// T I M I N G 
//============

// flash is for deadly dangers
const byte flashPeriod   = 255;
      long nextFlash      = flashPeriod;

//==============================================================================
// R O O M - F U N C T I O N S
//==============================================================================

// Linear Feedback Shift Register calculations

void lfsrRight() {
  // Changes room register, no return value.
  room = room << 1 | (bitRead(room, 3) ^ bitRead(room, 4) ^ 
    bitRead(room, 5) ^ bitRead(room, 7));
}

void lfsrLeft() {
  // Changes room register, no return value.
  room = room >> 1 | ((bitRead(room, 4) ^ bitRead(room, 5) ^ 
      bitRead(room, 6) ^ bitRead(room, 0)) * 128);
}

void nextRandom(boolean right, boolean above) {
  // Uses linear feedback shift register to generate next room 
  byte x = 3;       // repeat 3 times if below ground
  if (above) {      // just once if above ground
    x = 1;
  }
  for (byte i = 0; i < x; i++) {
    if (right) {
      lfsrRight();
    }
    else {
      lfsrLeft();
    }
  dangers = room & 0b111;
  initCells();
  }
  #ifdef DEBUG
    Serial.print("Entering room ");
    Serial.println(room);
  #endif
}

// movement convenience functions

void nextRight() {
  // XXX needs to check above/below.
  nextRandom(true, true);
  drawRoom();
}

void nextLeft() {
  // XXX needs to check above/below.
  nextRandom(false, true);
  drawRoom();
}

// color convenience functions

int getBackgroundColor() {
  // Determined by bits six and seven.
  return TREES[room >> 6];
}

int getFlash() {
  return bitRead(timers, FLASH_BIT);
}

void drawDanger(byte cell) {
  byte sw = bitRead(timers, FLASH_BIT);
  switch (dangers) {
    case LOG1: 
    case LOG3: 
    case MLOG2S: 
    case MLOG2G: 
    case MLOG3:
      CircuitPlayground.setPixelColor(cell, LOG_COLOR);
      break;
    case FIRE:
      CircuitPlayground.setPixelColor(cell, FIRE_COLORS[sw]);
      break;
    case SNAKE:
      CircuitPlayground.setPixelColor(cell, SNAKE_COLORS[sw]);
      break;
    default:
      CircuitPlayground.setPixelColor(cell, getBackgroundColor());
      break;
  }
}

void moveLogs() {
  for (byte i = 9; i > 0; i--) {
    //
  }
}

boolean containsDanger(byte c) {
  return ((bitRead(cells[c], DANGER_BIT)) || (bitRead(cells[c], MDANGER_BIT)));
}

void updateDangers() {
  if (!(bitRead(dangers, 2))) {
    moveLogs();
  }
  for (byte i = 0; i < CELL_COUNT; i++) {
    if (containsDanger(i)) {
      drawDanger(i);
    }
  }
}

void drawCell(byte cell) {
  // Current state of the cell.
  // These need to be sequenced by precedence.
  if bitRead(cells[cell], DANGER_BIT) {
    drawDanger(cell);
  }
  else {
    CircuitPlayground.setPixelColor(cell, getBackgroundColor());
  }
}

void writeCell(byte cell, byte bit, byte value) {
  // Convenience method to write a specific bit to a cell.
  bitWrite(cells[cell], bit, value);
}

void parseMLogs() {
  writeCell(1, MDANGER_BIT, 1);         // mobile logs
  if (dangers == MLOG2S) {              // 2 logs close
    writeCell(3, MDANGER_BIT, 1);
  }                 
  else if (dangers == MLOG2G) {         // 2 spaced logs
    writeCell(4, MDANGER_BIT, 1);
  }
  else if (dangers == MLOG3) {
    writeCell(4, MDANGER_BIT, 1);
    writeCell(8, MDANGER_BIT, 1);
  }
}

void parseDangers() {
  writeCell(1, DANGER_BIT, 1);
  if (dangers == LOG3) {                 // three stationary
    writeCell(8, DANGER_BIT, 1);          
    writeCell(0, DANGER_BIT, 1);
  }
}

void parseRoom() {
  if (bitRead(room, 2)) {                  // all stationary dangers
    parseDangers();
  }
  else {   
    parseMLogs();
  }
}


void drawRoom() {
  // This is the initial draw of the whole room.
  parseRoom(); 
  for (byte i = 0; i < CELL_COUNT; i++) {
    drawCell(i);
  }
}

void setup() {
  CircuitPlayground.begin(255);  // set to max. brightness to access full range
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_2_G);  // max. sensitivity
  CircuitPlayground.redLED(false);
  // I N T E R R U P T S
  // right button -> jump -> D4
  // left button  -> jump -> D5
  attachInterrupt(digitalPinToInterrupt(4), nextRight, RISING); 
  attachInterrupt(digitalPinToInterrupt(5), nextLeft, RISING);  
  #ifdef DEBUG
    Serial.begin(9600);
    delay(3000);
    Serial.println("Initializing Circuit Pitfall...");
    CircuitPlayground.setPixelColor(0, GREY);
    CircuitPlayground.setPixelColor(1, YELLOW);
    CircuitPlayground.setPixelColor(2, ORANGE);
    CircuitPlayground.setPixelColor(3, RED);
    CircuitPlayground.setPixelColor(4, GREEN);
    CircuitPlayground.setPixelColor(5, BLUE);
    CircuitPlayground.setPixelColor(6, YELLOW_GREEN);
    CircuitPlayground.setPixelColor(7, PINK);
    CircuitPlayground.setPixelColor(8, DARK_GREEN);
    CircuitPlayground.setPixelColor(9, DARK_RED);
    delay(500);
  #endif
  initCells();
  drawRoom();
}

void loop() {
  // try reading millis() as we go rather than storing per loop
  if (millis() > nextFlash) {
    updateDangers();
    nextFlash = nextFlash + flashPeriod;
    timers = timers ^ FLASH_MASK; // XOR to toggle the flash bit
  }
}

//==============================================================================
// E N D - N O T E S
//==============================================================================

// First sequence of rooms:
// 196 -> 137 -> 18 -> 37 -> 75
