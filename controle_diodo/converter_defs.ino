
void writeOnDAC(Adafruit_MCP4725 dac, float vout)
{
  int dac_bits = min(int((vout/DAC_INPUT_RANGE)*DAC_12BIT_MAX), DAC_12BIT_MAX); 
  dac.setVoltage(dac_bits,false);
}

void initConverters() {
    // A/D converter initialization
    pinMode(ADC_ADDR_PIN, OUTPUT);
    digitalWrite(ADC_ADDR_PIN, LOW);
    adc.setGain(GAIN_TWOTHIRDS);  // +/- 6.144V -> 1 bit = 0.1875mV
    adc.begin(ADC_ADDR);

    // D/A converters address initialization
    Tdac.begin(T_DAC_ADDR);
    Idac.begin(I_DAC_ADDR); 

    // D/A converters output initialization
    Tdac.setVoltage(T_DAC_X0, false);
    Idac.setVoltage(I_DAC_X0, false);
}
