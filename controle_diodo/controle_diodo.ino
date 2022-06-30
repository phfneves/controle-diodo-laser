#include <TFT_HX8357.h>  // Hardware-specific library
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <math.h>

class Diodo {
private:
  String name;

private:
  float maxCurrent, minCurrent, maxTemp, minTemp, current_control_param_P, current_control_param_I, currentValue, tempValue;

public:
  Diodo() {}

public:
  Diodo(String name, float maxCurrent, float minCurrent, float maxTemp, float minTemp, float current_control_param_P, float current_control_param_I, float currentValue, float tempValue) {
    this->name = name;
    this->maxCurrent = maxCurrent;
    this->minCurrent = minCurrent;
    this->maxTemp = maxTemp;
    this->minTemp = minTemp;
    this->current_control_param_P = current_control_param_P;
    this->current_control_param_I = current_control_param_I;
    this->currentValue = currentValue;
    this->tempValue = tempValue;
  }

public:
  String getName() {
    return this->name;
  }

public:
  float getMaxCurrentValue() {
    return this->maxCurrent;
  }

public:
  float getMinCurrentValue() {
    return this->minCurrent;
  }

public:
  float getMaxTempValue() {
    return this->maxTemp;
  }

public:
  float getMinTempValue() {
    return this->minTemp;
  }

public:
  float getCurrentControlParamP() {
    return this->current_control_param_P;
  }

public:
  float getCurrentControlParamI() {
    return this->current_control_param_I;
  }

public:
  float getCurrentValue() {
    return this->currentValue;
  }

public:
  float getTempValue() {
    return this->tempValue;
  }

public:
  float setCurrentValue(float currentValue) {
    this->currentValue = currentValue;
  }

public:
  float setTempValue(float tempValue) {
    this->tempValue = tempValue;
  }
};

ClickEncoder *encoder;

const int8_t CLK_PIN = 10;  // Pin 11 to clk on encoder
const int8_t DT_PIN = 9;    // Pin 13 to DT on encoder
const int8_t SW_PIN = 8;    // Pin 7 to SW on encoder

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
  Diodo("Diodo 1", 100.10, 10.10, 10.10, 1.01, 0.0, 0.0, 55.10, 5.05),
  Diodo("Diodo 2", 200.20, 20.20, 20.20, 2.02, 0.0, 0.0, 110.20, 11.11),
  Diodo("Diodo 3", 300.30, 30.30, 30.30, 3.03, 0.0, 0.0, 165.30, 16.66),
  Diodo("Diodo 4", 400.40, 40.40, 40.40, 4.04, 0.0, 0.0, 220.40, 22.22),
  Diodo("Diodo 5", 500.50, 50.50, 50.50, 5.05, 0.0, 0.0, 250.50, 25.27)
};
int8_t diodeListSize;
int8_t selected_diodo_index;
Diodo selected_diodo;

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

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

#define HX8357_GREY 0x2104  // Dark grey 16 bit colour

bool firstDraw = true;
bool reedrawDiode = true;
bool reedrawCurrent = true;
bool reedrawTemp = true;

TFT_HX8357 display = TFT_HX8357();  // Invoke custom library

void setup() {
  // put your setup code here, to run once:
  display.init();
  display.setRotation(1);  //orientação na horizontal

  Serial.begin(9600);

  encoder = new ClickEncoder(CLK_PIN, DT_PIN, SW_PIN, 4);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  setupVariables();
}

void setupVariables() {
  selected_state = NO_SELECTION_STATE;
  updateCurrentAndTempValuesFromSelectedDiodeIndex(0);
  diodeListSize = sizeof(diodoList) / sizeof(diodoList[0]);
}

void updateCurrentAndTempValuesFromSelectedDiodeIndex(int8_t index) {
  selected_diodo_index = index;
  selected_diodo = diodoList[selected_diodo_index];
  reedrawDiode = true;
}

void timerIsr() {
  encoder->service();
}

void loop() {
  setEncoderButtonClickedHandler();
  selectedValueChangeHandler();
  drawInterface();
}

void drawInterface() {
  if (firstDraw) {
    display.fillScreen(BACKGROUND_COLOR);  //set background colour
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
  if (reedraw) {
    clearDiodeListBackground();
    for (int i = 0; i < diodeListSize; i++) {
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
  display.fillRect(0, (SCREEN_MARGINS + (LARGE_TEXT_FONT_HEIGHT * 1.5)), 2 * SCREEN_WIDTH / 8, SCREEN_HEIGHT, BACKGROUND_COLOR);
}

void drawDiodeNameByIndex(int16_t index) {
  int16_t x0 = (SCREEN_MARGINS * 1.5) + (MEDIUM_TEXT_FONT_HEIGHT / 2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT * 1.5 + (MEDIUM_TEXT_FONT_HEIGHT * index);
  display.setTextColor(TEXT_COLOR);
  display.setTextFont(MEDIUM_TEXT_FONT);
  display.setCursor(x0, y0);
  display.println(diodoList[index].getName());
}

void drawSelectedDiodeArrowByIndex(int16_t index) {
  int16_t x0 = SCREEN_MARGINS;
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT * 1.5 + (MEDIUM_TEXT_FONT_HEIGHT * index);
  drawSelectedArrow(x0, y0);
}

void drawSelectedDiodeLineByIndex(int16_t index) {
  int16_t x0 = (SCREEN_MARGINS * 1.5) + (MEDIUM_TEXT_FONT_HEIGHT / 2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT * 1.5 + (MEDIUM_TEXT_FONT_HEIGHT * (index + 1));
  int16_t x1 = (2 * SCREEN_WIDTH / 8) - SCREEN_MARGINS;
  display.drawLine(x0, y0, x1, y0, TEXT_COLOR);
}

void drawSelectedArrow(int16_t x0, int16_t y0) {
  int16_t x1 = x0 + (MEDIUM_TEXT_FONT_HEIGHT / 2);
  int16_t y1 = y0 + (MEDIUM_TEXT_FONT_HEIGHT / 2);
  int16_t y2 = y0 + MEDIUM_TEXT_FONT_HEIGHT;
  display.fillTriangle(x0, y0, x1, y1, x0, y2, SELECTED_COLOR);
}

void drawCurrentInterface(bool firstDraw, bool reedraw) {
  int16_t x0 = 3.5 * SCREEN_WIDTH / 8;
  int16_t r = (3 * SCREEN_WIDTH / 16) - (SCREEN_MARGINS * 2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT * 1.5 + r;
  if (firstDraw) {
    drawTitle((2 * SCREEN_WIDTH / 8) + 40, SCREEN_MARGINS, "Corrente");
    display.drawCircle(x0, y0, r, TEXT_COLOR);
  }
  if (reedraw) {
    int16_t r1 = r + SCREEN_MARGINS;
    ringMeter(selected_diodo.getCurrentValue(), selected_diodo.getMinCurrentValue(), selected_diodo.getMaxCurrentValue(), x0-r1, y0-r1, r1, " mA", GREEN2RED, FINE_CURRENT_DIGITS_RESOLUTION);
    reedrawCurrent = false;
  }
}

void drawTempInterface(bool firstDraw, bool reedraw) {
  int16_t x0 = 6.5 * SCREEN_WIDTH / 8;
  int16_t r = (3 * SCREEN_WIDTH / 16) - (SCREEN_MARGINS * 2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT * 1.5 + r;
  if (firstDraw) {
    drawTitle((5 * SCREEN_WIDTH / 8) + 17, SCREEN_MARGINS, "Temperatura");
    display.drawCircle(x0, y0, r, TEXT_COLOR);
  }
  if (reedraw) {
    int16_t r1 = r + SCREEN_MARGINS;
    ringMeter(selected_diodo.getTempValue(), selected_diodo.getMinTempValue(), selected_diodo.getMaxTempValue(), x0-r1, y0-r1, r1, "C", GREEN2RED, FINE_TEMP_DIGITS_RESOLUTION);
    reedrawTemp = false;
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
      index = diodeListSize - 1;
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
  } else if (increaseValue < 0) {
    increaseCurrentByValue(-1);
  }
}

void changeFineCurrentSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    increaseCurrentByValue(pow(10, -FINE_CURRENT_DIGITS_RESOLUTION));
  } else if (increaseValue < 0) {
    increaseCurrentByValue(-pow(10, -FINE_CURRENT_DIGITS_RESOLUTION));
  }
}

void increaseCurrentByValue(float increaseValue) {
  Serial.print("Increase Value: ");
  Serial.println(increaseValue);
  float value = selected_diodo.getCurrentValue() + increaseValue;
  if (value >= selected_diodo.getMaxCurrentValue()) {
    value = selected_diodo.getMaxCurrentValue();
  } else if (value <= selected_diodo.getMinCurrentValue()) {
    value = selected_diodo.getMinCurrentValue();
  }
  selected_diodo.setCurrentValue(value);
  reedrawCurrent = true;
  Serial.print("Current Value: ");
  Serial.println(selected_diodo.getCurrentValue());
}

void changeCoarseTempSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    increaseTempByValue(1);
  } else if (increaseValue < 0) {
    increaseTempByValue(-1);
  }
}

void changeFineTempSelected() {
  int16_t increaseValue = encoder->getValue();
  if (increaseValue > 0) {
    increaseTempByValue(pow(10, -FINE_TEMP_DIGITS_RESOLUTION));
  } else if (increaseValue < 0) {
    increaseTempByValue(-pow(10, -FINE_TEMP_DIGITS_RESOLUTION));
  }
}

void increaseTempByValue(float increaseValue) {
  Serial.print("Increase Value: ");
  Serial.println(increaseValue);
  float value = selected_diodo.getTempValue() + increaseValue;
  if (value >= selected_diodo.getMaxTempValue()) {
    value = selected_diodo.getMaxTempValue();
  } else if (value <= selected_diodo.getMinTempValue()) {
    value = selected_diodo.getMinTempValue();
  }
  selected_diodo.setTempValue(value);
  reedrawTemp = true;
  Serial.print("Temp Value: ");
  Serial.println(selected_diodo.getTempValue());
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(float value, float vmin, float vmax, int x, int y, int r, char *units, byte scheme, int8_t resolution) {
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option

  x += r; y += r;   // Calculate coords of centre of ring

  int w = SCREEN_MARGINS;    // Width of outer ring is 1/4 of radius
  // int w = r / 5;    // Width of outer ring is 1/4 of radius

  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 3; // Segments are 3 degrees wide = 100 segments for 300 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Variable to save "value" text colour from scheme and set default
  int colour = HX8357_BLUE;

  // Draw colour blocks every inc degrees
  for (int i = -angle+inc/2; i < angle-inc/2; i += inc) {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      switch (scheme) {
        case 0: colour = HX8357_RED; break; // Fixed colour
        case 1: colour = HX8357_GREEN; break; // Fixed colour
        case 2: colour = HX8357_BLUE; break; // Fixed colour
        case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
        case 4: colour = rainbow(map(i, -angle, angle, 70, 127)); break; // Green to red (high temperature etc)
        case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
        default: colour = HX8357_BLUE; break; // Fixed colour
      }
      display.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      display.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      //text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      display.fillTriangle(x0, y0, x1, y1, x2, y2, HX8357_GREY);
      display.fillTriangle(x1, y1, x2, y2, x3, y3, HX8357_GREY);
    }
  }
  
  display.fillCircle(x, y, r - SCREEN_MARGINS - 1, BACKGROUND_COLOR);

  // Convert value to a string
  char buf[10];
  dtostrf(value, 5, resolution, buf);
  // Set the text colour to default
  display.setTextSize(1);

  display.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  // Uncomment next line to set the text colour to the last segment value!
  // display.setTextColor(colour, BACKGROUND_COLOR);
  display.setTextDatum(MC_DATUM);
  // Print value, if the meter is large then use big font 8, othewise use 4
  // display.drawCentreString(buf, x, y, LARGE_TEXT_FONT);
  display.setTextPadding(3 * 14); // Allow for 3 digits each 14 pixels wide
  display.drawCentreString(buf, x, y-(LARGE_TEXT_FONT_HEIGHT/2), LARGE_TEXT_FONT); // Value in middle

  display.setTextSize(1);
  display.setTextPadding(0);
  // Print units, if the meter is large then use big font 4, othewise use 2
  display.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  display.drawCentreString(units, x, y+(LARGE_TEXT_FONT_HEIGHT/2), MEDIUM_TEXT_FONT); // Units display
  // Calculate and return right hand side x coordinate
  return x + r;
}


// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

