#ifndef CONVERTER_DEFS_H
#define CONVERTER_DEFS_H
#include <Arduino.h>
#include <Adafruit_MCP4725.h>
#include <Adafruit_ADS1X15.h>

#define ADC_ADDR 0x48  // ADDR pin = GND

#define I_DAC_ADDR 0x60
#define T_DAC_ADDR 0x61

const int8_t ADC_I_CH = 1;
const int8_t ADC_T_CH = 2;
const int8_t ADC_ADDR_PIN = 7; // Note: this might not matter

const int16_t DAC_12BIT_MAX = 4095;
const int32_t ADC_16BIT_MAX = 65535;

const float DAC_INPUT_RANGE = 5.0;
const float DAC_BIT_VOLTAGE = DAC_INPUT_RANGE / (DAC_12BIT_MAX + 1);
//const float T_DAC_X0 = 1228.5;  // 1.5V on CTLI port
//const float I_DAC_X0 = DAC_12BIT_MAX/2;

const float ADC_INPUT_RANGE = 6.144;
const double ADC_BIT_VOLTAGE = 2*ADC_INPUT_RANGE / (ADC_16BIT_MAX + 1);  // V/bit

// I reading constant
const float I_SENSE_GAIN = 1000*((10.0/100.0)/1.8); // V -> mA

const float VCTLI_OFFSET = 3.0;
const float MAX_VCTLI = 2.0;
const float MIN_VCTLI = -2.0;

Adafruit_ADS1115 adc;

Adafruit_MCP4725 Tdac;
Adafruit_MCP4725 Idac;

void initConverters();

float getTemperature();
float getCurrent();

void writeOnDAC(Adafruit_MCP4725 dac, float vout);


#endif