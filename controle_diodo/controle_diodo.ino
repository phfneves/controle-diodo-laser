#include <TFT_HX8357.h>  // Hardware-specific library
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <math.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setupDisplay();
}

void loop() {
  loopDisplay();
}

