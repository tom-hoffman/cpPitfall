// Circuit Pitfall rooms
// Functions that parse rooms and the gameplay they contain.
// Essentially most of the core functions.

byte lfsrRight(byte r) {
  // Changes room register, no return value.
  return r << 1 | (bitRead(r, 3) ^ bitRead(r, 4) ^ bitRead(r, 5) ^ bitRead(r, 7));
}

byte lfsrLeft(byte r) {
  // Changes room register, no return value.
  return r >> 1 | ((bitRead(r, 4) ^ bitRead(r, 5) ^ bitRead(r, 6) ^ bitRead(r, 0)) * 128);
}

void resetRoom() {
  dangers = room & 0b111;
  holes   = (room >> 3) & 0b111;
  roomContainsTreasure = checkRoomForTreasure();
  initCells();
  dirtyCells        = 0b1111111111;
}

void nextRandom(boolean right, boolean above) {
  // Uses linear feedback shift register to generate next room 
  byte x = 3;       // repeat 3 times if below ground
  if (above) {      // just once if above ground
    x = 1;
  }
  for (byte i = 0; i < x; i++) {
    if (right) {
      room = lfsrRight(room);
    }
    else {
      room = lfsrLeft(room);
    }
  resetRoom();
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
  parseRoom();
  drawRoom();
}

void nextLeft() {
  // XXX needs to check above/below.
  nextRandom(false, true);
  parseRoom();
  drawRoom();
}

void drawDanger(byte cell) {
  // this will become getDangerColor
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
      CircuitPlayground.setPixelColor(cell, getBackgroundColor(cell));
      break;
  }
}

void moveLogs() {
  for (byte i = 9; i > 0; i--) {
    // tbd
  }
}

void updateDangers() {
  for (byte i = 0; i < CELL_COUNT; i++) {
    if (containsDanger(i)) {
      drawDanger(i);
    }
  }
}

void flashDangers() {
  if (!(bitRead(dangers, 2))) {
    moveLogs(); //tbd
  }
  updateDangers(); 
  nextFlash = nextFlash + FLASH_PERIOD;
  timers = timers ^ FLASH_MASK; // XOR to toggle the flash bit
}

void flickerTreasure() {
  if (millis() > nextFlicker) {
    bool fstate = getFlicker();
    if (fstate) {
      nextFlicker = nextFlicker + SHORT_FLICKER;
    }
    else {
      nextFlicker = nextFlicker + LONG_FLICKER;
    }
    timers = timers ^ FLICKER_MASK;
    bitWrite(dirtyCells, TREASURE_SPAWN, 1);
  }
}

void drawCell(byte cell) {
  // Current state of the cell.
  // These need to be sequenced by precedence.
  if (containsTreasure(cell)) {
    CircuitPlayground.setPixelColor(cell, getTreasureColor());
  }
  else {
    CircuitPlayground.setPixelColor(cell, getBackgroundColor(cell));
  }
  bitWrite(dirtyCells, cell, 0);
}

void parseMLogs() {

}

void parseDangers() {

}

void parseObjectMask(short mask, byte offset) {
  // Reads the starting positions from a binary mask; 
  // writes the value to the cells array.
  for (byte i = 0; i <= CELL_COUNT; i++) {
    if (bitRead(mask, i)) {
      writeCell(i, offset, 1);
    }
  }
}

void parseBackground() {
  parseObjectMask(BACKGROUND_MASKS[room >> 6], TREE_BIT);
}

void parseRoom() {
  // this is the main flow chart
  parseBackground();
  // if bits 3-5 are 101 (5)
  // place treasure NOT holes/crocs/tar/quicksand/water
  Serial.println(holes);
  if (holes == 5) {
    writeCell(TREASURE_SPAWN, TREASURE_BIT, 1);
  }
} 
  // if bits 3-5 are 100 (4)
  // place crocs, no objects 
  // AND if bits 0-2 are 010, 011, 110 or 111, add a vine
  // OTHERWISE
  // bits 3-5 determine pits
  // bits 0-2 determine logs/fire/snake

void drawRoom() {
  for (byte i = 0; i < CELL_COUNT; i++) {
    if (bitRead(dirtyCells, i)) {
      drawCell(i);
    }
  }
}
