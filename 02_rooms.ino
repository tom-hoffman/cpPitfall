// Circuit Pitfall rooms
// Functions that parse rooms and the gameplay they contain.
// Essentially most of the core functions.

uint8_t lfsrRight(uint8_t r) {
  // Changes room register, no return value.
  return r << 1 | (bitRead(r, 3) ^ bitRead(r, 4) ^ bitRead(r, 5) ^ bitRead(r, 7));
}

uint8_t lfsrLeft(uint8_t r) {
  // Changes room register, no return value.
  return r >> 1 | ((bitRead(r, 4) ^ bitRead(r, 5) ^ bitRead(r, 6) ^ bitRead(r, 0)) * 128);
}

void nextRandom(boolean right, boolean above) {
  // Uses linear feedback shift register to generate next room 
  uint8_t x = 3;       // repeat 3 times if below ground
  if (above) {      // just once if above ground
    x = 1;
  }
  for (uint8_t i = 0; i < x; i++) {
    if (right) {
      room = lfsrRight(room);
    }
    else {
      room = lfsrLeft(room);
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
  parseRoom();
  drawRoom();
}

void nextLeft() {
  // XXX needs to check above/below.
  nextRandom(false, true);
  parseRoom();
  drawRoom();
}

uint32_t getDangerColor() {
  uint8_t sw = bitRead(timers, FLASH_BIT);
  switch (bits0to2) {
    case FIRE:
      return FIRE_COLORS[sw];
    case SNAKE:
      return SNAKE_COLORS[sw];
    default:
      return LOG_COLOR;
  }
}

uint32_t getCrocColor() {
  return CROC_COLORS[bitRead(timers, CROC_FLASH_BIT) + 
                     bitRead(timers, CROC_DANGER_BIT)];
}

void flashDangers() { // old code
  // first move the mobile logs
  if (!(bitRead(bits0to2, 2))) {
    moveLogs(); //tbd
  }
  // loop through and find the FIRE or SNAKE to mark as dirty.

  nextFlash = nextFlash + FLASH_PERIOD;
  timers = timers ^ FLASH_MASK; // XOR to toggle the flash bit
}

void moveLogs() { // old code
  for (byte i = 9; i > 0; i--) {
    // tbd
  }
}

void flickerTreasure() {
  // couldn't get the quick flicker with the bit shift method
  if (checkRoomForTreasure()) {
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
}

void drawCell(uint8_t cell) {
  // Current state of the cell.
  // These need to be sequenced by precedence.
  // There may be multiple contents but just one color is shown.
  if (cellContainsTreasure(cell)) {
    CircuitPlayground.setPixelColor(cell, getTreasureColor());
  }
  else if (cellContainsCroc(cell)) {
    CircuitPlayground.setPixelColor(cell, getCrocColor());
  }
  else if (cellContainsDanger(cell)) {
    CircuitPlayground.setPixelColor(cell, getDangerColor());
  }
  else if (cellContainsPit(cell)) {
    CircuitPlayground.setPixelColor(cell, getPitColor());
  }
  else { // could be optimized
    CircuitPlayground.setPixelColor(cell, getBackgroundColor(cell)); 
  }
  bitWrite(dirtyCells, cell, 0);
}

void parseObjectMask(uint16_t mask, uint8_t offset) {
  // Reads the starting positions from a binary mask; 
  // writes the value to the cells array.
  for (uint8_t i = 0; i <= CELL_COUNT; i++) {
    if (bitRead(mask, i)) {
      writeCell(i, offset, 1);
    }
    else {
      writeCell(i, offset, 0);
    }
  }
}

void parseBackground() {
  parseObjectMask(BACKGROUND_MASKS[room >> 6], TREE_BIT);
}

uint16_t getShiftingPitMask(uint8_t n) {
  if (n < 32)         {return PIT0_MASK;}
  else if (n < 33)    {return PIT1_MASK;}
  else if (n < 34)    {return PIT2_MASK;}
  else if (n < 36)    {return PIT3_MASK;}
  else if (n < 60)    {return PIT4_MASK;}
  else if (n < 62)    {return PIT3_MASK;}
  else if (n < 63)    {return PIT2_MASK;}
  else                {return PIT1_MASK;}
}

void updateShiftingPit() {
  // This is not called by parseRoom() because it has to be
  // frequently updated from the beginning.
  // The finest grained check is once every 128 millis.
  // There are 64 steps.
  byte n = ((millis() >> 6) & 0b00111111);
  uint16_t newMask = getShiftingPitMask(n);
  if (newMask != currentPitMask) {
    parseObjectMask(newMask, PIT_BIT);
    currentPitMask = newMask;
    dirtyCells = 0b0111111100; // could be more optimized
  }
}

void updateCrocs() {
  uint32_t mil = millis();
  byte n = ((mil >> 11) & 0b0001);
  if (n != bitRead(timers, CROC_FLASH_BIT)) {
    bitWrite(timers, CROC_FLASH_BIT, n);
    bitWrite(timers, CROC_DANGER_BIT, 0);
    dirtyCells = dirtyCells | CROC_MASK;
  }
  if (n) {
    byte s = ((mil >> 5) & 0b0001);
    if (s != bitRead(timers, CROC_DANGER_BIT)) {
      bitWrite(timers, CROC_DANGER_BIT, s);
      dirtyCells = dirtyCells | CROC_MASK;
    }
  }
}

void parseRoom() {
  // This is the main flow chart for parsing a new room.
  // First reset stuff.
  bits0to2 = room & 0b111;
  bits3to5 = (room >> 3) & 0b111;
  initCells();
  dirtyCells        = 0b1111111111;
  // Parse new room.
  parseBackground();
  // if bits 3-5 are 101 (5)
  // place treasure NOT holes/crocs/tar/quicksand/water
  if (roomIsTreasureRoom()) {
    writeCell(TREASURE_SPAWN, TREASURE_BIT, 1);
  }
  // if bits 3-5 are 100 (4)
  // place crocs, no objects 
  else if (roomHasCrocs()) {
    parseObjectMask(CROC_MASK, CROC_BIT);
    parseObjectMask(PIT4_MASK, PIT_BIT);
  }
} 

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
