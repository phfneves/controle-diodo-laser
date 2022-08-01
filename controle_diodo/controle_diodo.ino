#include <math.h>
#include "converter_defs.h"

//void initInterrupt() {
//    //set timer4 interrupt at 1Hz
// TCCR4A = 0;// set entire TCCR1A register to 0
// TCCR4B = 0;// same for TCCR1B
// TCNT4  = 0;//initialize counter value to 0
// // set compare match register for 1hz increments
// OCR4A = 15624/1;// = (16*10^6) / (1*1024) - 1 (must be <65536)
// // turn on CTC mode
// TCCR4B |= (1 << WGM12);
// // Set CS12 and CS10 bits for 1024 prescaler
// TCCR4B |= (1 << CS12) | (1 << CS10);  
// // enable timer compare interrupt
// TIMSK4 |= (1 << OCIE4A);
//
//sei();//allow interrupts
//}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setupDisplay();
  initConverters();
}



void loop() {
  loopDisplay();
  float Vin = computeIControl(readIRef());
  float VCTLI = computeTControl(readTRef());
  Serial.print(readTRef());
  Serial.print(",");
  Serial.print(readIRef());
  Serial.print(",");
  Serial.print(VCTLI);
  Serial.print(",");
  Serial.print(Vin);
  Serial.print(",");
  Serial.print(getCurrent());
  Serial.print(",");
  Serial.println(getTemperature());
}

void updateIndicators(bool *currentOvershoot, bool *tempOvershoot){
  float I = getCurrent();
  float T = getTemperature();
  float IRef = readIRef();
  float TRef = readTRef();

  float Ierror = I - IRef;
  float Terror = T - TRef;
  
  if (abs(Ierror) >= 0.05*IRef)
    *currentOvershoot = true;
  else
    *currentOvershoot = false;

  if (abs(Terror) >= 0.05*TRef)
    *tempOvershoot = true;
  else
    *tempOvershoot = false;

}
