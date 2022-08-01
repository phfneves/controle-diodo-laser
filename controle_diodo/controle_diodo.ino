#include <TFT_HX8357.h>  // Hardware-specific library
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <math.h>
#include <TimerThree.h>
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
  Timer3.initialize(1000000);
  
  //initInterrupt();
}

void loop() {
  loopDisplay();
}

//ISR(TIMER4_COMPA_vect){
//
// Serial.println(getTemperature());
//}
