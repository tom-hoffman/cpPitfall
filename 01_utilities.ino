// Various utility functions for Circuit Pitfall.

void initCells() {
  for (uint8_t i = 0; i < CELL_COUNT; i++) {
    cells[i] = 0;
  }
}

void initTreasureArray() {
  uint8_t arrayIndex = 0;
  uint8_t last = lfsrLeft(RAND_SEED);
  for (uint8_t rm = RAND_SEED; rm != last; rm = lfsrRight(rm)) {
    if (((rm >> 3) & 0b111) == 5) {
      treasureArray[arrayIndex] = rm;
      arrayIndex++;
    }
  }
}

bool checkRoomForTreasure() {
  bool t = false;
  for (uint8_t i = 0; i < 32; i++) {
    if (treasureArray[i] == room) {
      t = true;
    }
  }
  return t;
}

bool roomIsTreasureRoom() {
  // True if room initially has treasure.  It might be gone.
  return bits3to5 == SHIFTING_TART;
}

bool roomHasShiftingPit() {
  // values 5-7 have shifting pits.
  return bits3to5 > 4;
}

bool roomHasCrocs() {
  return bits3to5 == CROCS;
}

bool roomHasDanger() {
  return !(roomIsTreasureRoom() || roomHasCrocs());
}

bool roomHasMobileLogs() {
  return (roomHasDanger() && (!(bitRead(bits0to2, 2))));
}
// may not need these...
bool roomHasOneMobileLog() {
  return (roomHasDanger() && (!(bitRead(bits0to2, MLOG1))));
}

bool roomHasTwoSilverLogs() {
  return (roomHasDanger() && (!(bitRead(bits0to2, MLOG2S))));
}

bool roomHasTwoGoldLogs() {
  return (roomHasDanger() && (!(bitRead(bits0to2, MLOG2G))));
}

bool roomHasThreeMobileLogs() {
  return (roomHasDanger() && (!(bitRead(bits0to2, MLOG3))));
}

bool roomHasOneStationaryLog() {
  return (roomHasDanger() && (!(bitRead(bits0to2, LOG1))));
}

bool roomHasThreeStationaryLogs() {
  return (roomHasDanger() && (!(bitRead(bits0to2, LOG3))));
}

bool roomHasFire() {
  return (roomHasDanger() && (bits0to2 = FIRE));
}

bool roomHasSnake() {
  return (roomHasDanger() && (bits0to2 = SNAKE));
}

void writeCell(uint8_t cell, uint8_t bit, uint8_t value) {
  // Convenience method to write a specific bit to a cell.
  bitWrite(cells[cell], bit, value);
}

uint32_t getBackgroundColor(uint8_t c) {
  if (bitRead(cells[c], TREE_BIT)) {
    return (TREE_COLOR);
  }
  else {
    return(BG_COLOR);
  }
}

uint32_t getTreasureColor() {
  // this returns the active flashing color
  if (bitRead(timers, FLICKER_BIT)) {
    return TREASURE_COLORS[bits0to2 & 0b011]; // treasure type bits 1 0
  }
  else {
    return FLICKER_COLOR;
  }  
}

uint32_t getPitColor() {
  switch (bits3to5) {
    case 2: case 5: case 6: 
      return TARPIT_COLOR; 
    case 3: case 7:
      return QUICKSAND_COLOR;
  }
}

bool cellContainsPit(uint8_t c) {
  return (bitRead(cells[c], PIT_BIT));
}

bool cellContainsDanger(uint8_t c) {
  return (bitRead(cells[c], DANGER_BIT));
}

bool cellContainsTreasure(uint8_t c) {
  return (bitRead(cells[c], TREASURE_BIT));
}

uint8_t getFlash() {
  return bitRead(timers, FLASH_BIT);
}

bool getFlicker() {
  return bitRead(timers, FLICKER_BIT);
}

void debugSetup() {
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
}
