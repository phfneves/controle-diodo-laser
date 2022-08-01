#define RT0 1000   // Ω
#define B 3460      // K
#define VCC 5    //Supply voltage
#define R 9830  //R=10KΩ

float RT, VR, ln, TX, T0, VRT;

float getTemperature() {
  int16_t intT = adc.readADC_SingleEnded(ADC_T_CH);
  VRT = intT * ADC_BIT_VOLTAGE;     //Conversion to voltage
  VR = VCC - VRT;
  RT = VRT / (VR / R);               //Resistance of RT

  ln = log(RT / RT0);
  TX = (1 / ((ln / B) + (1 / T0))); //Temperature from thermistor

  TX = 273.15 - TX;                //Conversion to Celsius

  return TX;
}
