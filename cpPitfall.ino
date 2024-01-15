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
// Sticking to original palette as much as possible.
// https://www.randomterrain.com/atari-2600-memories-tia-color-charts.html
// Minimum value of each RGB is 0x0D.

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
// Above ground background colors are shades of pure green.
// Anything not pure green is not background.
// In the original game these are different tree patterns.
const int BG_0          = 0x000D00;
const int BG_1          = 0x001A00;
const int BG_2          = 0x002700;
const int BG_3          = 0x003400;
const int TREES[]       = {BG_0, BG_1, BG_2, BG_3};
const int BARREL_COLOR  = BROWN;

const byte CELL_COUNT   = 10; // In case you want a bigger led string.
const byte SPAWN_POINT  = 1;  // For stationary/on-screen dangers.

//===================================
// C E L L - C O N T E N T S
//===================================
                              // bit -- type
                              // above ground
const byte HARRY_BIT    = 0;  // 0      harry
const byte VINE_BIT     = 1;  // 1      vine**
const byte MDANGER_BIT  = 2;  // 2      mobile danger (log/fire)**
const byte DANGER_BIT   = 3;  // 3      stationary danger (cobra/fire/log)
const byte CROC_BIT     = 4;  // 4      crocodile**
const byte PIT_BIT      = 5;  // 5      pit (all)
const byte HOLE_BIT     = 6;  // 6      hole/ladder
const byte TREASURE_BIT = 7;  // 7      treasure
                              // ** needs overlay graphic


//==============================================================================
// Z P - V A R I A B L E S
//==============================================================================

volatile  byte lives = 3;
volatile  byte room = RAND_SEED;              // in place of original's "random" 
volatile  boolean above = true;               // are you above ground?
volatile  byte cells[CELL_COUNT];             // indicates contents of each cell

void initCells() {
  for (byte i = 0; i < CELL_COUNT; i++) {
    cells[i] = 0;
  }
}

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

int getBackgroundColor() {
  // Determined by bits six and seven.
  return TREES[room >> 6];
}

void drawDanger(byte cell) {
  switch (room & 0b111) {
    case 4:
      Serial.println(room & 0b111);
      CircuitPlayground.setPixelColor(cell, BARREL_COLOR);
      break;
    default:
      CircuitPlayground.setPixelColor(cell, getBackgroundColor());
      break;
  }
}

void drawCell(byte cell) {
  // These need to be sequenced by precedent.
  if bitRead(cells[cell], DANGER_BIT) {
    drawDanger(cell);
  }
  else {
    CircuitPlayground.setPixelColor(cell, getBackgroundColor());
  }
}

void drawRoom() {
  parseRoom(); // XXX Might not want this here.
  for (byte i = 0; i < CELL_COUNT; i++) {
    drawCell(i);
  }
}

void writeCell(byte cell, byte bit, byte value) {
  bitWrite(cells[cell], bit, value);
}

void parseRoom() {
  // bits 0-2 = dangers
  if (bitRead(room, 2)) {                   // stationary dangers
    writeCell(SPAWN_POINT, DANGER_BIT, 1);
  }
}

void setup() {
  CircuitPlayground.begin();
  //===================================
  // I N T E R R U P T S
  //===================================
  // 
  // right button -> jump -> D4
  // left button  -> jump -> D5
  // right run    -> A5   -> D2
  // right down   -> A4   -> D3
  // left run     -> A2   -> D9
  // left down    -> A3   -> D10
  //
  attachInterrupt(digitalPinToInterrupt(4), nextRight, RISING); 
  attachInterrupt(digitalPinToInterrupt(5), nextLeft, RISING);  
  #ifdef DEBUG
    Serial.begin(9600);
    delay(3000);
    Serial.println("Initializing Circuit Pitfall...");
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
    delay(500);
  #endif
  initCells();
  drawRoom();
}

void loop() {

}

//==============================================================================
// E N D - N O T E S
//==============================================================================


// 
// under ground
// 0      harry
// 1      mobile danger
// 2      treasure
// 3      wall


// First sequence of rooms:
// 196 -> 137 -> 18 -> 37 -> 75
