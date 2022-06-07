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

ClickEncoder *encoder;

const int8_t CLK_PIN = 11;  // Pin 11 to clk on encoder
const int8_t DT_PIN = 13;  // Pin 13 to DT on encoder
const int8_t SW_PIN = 7;  // Pin 7 to SW on encoder

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
  Diodo("Diodo 4", 300.30, 30.30, 30.30, 3.03, 0.0, 0.0),
  Diodo("Diodo 5", 300.30, 30.30, 30.30, 3.03, 0.0, 0.0)
};
int8_t diodeListSize;
int8_t selected_diodo_index;
Diodo selected_diodo;
int8_t coarse_current_value;
int8_t fine_current_value;
int8_t coarse_temp_value;
int8_t fine_temp_value;

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


TFT_HX8357 display = TFT_HX8357();       // Invoke custom library

void setup() {
  // put your setup code here, to run once:
  display.init();
  display.setRotation(1); //orientação na horizontal

  Serial.begin (9600);

  encoder = new ClickEncoder(CLK_PIN, DT_PIN, SW_PIN,4);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  //encoder->setAccelerationEnabled(true);
  
  setupVariables();
}

void setupVariables() {
  selected_state = NO_SELECTION_STATE;
  selected_diodo_index = 0;
  selected_diodo = diodoList[selected_diodo_index];
  diodeListSize = sizeof(diodoList)/sizeof(diodoList[0]);
  
  float currentValue = selected_diodo.getMaxCurrentValue()/2;
  coarse_current_value = (int8_t) currentValue;
  fine_current_value = extractFractionalPart(currentValue, FINE_CURRENT_DIGITS_RESOLUTION);
  
  float tempValue = selected_diodo.getMaxTempValue()/2;
  coarse_temp_value = (int8_t) tempValue;
  fine_temp_value = extractFractionalPart(tempValue, FINE_TEMP_DIGITS_RESOLUTION);
}

int extractFractionalPart(float value, uint8_t digits) {
  int8_t integerPart = (int8_t)(value);
  return pow(0, digits) * (value - integerPart);
}

void timerIsr() {
  encoder->service();
}

void loop() {
  setEncoderButtonClickedHandler();
  selectedValueChangeHandler();
  drawInterface();
}

bool reedraw = true;

void drawInterface() {
  if(reedraw) {
    display.fillScreen(BACKGROUND_COLOR); //set background colour
    display.drawLine(2*SCREEN_WIDTH/8, 0, 2*SCREEN_WIDTH/8, SCREEN_HEIGHT, TEXT_COLOR);
    display.drawLine(5*SCREEN_WIDTH/8, 0, 5*SCREEN_WIDTH/8, SCREEN_HEIGHT, TEXT_COLOR);
    
    drawDiodeList();
    drawTitle((2*SCREEN_WIDTH/8) + 40, SCREEN_MARGINS, "Corrente");
    drawTitle((5*SCREEN_WIDTH/8) + 17, SCREEN_MARGINS, "Temperatura");
    int line = 0;

    reedraw = false;
  }
}

void drawDiodeList() {
  drawTitle(25, SCREEN_MARGINS, "Diodo");
  for(int i =0; i<diodeListSize; i++) {
    drawDiodeNameByIndex(i);
    if (i == selected_diodo_index)  {
      drawSelectedDiodeArrowByIndex(i);
      drawSelectedDiodeLineByIndex(i);
    }
  }
}

void drawTitle(int16_t x0, int16_t y0, char *title) {
  display.setTextColor(TEXT_COLOR); display.setTextFont(LARGE_TEXT_FONT);  
  display.setCursor(x0, y0);
  display.println(title);
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
  if (increaseValue > 0) {
    selected_diodo_index += increaseValue;
    if (selected_diodo_index >= diodeListSize) {
      selected_diodo_index = 0;
    }
    selected_diodo = diodoList[selected_diodo_index];
  } else if (increaseValue < 0) {
    selected_diodo_index += increaseValue;
    if (selected_diodo_index <= -1) {
      selected_diodo_index = diodeListSize -1;
    }
    selected_diodo = diodoList[selected_diodo_index];
  }
  Serial.print("Selected Diodo: ");
  Serial.println(selected_diodo.getName());
}

void changeCoarseCurrentSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    if (!isCurrentGreaterOrEqualToMaxValue(increaseValue)) {
        increaseCoarseCurrentByValue(increaseValue);
        Serial.print("Coarse Current Value: ");
        Serial.println(coarse_current_value);
    }
  } else if (increaseValue < 0) {
    if (!isCurrentSmallerOrEqualToMinValue(increaseValue)) {
        increaseCoarseCurrentByValue(increaseValue);
        Serial.print("Coarse Current Value: ");
        Serial.println(coarse_current_value);
    }
  }
}

bool isCurrentGreaterOrEqualToMaxValue(int16_t increaseValue) {
  return (getCurrentValueComposed() + increaseValue) >= selected_diodo.getMaxCurrentValue();
}

bool isCurrentSmallerOrEqualToMinValue(int16_t decreaseValue) {
  return (getCurrentValueComposed() + decreaseValue) <= selected_diodo.getMinCurrentValue();
}

float getCurrentValueComposed() {
  return composeTwoIntInFloat(coarse_current_value, fine_current_value, FINE_CURRENT_DIGITS_RESOLUTION);
}

float composeTwoIntInFloat(int decimal, int fractional, uint8_t digits) {
  return decimal + (fractional/digits);
}

void increaseCoarseCurrentByValue(int16_t value) {
  coarse_current_value += value;
}

void changeFineCurrentSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    if (!isCurrentGreaterOrEqualToMaxValue(increaseValue/FINE_CURRENT_DIGITS_RESOLUTION)) {
        increaseFineCurrentByValue(increaseValue);
        Serial.print("Fine Current Value: ");
        Serial.println(fine_current_value);
    }
  } else if (increaseValue < 0) {
    if (!isCurrentSmallerOrEqualToMinValue(increaseValue/FINE_CURRENT_DIGITS_RESOLUTION)) {
        increaseFineCurrentByValue(increaseValue);
        Serial.print("Fine Current Value: ");
        Serial.println(fine_current_value);
    }
  }
}

void increaseFineCurrentByValue(int16_t value) {
  if (fine_current_value >= FINE_CURRENT_MAX_RESOLUTION_VALUE) {
    fine_current_value = 0;
    increaseCoarseCurrentByValue(1);
  } else if (fine_current_value <= FINE_CURRENT_MIN_RESOLUTION_VALUE) {
    fine_current_value = FINE_CURRENT_MAX_RESOLUTION_VALUE;
    increaseCoarseCurrentByValue(-1);
  } else {
    fine_current_value += value;
  }
}

void changeCoarseTempSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    if (!isTempGreaterOrEqualToMaxValue(increaseValue)) {
        increaseCoarseTempByValue(increaseValue);
        Serial.print("Coarse Temp Value: ");
        Serial.println(coarse_temp_value);
    }
  } else if (increaseValue < 0) {
    if (!isTempSmallerOrEqualToMinValue(increaseValue)) {
        increaseCoarseTempByValue(increaseValue);
        Serial.print("Coarse Temp Value: ");
        Serial.println(coarse_current_value);
    }
  }
}

bool isTempGreaterOrEqualToMaxValue(int16_t increaseValue) {
  return (getTempValueComposed() + increaseValue) >= selected_diodo.getMaxTempValue();
}

bool isTempSmallerOrEqualToMinValue(int16_t decreaseValue) {
  return (getTempValueComposed() + decreaseValue) <= selected_diodo.getMinTempValue();
}

float getTempValueComposed() {
  return composeTwoIntInFloat(coarse_temp_value, fine_temp_value, FINE_TEMP_DIGITS_RESOLUTION);
}

void increaseCoarseTempByValue(int16_t value) {
  coarse_temp_value += value;
}

void changeFineTempSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    if (!isTempGreaterOrEqualToMaxValue(increaseValue/FINE_TEMP_DIGITS_RESOLUTION)) {
        increaseFineTempByValue(increaseValue);
        Serial.print("Fine Temp Value: ");
        Serial.println(fine_current_value);
    }
  } else if (increaseValue < 0) {
    if (!isTempSmallerOrEqualToMinValue(increaseValue/FINE_CURRENT_DIGITS_RESOLUTION)) {
        increaseFineTempByValue(increaseValue);
        Serial.print("Fine Temp Value: ");
        Serial.println(fine_current_value);
    }
  }
}

void increaseFineTempByValue(int16_t value) {
  if (fine_temp_value >= FINE_TEMP_MAX_RESOLUTION_VALUE) {
    fine_temp_value = 0;
    increaseCoarseTempByValue(1);
  } else if (fine_temp_value <= FINE_TEMP_MIN_RESOLUTION_VALUE) {
    fine_temp_value = FINE_TEMP_MAX_RESOLUTION_VALUE;
    increaseCoarseTempByValue(-1);
  } else {
    fine_temp_value += value;
  }
}
