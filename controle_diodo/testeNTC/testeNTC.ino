/*thermistor parameters:
 * RT0: 1 000 Ω
 * B: 3460 K +- 0.75%
 * T0:  25 C
 * +- 5%
 */

//These values are in the datasheet
//#define RT0 10000   // Ω
#define RT0 9830   // Ω
#define B 3460      // K
//--------------------------------------

#define VCC 5    //Supply voltage
#define R 10000  //R=10KΩ

//Variables
float RT, VR, ln, TX, T0, VRT;

// float voltages[] = [1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0];
// float temps[]= [1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0];
// float time[] = [1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  T0 = 25 + 273.15;                 //Temperature T0 from datasheet, conversion from Celsius to kelvin
}

void loop() {
  // put your main code here, to run repeatedly:
  VRT = analogRead(A0);              //Acquisition analog value of VRT
  VRT = (5.00 / 1023.00) * VRT;      //Conversion to voltage
  VR = VCC - VRT;
  RT = VRT / (VR / R);               //Resistance of RT

  ln = log(RT / RT0);
  TX = (1 / ((ln / B) + (1 / T0))); //Temperature from thermistor

  TX = 273.15- TX;                 //Conversion to Celsius

  // Serial.print("Voltage:");
  // Serial.print(VRT);
  // Serial.print(" , Temperature:");
  Serial.println(TX);
  // Serial.println("C");

  delay(500);
}
