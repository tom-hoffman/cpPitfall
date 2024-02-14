// Various utility functions for Circuit Pitfall.

void initCells() {
  for (byte i = 0; i < CELL_COUNT; i++) {
    cells[i] = 0;
  }
}

void initTreasureArray() {
  byte arrayIndex = 0;
  byte last = lfsrLeft(RAND_SEED);
  for (byte rm = RAND_SEED; rm != last; rm = lfsrRight(rm)) {
    if (((rm >> 3) & 0b111) == 5) {
      treasureArray[arrayIndex] = rm;
      arrayIndex++;
    }
  }
}

bool checkRoomForTreasure() {
  bool t = false;
  for (byte i = 0; i < 32; i++) {
    if (treasureArray[i] == room) {
      t = true;
    }
  }
  return t;
}

bool roomIsTreasureRoom() {
  return holes == 5;
}

bool roomHasMobileLogs() {
  return (!(roomIsTreasureRoom()) && (!(bitRead(dangers, 2))));
}

bool roomHasFire () {

}

bool roomHasSnake() {

}

bool roomHasStationaryLogs() {

}

void writeCell(byte cell, byte bit, byte value) {
  // Convenience method to write a specific bit to a cell.
  bitWrite(cells[cell], bit, value);
}


int getBackgroundColor(byte c) {
  if (bitRead(cells[c], TREE_BIT)) {
    return (TREE_COLOR);
  }
  else {
    return(BG_COLOR);
  }
}

int getTreasureColor() {
  // this returns the active flashing color
  if (bitRead(timers, FLICKER_BIT)) {
    return TREASURE_COLORS[dangers & 0b011]; // treasure type bits 1 0
  }
  else {
    return FLICKER_COLOR;
  }  
}



boolean cellContainsDanger(byte c) {
  return (bitRead(cells[c], DANGER_BIT));
}

boolean cellContainsTreasure(byte c) {
  return (bitRead(cells[c], TREASURE_BIT));
}

byte getFlash() {
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
