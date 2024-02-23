// Main loops for Circuit Pitfall.

void setup() {
  CircuitPlayground.begin(255);  // set to max. brightness to access full range
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_2_G);  // max. sensitivity
  CircuitPlayground.redLED(false);
  // I N T E R R U P T S
  // Probably these aren't going to stay interrupts
  // because of bounce management.
  // right button -> jump -> D4
  // left button  -> jump -> D5
  attachInterrupt(digitalPinToInterrupt(4), nextRight, RISING); 
  attachInterrupt(digitalPinToInterrupt(5), nextLeft, RISING);  
  #ifdef DEBUG
    debugSetup();
  #endif
  initTreasureArray();
  resetRoom();
  parseRoom();
  drawRoom();
}

void loop() {
  if (roomIsTreasureRoom()) {
    flickerTreasure();
  }
  if (roomHasShiftingPit()) {
    updateShiftingPit();
  }
  if (millis() > nextFlash) {
    flashDangers();
  }
  drawRoom();
}
