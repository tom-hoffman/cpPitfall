// Various utility functions for Circuit Pitfall.

void initCells() {
  for (byte i = 0; i < CELL_COUNT; i++) {
    cells[i] = 0;
  }
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

boolean containsDanger(byte c) {
  return (bitRead(cells[c], DANGER_BIT));
}

int getFlash() {
  return bitRead(timers, FLASH_BIT);
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