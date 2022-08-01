float Vin;

// Current control parameters
const float IKc = 0.01;
const float Iz = 0.7;

// State variables
float Ie_km1, Iu_km1;

float getCurrent() {
    int16_t intI = adc.readADC_SingleEnded(ADC_I_CH);
    float vI = intI * ADC_BIT_VOLTAGE;
    float I = vI * I_SENSE_GAIN + 2;
    return I;
}

float computeIControl(float IRef) {
  // Compute control
  float I = getCurrent(); 
  float Ie_k = IRef - I;
  float Iu_k = Iu_km1 + IKc * Ie_k - IKc * Iz * Ie_km1;

  // Saturation
  Iu_k = max(Iu_k, 0.0);
  Iu_k = min(Iu_k, DAC_INPUT_RANGE);
  // Output value update
  Vin = Iu_k;
  writeVin();
  // State variables update
  Iu_km1 = Iu_k;
  Ie_km1 = Ie_k;

  writeVin();

  return Vin;
}
void writeVin() {
  float x = Vin / DAC_BIT_VOLTAGE;
  //float VADC_bits = IRef_raw*4.00293255132;
  x = min(DAC_12BIT_MAX, x);
  //Vin = x * 5.0 / DAC_12BIT_MAX;
  Idac.setVoltage(x, false);
}
