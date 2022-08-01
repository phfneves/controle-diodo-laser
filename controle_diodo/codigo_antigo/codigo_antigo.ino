#include <LiquidCrystal.h>
#include <Wire.h>
#include "converter_defs.h"

char
firstLine[16],
          secondLine[16],
          IRef_str[6],
          I_str[6];

// LCD display
LiquidCrystal lcd(31, 37, 29, 27, 25, 23);

// Loop delay (ms)
const int LOOP_DELAY = 500;

const int COARSE_I_REF_ADDR = A15;

// Reference maximum readings
const float MAX_COARSE_T_REF = 36.0;
const float MAX_FINE_T_REF = 4.0;
//const float MAX_COARSE_I_REF = 5.0*1000*((68.0/820.0)/3.3);
const float MAX_COARSE_I_REF = 110.0;
const float MAX_FINE_I_REF = 10.0;

// T reading constants
const float TERMISTOR_NOMINAL_R = 10000.0;
const float NOMINAL_T = 25.0;
const float B_COEFFICIENT = 3460.0;
const float T_SERIES_RESISTOR = 10000.0;



// Interruption parameters
const uint16_t TIMER_RESET_VAL = 0;  // Timer reset value
const uint16_t TIMER_COMP = 60000;  // Timer limit

// Temperature control parameters
const float TKc = 0.01;
const float Tz = 0.8;

// Current control parameters
const float IKc = 0.01;
const float Iz = 0.7;

// Temperature (C) and current (mA) values
float T, I;

float VADC;

float Vin, VCTLI;

// Temperature and current references
float IRef, TRef;

int I_raw, IRef_raw, Vin_raw;

float Vout;

// State variables
float Ie_km1, Iu_km1, Te_km1, Tu_km1;

void initConversors() {
  // A/D converter initialization
  pinMode(ADC_ADDR_PIN, OUTPUT);
  digitalWrite(ADC_ADDR_PIN, LOW);
  adc.setGain(GAIN_TWOTHIRDS);  // +/- 6.144V -> 1 bit = 0.1875mV
  //adc.setGain(GAIN_ONE);
  adc.begin(ADC_ADDR);

  // D/A converters address initialization
  T_DAC.begin(T_DAC_ADDR);
  I_DAC.begin(I_DAC_ADDR);

  // D/A converters output initialization
  T_DAC.setVoltage(T_DAC_X0, false);
  I_DAC.setVoltage(I_DAC_X0, false);
  T_sensor.begin();
}

float readTRef() {
  float coarseT = (analogRead(COARSE_T_REF_ADDR) * MAX_COARSE_T_REF) / 1023.0;
  float fineT = (analogRead(FINE_T_REF_ADDR) * MAX_FINE_T_REF) / 1023.0;

  TRef = coarseT + fineT;
}

float readIRef() {
  //float coarseI = (analogRead(COARSE_I_REF_ADDR) * MAX_COARSE_I_REF) / 1023.0;
  //float fineI = (analogRead(FINE_I_REF_ADDR) * MAX_FINE_I_REF) / 1023.0;
  IRef_raw = analogRead(COARSE_I_REF_ADDR);
  //IRef = (MAX_COARSE_I_REF/4);
  IRef = IRef_raw * MAX_COARSE_I_REF / 1023.0;
}

void readT() {
  int16_t intT = adc.readADC_SingleEnded(ADC_T_CH);
  //Serial.print("T: ");
  //Serial.println(intT);
  // Voltage to resistance conversion
  //float x = ((((ADC_16BIT_MAX + 1) / 2) - 1) / (intT - 1));
  //float x = (intT)/(((ADC_16BIT_MAX + 1) / 2) - 1);
  //x = T_SERIES_RESISTOR / x;
  T = intT;
  // Beta factor equations
  //x = x / TERMISTOR_NOMINAL_R;      // (R/Ro)
  //x = log(x);                       // ln(R/Ro)
  //x /= B_COEFFICIENT;               // 1/B * ln(R/Ro)
  //x += 1.0 / (NOMINAL_T + 273.15);  // + (1/To)
  //x = 1.0 / x;                      // Inversion

  //T = x - 273.15;  // Celsius value

}

float readI() {
  int intI = adc.readADC_SingleEnded(ADC_I_CH);
  I_raw = intI;
  //Serial.println(intI);
  //Serial.print("intI: ");
  //Serial.println(intI);
  float vI = intI * ADC_BIT_VOLTAGE;
  Vout = vI;
  //Serial.print("ADC_BIT_VOLTAGE: ");
  //Serial.println(ADC_BIT_VOLTAGE);
  I = vI * I_SENSE_GAIN;
  return I;
}

void writeOnDAC(Adafruit_MCP4725 dac, float vout)
{
  int dac_bits = min(int((vout / DAC_INPUT_RANGE) * DAC_12BIT_MAX), DAC_12BIT_MAX);
  dac.setVoltage(dac_bits, false);
}

void updateInterface() {
  dtostrf(IRef, 4, 2, IRef_str );
  sprintf(firstLine, "IRef: %s mA ", IRef_str);
  lcd.setCursor(0, 0);
  lcd.print(firstLine);

  dtostrf(I, 4, 2, I_str );
  sprintf(secondLine, "I:    %s mA ", I_str);
  lcd.setCursor(0, 1);
  lcd.print(secondLine);
}

void initRefsAndStateVariables() {
  readTRef();
  readIRef();

  readT();
  readI();
  //
  //    Te_km1 = T - TRef;
  //    Tu_km1 = (T_DAC_X0 * DAC_BIT_VOLTAGE) - VCTLI_OFFSET;
  //
  //    Ie_km1 = IRef - I;
  //    Iu_km1 = I_DAC_X0 * DAC_BIT_VOLTAGE;
}

void initInterrupt() {
  //set timer4 interrupt at 1000Hz
  TCCR4A = 0;// set entire TCCR1A register to 0
  TCCR4B = 0;// same for TCCR1B
  TCNT4  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR4A = 15624 / 1; // = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR4B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR4B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK4 |= (1 << OCIE4A);

  // Configuração do TIMER1
  TCCR1A = 0;                //confira timer para operação normal
  TCCR1B = 0;                //limpa registrador
  TCNT1  = 0;                //zera temporizado

  OCR1A = 0x3D09;            // carrega registrador de comparação: 16MHz/1024/1Hz = 15625 = 0X3D09
  TCCR1B |= (1 << WGM12) | (1 << CS10) | (1 << CS12); // modo CTC, prescaler de 1024: CS12 = 1 e CS10 = 1
  TIMSK1 |= (1 << OCIE1A);  // habilita interrupção por igualdade de comparação

  sei();//allow interrupts
}

void computeIControl() {
  // Compute control
  float Ie_k = IRef - I;
  float Iu_k = Iu_km1 + IKc * Ie_k - IKc * Iz * Ie_km1;

  // Saturation
  Iu_k = max(Iu_k, 0.0);
  Iu_k = min(Iu_k, DAC_INPUT_RANGE);

  // Output value update
  Vin = Iu_k;

  // State variables update
  Iu_km1 = Iu_k;
  Ie_km1 = Ie_k;
}

void computeTControl() {
  // Compute control
  float Te_k = T - TRef;
  float Tu_k = Tu_km1 + TKc * Te_k - TKc * Tz * Te_km1;

  // Offset and saturation
  Tu_k = max(Tu_k, MIN_VCTLI);
  Tu_k = min(Tu_k, MAX_VCTLI);

  // Output value update
  VCTLI = Tu_k;

  // State variables update
  Tu_km1 = Tu_k;
  Te_km1 = Te_k;
}

void writeVin() {
  float x = Vin / DAC_BIT_VOLTAGE;
  //float VADC_bits = IRef_raw*4.00293255132;
  x = min(DAC_12BIT_MAX, x);
  Vin_raw = x;
  //Vin = x * 5.0 / DAC_12BIT_MAX;
  I_DAC.setVoltage(x, false);
}

void writeCTLI() {
  float x = (VCTLI + VCTLI_OFFSET) / DAC_BIT_VOLTAGE;
  x = min(DAC_12BIT_MAX, x);
  T_DAC.setVoltage(x, false);
}

void serialPrint() {
  Serial.print("Iref_raw: ");
  Serial.print(IRef_raw);
  Serial.print(", I_raw: ");
  Serial.print(I_raw);
  Serial.print(", Vin_raw: ");
  //Serial.print(TRef);
  //Serial.print(" C, T: ");
  //Serial.print(T);
  //Serial.print("C, Vin: ");
  Serial.print(Vin_raw);
  Serial.print(", Vin: ");
  //Serial.print("V, VCTLI: ");
  //Serial.print(VCTLI);
  Serial.print(Vin);
  Serial.print(" V, Vout: ");
  Serial.println(Vout);
  delay(60);
}

// -------------------- EXECUTION --------------------
void setup() {


  initConversors();
  initRefsAndStateVariables();

  lcd.begin(16, 2);
  updateInterface();

  Serial.begin(9600);
  while (!Serial);
  initInterrupt();
}

void loop() {
  unsigned long T0 = micros();
  //readTRef();
  //readIRef();
  IRef = 120.0;
  VADC = readI();
  //readT();
  computeIControl();
  writeVin();
  // T control
  //computeTControl();
  //writeCTLI();
  //Serial.println(micros()- T0);
}

//ISR(TIMER1_COMPA_vect) {
//    // Timer 1 reset
//    TCNT1 = TIMER_RESET_VAL;
//
//}

ISR(TIMER4_COMPA_vect) {

  updateInterface();
  serialPrint();
}
