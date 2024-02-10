// Circuit Pitfall rooms
// Functions that parse rooms and the gameplay they contain.
// Essentially most of the core functions.

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

void resetRoom() {
  dangers = room & 0b111;
  holes   = room & 0b111000;
  initCells();
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
  drawRoom();
}

void nextLeft() {
  // XXX needs to check above/below.
  nextRandom(false, true);
  drawRoom();
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
      CircuitPlayground.setPixelColor(cell, getBackgroundColor(cell));
      break;
  }
}

void moveLogs() {
  for (byte i = 9; i > 0; i--) {
    //
  }
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
  CircuitPlayground.setPixelColor(cell, getBackgroundColor(cell));
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
  // treasures are determined by bits 0-2
  // if bits 3-5 are 100 (4)
  // place crocs, no objects 
  // AND if bits 0-2 are 010, 011, 110 or 111, add a vine
  // OTHERWISE
  // bits 3-5 determine pits
  // bits 0-2 determine logs/fire/snake

}



void drawRoom() {
  // This is the initial draw of the whole room.
  parseRoom(); 
  for (byte i = 0; i < CELL_COUNT; i++) {
    drawCell(i);
  }
}
