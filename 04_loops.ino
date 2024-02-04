// Main loops for Circuit Pitfall.

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
    debugSetup();
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
