float getCurrent() {
    int16_t intI = adc.readADC_SingleEnded(ADC_I_CH);
    float vI = intI * ADC_BIT_VOLTAGE;
    float I = vI * I_SENSE_GAIN;
    return I;
}
