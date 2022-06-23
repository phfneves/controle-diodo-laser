#include <TFT_HX8357.h> // Hardware-specific library
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <math.h>

class Diodo {
  private: String name;
  private: float maxCurrent, minCurrent, maxTemp, minTemp, current_control_param_P, current_control_param_I;

  public: Diodo(){}

  public: Diodo(String name, float maxCurrent, float minCurrent, float maxTemp, float minTemp, float current_control_param_P, float current_control_param_I){
    this->name = name;
    this->maxCurrent = maxCurrent;
    this->minCurrent = minCurrent;
    this->maxTemp = maxTemp;
    this->minTemp = minTemp;
    this->current_control_param_P = current_control_param_P;
    this->current_control_param_I = current_control_param_I;
  }

  public: String getName() {
    return this->name;
  }

  public: float getMaxCurrentValue() {
    return this->maxCurrent;
  }

  public: float getMinCurrentValue() {
    return this->minCurrent;
  }

  public: float getMaxTempValue() {
    return this->maxTemp;
  }

  public: float getMinTempValue() {
    return this->minTemp;
  }
  
  public: float getCurrentControlParamP() {
    return this->current_control_param_P;
  } 
  
  public: float getCurrentControlParamI() {
    return this->current_control_param_I;
  } 
};

struct Point {
  int16_t x0;
  int16_t y0;
};

ClickEncoder *encoder;

const int8_t CLK_PIN = 10;  // Pin 11 to clk on encoder
const int8_t DT_PIN = 9;  // Pin 13 to DT on encoder
const int8_t SW_PIN = 8;  // Pin 7 to SW on encoder

const int8_t NO_SELECTION_STATE = 0;
const int8_t DIODO_SELECTION_STATE = 1;
const int8_t CURRENT_COARSE_ADJUSTMENT_STATE = 2;
const int8_t CURRENT_FINE_ADJUSTMENT_STATE = 3;
const int8_t TEMP_COARSE_ADJUSTMENT_STATE = 4;
const int8_t TEMP_FINE_ADJUSTMENT_STATE = 5;
int8_t selected_state;

const int8_t FINE_CURRENT_DIGITS_RESOLUTION = 2;
const int8_t FINE_CURRENT_MAX_RESOLUTION_VALUE = 99;
const int8_t FINE_CURRENT_MIN_RESOLUTION_VALUE = 0;
const int8_t FINE_TEMP_DIGITS_RESOLUTION = 2;
const int8_t FINE_TEMP_MAX_RESOLUTION_VALUE = 99;
const int8_t FINE_TEMP_MIN_RESOLUTION_VALUE = 0;
Diodo diodoList[] = {
  Diodo("Diodo 1", 100.10, 10.10, 10.10, 1.01, 0.0, 0.0),
  Diodo("Diodo 2", 200.20, 20.20, 20.20, 2.02, 0.0, 0.0),
  Diodo("Diodo 3", 300.30, 30.30, 30.30, 3.03, 0.0, 0.0),
  Diodo("Diodo 4", 400.40, 40.40, 40.40, 4.04, 0.0, 0.0),
  Diodo("Diodo 5", 500.50, 50.50, 50.50, 5.05, 0.0, 0.0)
};
int8_t diodeListSize;
int8_t selected_diodo_index;
Diodo selected_diodo;
float current_value;
float temp_value;

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define SCREEN_MARGINS 10
#define SMALL_TEXT_FONT 1
#define MEDIUM_TEXT_FONT 2
#define LARGE_TEXT_FONT 4
#define SMALL_TEXT_FONT_HEIGHT 8
#define MEDIUM_TEXT_FONT_HEIGHT 16
#define LARGE_TEXT_FONT_HEIGHT 26
#define BACKGROUND_COLOR 0xFFFF
#define TEXT_COLOR 0x212121
#define SELECTED_COLOR 0x2196F3
#define SELECTED_TEXT_COLOR 0x212121

bool firstDraw = true;
bool reedrawDiode = true;
bool reedrawCurrent = true;
bool reedrawTemp = true;

TFT_HX8357 display = TFT_HX8357();       // Invoke custom library

void setup() {
  // put your setup code here, to run once:
  display.init();
  display.setRotation(1); //orientação na horizontal

  Serial.begin (9600);

  encoder = new ClickEncoder(CLK_PIN, DT_PIN, SW_PIN,4);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  
  setupVariables();
}

void setupVariables() {
  selected_state = NO_SELECTION_STATE;
  updateCurrentAndTempValuesFromSelectedDiodeIndex(0);
  diodeListSize = sizeof(diodoList)/sizeof(diodoList[0]);
}

void updateCurrentAndTempValuesFromSelectedDiodeIndex(int8_t index) {
  selected_diodo_index = index;
  selected_diodo = diodoList[selected_diodo_index];
  current_value = selected_diodo.getMaxCurrentValue()/2;
  temp_value = selected_diodo.getMaxTempValue()/2;
  reedrawDiode = true;
}

// int extractFractionalPart(float value, uint8_t digits) {
//   int8_t integerPart = (int8_t)(value);
//   return pow(0, digits) * (value - integerPart);
// }

void timerIsr() {
  encoder->service();
}

void loop() {
  setEncoderButtonClickedHandler();
  selectedValueChangeHandler();
  drawInterface();
}

void drawInterface() {
  if(firstDraw) {
    display.fillScreen(BACKGROUND_COLOR); //set background colour
    //display.drawLine(2*SCREEN_WIDTH/8, 0, 2*SCREEN_WIDTH/8, SCREEN_HEIGHT, TEXT_COLOR);
    //display.drawLine(5*SCREEN_WIDTH/8, 0, 5*SCREEN_WIDTH/8, SCREEN_HEIGHT, TEXT_COLOR); 
  }
  drawDiodeList(firstDraw, (firstDraw || reedrawDiode));
  drawCurrentInterface(firstDraw, (firstDraw || reedrawDiode || reedrawCurrent));
  drawTempInterface(firstDraw, (firstDraw || reedrawDiode || reedrawTemp));
  firstDraw = false;
  reedrawDiode = false;
}

void drawDiodeList(bool firstDraw, bool reedraw) {
  if (firstDraw) {
    drawTitle(25, SCREEN_MARGINS, "Diodo");
  }
  if(reedraw) {
    clearDiodeListBackground();
    for(int i =0; i<diodeListSize; i++) {
      drawDiodeNameByIndex(i);
      if (i == selected_diodo_index) {
        drawSelectedDiodeArrowByIndex(i);
        drawSelectedDiodeLineByIndex(i);
      }
    }
  }
}

void drawTitle(int16_t x0, int16_t y0, char *title) {
  display.setTextColor(TEXT_COLOR); 
  display.setTextFont(LARGE_TEXT_FONT);  
  display.setCursor(x0, y0);
  display.println(title);
}

void clearDiodeListBackground() {
  display.fillRect(0, (SCREEN_MARGINS + (LARGE_TEXT_FONT_HEIGHT*1.5)), 2*SCREEN_WIDTH/8, SCREEN_HEIGHT, BACKGROUND_COLOR);
}

void drawDiodeNameByIndex(int16_t index) {
  int16_t x0 = (SCREEN_MARGINS*1.5) + (MEDIUM_TEXT_FONT_HEIGHT/2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT*1.5 + (MEDIUM_TEXT_FONT_HEIGHT*index);
  display.setTextColor(TEXT_COLOR); display.setTextFont(MEDIUM_TEXT_FONT);
  display.setCursor(x0, y0);
  display.println(diodoList[index].getName());
}

void drawSelectedDiodeArrowByIndex(int16_t index) {
  int16_t x0 = SCREEN_MARGINS;
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT*1.5 + (MEDIUM_TEXT_FONT_HEIGHT*index);
  drawSelectedArrow(x0, y0);
}

void drawSelectedDiodeLineByIndex(int16_t index){
  int16_t x0 = (SCREEN_MARGINS*1.5) + (MEDIUM_TEXT_FONT_HEIGHT/2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT*1.5 + (MEDIUM_TEXT_FONT_HEIGHT*(index+1));
  int16_t x1 =  (2*SCREEN_WIDTH/8)- SCREEN_MARGINS;
  display.drawLine(x0, y0, x1, y0, TEXT_COLOR);
}

void drawSelectedArrow(int16_t x0, int16_t y0){
  int16_t x1 = x0 + (MEDIUM_TEXT_FONT_HEIGHT/2);
  int16_t y1 = y0 + (MEDIUM_TEXT_FONT_HEIGHT/2);
  int16_t y2 = y0 + MEDIUM_TEXT_FONT_HEIGHT;
  display.fillTriangle(x0, y0, x1, y1, x0, y2, SELECTED_COLOR);
}

void drawCurrentInterface(bool firstDraw, bool reedraw) {
  int16_t x0 = 3.5*SCREEN_WIDTH/8;
  int16_t r = (3*SCREEN_WIDTH/16) - (SCREEN_MARGINS*2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT*1.5 + r;
  if (firstDraw) {
    drawTitle((2*SCREEN_WIDTH/8) + 40, SCREEN_MARGINS, "Corrente");
    display.drawCircle(x0, y0, r, TEXT_COLOR);
    drawMinValueIndicator(x0, y0, r);
    drawMaxValueIndicator(x0, y0, r);
    drawMainValueIndicatorAtDeg(x0, y0, r, 155);
    drawMainValueIndicatorAtDeg(x0, y0, r, 90);
    drawMainValueIndicatorAtDeg(x0, y0, r, 25);
  }
  if(reedraw) {
    clearCenterValueTextBackground(x0, y0, r);
    // fillValueIndicatorsFromValue(x0, y0, r+1, selected_diodo.getMaxCurrentValue(), selected_diodo.getMinCurrentValue(), getCurrentValueComposed());
    drawCurrentValueText(x0, y0, r);
    reedrawCurrent = false;
  }
}

void drawTempInterface(bool firstDraw, bool reedraw) {
  int16_t x0 = 6.5*SCREEN_WIDTH/8;
  int16_t r = (3*SCREEN_WIDTH/16) - (SCREEN_MARGINS*2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT*1.5 + r;
  if (firstDraw) {
    drawTitle((5*SCREEN_WIDTH/8) + 17, SCREEN_MARGINS, "Temperatura");
    display.drawCircle(x0, y0, r, TEXT_COLOR);
    drawMinValueIndicator(x0, y0, r);
    drawMaxValueIndicator(x0, y0, r);
    drawMainValueIndicatorAtDeg(x0, y0, r, 155);
    drawMainValueIndicatorAtDeg(x0, y0, r, 90);
    drawMainValueIndicatorAtDeg(x0, y0, r, 25);
  }
  if (reedraw) {
    clearCenterValueTextBackground(x0, y0, r);
    // fillValueIndicatorsFromValue(x0, y0, r+1, selected_diodo.getMaxTempValue(), selected_diodo.getMinTempValue(), getTempValueComposed());
    drawTempValueText(x0, y0, r);
    reedrawTemp = false;
  }
}

void drawMinValueIndicator(int16_t x0, int16_t y0, int16_t r) {
  Point point = drawMainValueIndicatorAtDeg(x0, y0, r, 220);
  drawTextIndicator(point.x0, point.y0, "min");
}

struct Point drawMainValueIndicatorAtDeg(int16_t x0, int16_t y0, int16_t r, float deg) {
  int16_t r2 = r + (SCREEN_MARGINS*1.5);
  Point point = drawValueIndicator(x0, y0, r, r2, deg);
  return point;
}

struct Point drawValueIndicator(int16_t x0, int16_t y0, int16_t r, int16_t r2, float deg) {
  double rad = deg2rad(deg);
  int16_t sx0 = x0 + (cos(rad)*r);
  int16_t sy0 = y0 - (sin(rad)*r);
  int16_t sx1= x0 + (cos(rad)*r2);
  int16_t sy1= y0 - (sin(rad)*r2);
  display.drawLine(sx0, sy0, sx1, sy1, SELECTED_COLOR);
  Point point = {sx1, sy1};
  return point;
}

void drawTextIndicator(int16_t x0, int16_t y0, String text) {
  int16_t x1 = x0 - 8;
  int16_t y1 = y0 + (SMALL_TEXT_FONT_HEIGHT/2);
  display.setTextColor(TEXT_COLOR); display.setTextFont(SMALL_TEXT_FONT);
  display.setCursor(x1, y1);
  display.println(text);
}

void drawMaxValueIndicator(int16_t x0, int16_t y0, int16_t r) {
  Point point = drawMainValueIndicatorAtDeg(x0, y0, r, 320);
  drawTextIndicator(point.x0, point.y0, "max");
}

void drawCurrentValueText(int16_t x0, int16_t y0, int16_t r){
  char currentText[10];
  dtostrf(current_value, 5, FINE_CURRENT_DIGITS_RESOLUTION, currentText);
  sprintf(currentText, "%s mA", currentText);
  drawValueText(x0, y0, r, currentText);
}

void drawTempValueText(int16_t x0, int16_t y0, int16_t r){
  char tempText[9] = "";
  dtostrf(temp_value, 5, FINE_TEMP_DIGITS_RESOLUTION, tempText);
  sprintf(tempText, "%s C", tempText);
  drawValueText(x0, y0, r, tempText);
}

void drawValueText(int16_t x0, int16_t y0, int16_t r, char* text){
  int16_t x1 = x0 - r + SCREEN_MARGINS;
  int16_t y1 = y0 - (LARGE_TEXT_FONT_HEIGHT/2);
  display.setTextColor(TEXT_COLOR);
  display.drawCentreString(text, x0, y1, LARGE_TEXT_FONT);
}

void clearCenterValueTextBackground(int16_t x0, int16_t y0, int16_t r) {
  display.fillCircle(x0, y0, r-1, BACKGROUND_COLOR);
}

double deg2rad(float deg) {
  return (deg * 71)/ 4068;
}

void fillValueIndicatorsFromValue(int16_t x0, int16_t y0, int16_t r, float maxValue, float minValue, float value) {
  int16_t r2 = r + SCREEN_MARGINS;
  float valueGap = maxValue - minValue;
  float absoluteValue = value - minValue;
  float valuePercent = absoluteValue/valueGap;
  Serial.print("maxValue = ");
  Serial.println(maxValue);
  Serial.print("minValue = ");
  Serial.println(minValue);
  Serial.print("value = ");
  Serial.println(value);
  Serial.print("valueGap = ");
  Serial.println(valueGap);
  Serial.print("absoluteValue = ");
  Serial.println(absoluteValue);
  Serial.print("valuePercent = ");
  Serial.println(valuePercent);
  for (float i = 0; i < (valuePercent*260); i += 0.01) {
    float deg = 220 - i;
    if (deg < 0) {
      deg = 360 + deg;
    }
    drawValueIndicator(x0, y0, r, r2, deg);
  }
}

void setEncoderButtonClickedHandler() {
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    Serial.print("Button: ");
    if (b == ClickEncoder::Clicked) {
        encoderButtonClicked();
    }
  } 
}

void encoderButtonClicked() {
  selected_state++;
  if (selected_state > TEMP_FINE_ADJUSTMENT_STATE) {
    selected_state = NO_SELECTION_STATE;
  }
  executeSelectedStateActionBySelectedStateValue();
}

void executeSelectedStateActionBySelectedStateValue() {
  Serial.println("SELECTED_STATE: ");
  switch (selected_state) {
    case DIODO_SELECTION_STATE:
      Serial.println("DIODO_SELECTION_STATE");
      break;
    case CURRENT_COARSE_ADJUSTMENT_STATE:
      Serial.println("CURRENT_COARSE_ADJUSTMENT_STATE");
      break;
    case CURRENT_FINE_ADJUSTMENT_STATE:
      Serial.println("CURRENT_FINE_ADJUSTMENT_STATE");
      break;
    case TEMP_COARSE_ADJUSTMENT_STATE:
      Serial.println("TEMP_COARSE_ADJUSTMENT_STATE");
      break;
    case TEMP_FINE_ADJUSTMENT_STATE:
      Serial.println("TEMP_FINE_ADJUSTMENT_STATE");
      break;
    default:
      Serial.println("NO_SELECTION_STATE");
      break;
  }
}

void selectedValueChangeHandler() {
  if (selected_state != NO_SELECTION_STATE) {
    switch (selected_state) {
      case DIODO_SELECTION_STATE:
        diodeSelectorSelected();
        break;
      case CURRENT_COARSE_ADJUSTMENT_STATE:
        changeCoarseCurrentSelected();
        break;
      case CURRENT_FINE_ADJUSTMENT_STATE:
        changeFineCurrentSelected();
        break;
      case TEMP_COARSE_ADJUSTMENT_STATE:
        changeCoarseTempSelected();
        break;
      case TEMP_FINE_ADJUSTMENT_STATE:
        changeFineTempSelected();
        break;
      default:
        Serial.println("NO_SELECTION_STATE");
        break;
    }
  }
}

void diodeSelectorSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue != 0) {
    int8_t index = selected_diodo_index + increaseValue;
    if (index >= diodeListSize) {
      index = 0;
    } else if (index <= -1) {
      index = diodeListSize -1;
    }
    updateCurrentAndTempValuesFromSelectedDiodeIndex(index);
    Serial.print("Selected Diodo: ");
    Serial.println(selected_diodo.getName());
  }
}

void changeCoarseCurrentSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    increaseCurrentByValue(1);
  } else if (increaseValue < 0){
    increaseCurrentByValue(-1);
  }
}

void changeFineCurrentSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    increaseCurrentByValue(pow(10,-FINE_CURRENT_DIGITS_RESOLUTION));
  } else if (increaseValue < 0){
    increaseCurrentByValue(-pow(10,-FINE_CURRENT_DIGITS_RESOLUTION));
  }
}

void increaseCurrentByValue(float increaseValue) {
  Serial.print("Increase Value: ");
  Serial.println(increaseValue);
  float value = current_value + increaseValue;
  if (value >= selected_diodo.getMaxCurrentValue()) {
    value = selected_diodo.getMaxCurrentValue();
  } else if (value <= selected_diodo.getMinCurrentValue()) {
    value = selected_diodo.getMinCurrentValue();
  }
  current_value = value;
  reedrawCurrent = true;
  Serial.print("Current Value: ");
  Serial.println(current_value);
}

void changeCoarseTempSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    increaseTempByValue(1);
  } else if (increaseValue < 0){
    increaseTempByValue(-1);
  }
}

void changeFineTempSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    increaseTempByValue(pow(10,-FINE_TEMP_DIGITS_RESOLUTION));
  } else if (increaseValue < 0){
    increaseTempByValue(-pow(10,-FINE_TEMP_DIGITS_RESOLUTION));
  }
}

void increaseTempByValue(float increaseValue) {
  Serial.print("Increase Value: ");
  Serial.println(increaseValue);
  float value = temp_value + increaseValue;
  if (value >= selected_diodo.getMaxTempValue()) {
    value = selected_diodo.getMaxTempValue();
  } else if (value <= selected_diodo.getMinTempValue()) {
    value = selected_diodo.getMinTempValue();
  }
  temp_value = value;
  reedrawTemp = true;
  Serial.print("Temp Value: ");
  Serial.println(temp_value);
}
