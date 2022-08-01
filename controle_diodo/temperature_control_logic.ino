#define RT0 1000   // Ω
#define B 3460      // K
//--------------------------------------
#define R 9830  //R=10KΩ


const float VCC = 5.0;
double RT, VR, ln, TX, VRT;
const double T0 = 25.0 + 273.15;
const double RX = RT0 * exp(-B/T0);

float VCTLI;

// Temperature control parameters
const float TKc = 10;
const float Tz = 0.8;

// State variables
float Te_km1, Tu_km1;

double getTemperature() {
  int16_t intT = adc.readADC_SingleEnded(ADC_T_CH);
  VR = intT * ADC_BIT_VOLTAGE;     //Conversion to voltage
  VRT = VCC - VR;
  // Determina a resistência do termistor
    
  RT = (VCC*R)/VR - R;
 
  // Calcula a temperatura
  double T = B / log(RT/RX);
  T = T - 273.15;

  return T;
}

float computeTControl(float TRef) {
  float T = getTemperature(); 
  // Compute control
  float Te_k = T - TRef;
  float Tu_k = Tu_km1 + TKc * Te_k - TKc * Tz * Te_km1;

 Tu_k = Tu_km1 + TKc * Te_k;

  // Offset and saturation
  Tu_k = max(Tu_k, MIN_VCTLI);
  Tu_k = min(Tu_k, MAX_VCTLI);

  // Output value update
  VCTLI = Tu_k;
  writeCTLI();
  // State variables update
  Tu_km1 = Tu_k;
  Te_km1 = Te_k;
  return VCTLI;
}

void writeCTLI() {
  float x = (VCTLI + VCTLI_OFFSET) / DAC_BIT_VOLTAGE;
  x = min(DAC_12BIT_MAX, x);
  Tdac.setVoltage(x, false);
}
