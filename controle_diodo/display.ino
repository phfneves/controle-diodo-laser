#include <TFT_HX8357.h>  // Hardware-specific library
#include <ClickEncoder.h>
#include <math.h>
#include <TimerOne.h>

class Diodo {
  private:
    char* name;

  private:
    float maxCurrent, minCurrent, maxTemp, minTemp, currentValue, tempValue;

  public:
    Diodo() {}

  public:
    Diodo(char* name, float maxCurrent, float minCurrent, float maxTemp, float minTemp, float currentValue, float tempValue) {
      this->name = name;
      this->maxCurrent = maxCurrent;
      this->minCurrent = minCurrent;
      this->maxTemp = maxTemp;
      this->minTemp = minTemp;
      this->currentValue = currentValue;
      this->tempValue = tempValue;
    }

  public:
    char* getName() {
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
  Diodo("L785P090", 165.0, 0.0, 70.0, -10.0, 125.0, 23.0),
  Diodo("L780P010", 40.0, 0.0, 60.0, -10.0, 24.0, 23.0),
  Diodo("L840P200", 340.0, 0.0, 60.0, -10.0, 255.0, 23.0),
  Diodo("DL7140-201S", 140.0, 0.0, 60.0, -10.0, 100.0, 23.0)
};
int8_t diodeListSize;
int8_t selected_diodo_index;
bool currentOvershoot = false;
bool tempOvershoot = true;

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
bool blink = false;
int16_t blinkCycles = 50;
int16_t blinkCount = 0;

TFT_HX8357 display = TFT_HX8357();  // Invoke custom library

void setupDisplay() {
  display.init();
  display.setRotation(1);  //orientação na horizontal
  encoder = new ClickEncoder(CLK_PIN, DT_PIN, SW_PIN, 4);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  setupVariables();
}

void setupVariables() {
  selected_state = NO_SELECTION_STATE;
  selected_diodo_index = 0;
  reedrawDiode = true;
  diodeListSize = sizeof(diodoList) / sizeof(diodoList[0]);
}


void timerIsr() {
  encoder->service();
}

void loopDisplay() {
  setEncoderButtonClickedHandler();
  selectedValueChangeHandler();
  drawInterface();
  updateIndicators(&currentOvershoot, &tempOvershoot);
}


float readTRef() {
  return diodoList[selected_diodo_index].getTempValue();
}

float readIRef() {
  return diodoList[selected_diodo_index].getCurrentValue();
}

void drawInterface() {
  if (firstDraw) {
    display.fillScreen(BACKGROUND_COLOR);
  }
  drawDiodeList(firstDraw, (firstDraw || reedrawDiode));
  drawCurrentInterface(firstDraw, (firstDraw || reedrawDiode || reedrawCurrent));
  drawTempInterface(firstDraw, (firstDraw || reedrawDiode || reedrawTemp));
  firstDraw = false;
  reedrawDiode = false;
}

void drawDiodeList(bool firstDraw, bool reedraw) {
  if (firstDraw) {
    drawTitle(25, SCREEN_MARGINS, "Diode");
  }

  blinkSelectedDiode();

  if (reedraw) {
    clearDiodeListBackground();
    for (int i = 0; i < diodeListSize; i++) {
      drawDiodeNameByIndex(i);
      if (i == selected_diodo_index) {
        drawSelectedDiodeArrowByIndex(i, SELECTED_COLOR);
        drawSelectedDiodeLineByIndex(i, TEXT_COLOR);
      }
    }
  }
}

void blinkSelectedDiode() {
  if (selected_state == DIODO_SELECTION_STATE) {
    if (blink) {
      drawSelectedDiodeArrowByIndex(selected_diodo_index, BACKGROUND_COLOR);
      drawSelectedDiodeLineByIndex(selected_diodo_index, BACKGROUND_COLOR);
    } else {
      drawSelectedDiodeArrowByIndex(selected_diodo_index, SELECTED_COLOR);
      drawSelectedDiodeLineByIndex(selected_diodo_index, TEXT_COLOR);
    }
  }
}

void drawText(int16_t x0, int16_t y0, char *text, uint16_t color, uint8_t font) {
  display.setTextColor(color);
  display.setTextFont(font);
  display.setCursor(x0, y0);
  display.println(text);
}

void drawTitle(int16_t x0, int16_t y0, char *title) {
  drawText(x0, y0, title, TEXT_COLOR, LARGE_TEXT_FONT);
}

void clearDiodeListBackground() {
  display.fillRect(0, (SCREEN_MARGINS + (LARGE_TEXT_FONT_HEIGHT * 1.5)), 2 * SCREEN_WIDTH / 8, SCREEN_HEIGHT, BACKGROUND_COLOR);
}

void drawDiodeNameByIndex(int16_t index) {
  int16_t x0 = (SCREEN_MARGINS * 1.5) + (MEDIUM_TEXT_FONT_HEIGHT / 2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT * 1.5 + (MEDIUM_TEXT_FONT_HEIGHT * index);
  drawText(x0, y0, diodoList[index].getName(), TEXT_COLOR, MEDIUM_TEXT_FONT);
}

void drawSelectedDiodeArrowByIndex(int16_t index, uint16_t color) {
  int16_t x0 = SCREEN_MARGINS;
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT * 1.5 + (MEDIUM_TEXT_FONT_HEIGHT * index);
  drawSelectedArrow(x0, y0, color);
}

void drawSelectedDiodeLineByIndex(int16_t index, uint16_t color) {
  int16_t x0 = (SCREEN_MARGINS * 1.5) + (MEDIUM_TEXT_FONT_HEIGHT / 2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT * 1.5 + (MEDIUM_TEXT_FONT_HEIGHT * (index + 1));
  int16_t x1 = (2 * SCREEN_WIDTH / 8) - SCREEN_MARGINS;
  drawSelectedLine(x0, y0, x1, color);
}

void drawSelectedLine(int16_t x0, int16_t y0, int16_t x1, uint16_t color) {
  display.drawLine(x0, y0, x1, y0, color);
}

void drawSelectedArrow(int16_t x0, int16_t y0, uint16_t color) {
  int16_t x1 = x0 + (MEDIUM_TEXT_FONT_HEIGHT / 2);
  int16_t y1 = y0 + (MEDIUM_TEXT_FONT_HEIGHT / 2);
  int16_t y2 = y0 + MEDIUM_TEXT_FONT_HEIGHT;
  display.fillTriangle(x0, y0, x1, y1, x0, y2, color);
}

void drawCurrentInterface(bool firstDraw, bool reedraw) {
  int16_t x0 = 3.5 * SCREEN_WIDTH / 8;
  int16_t r = (3 * SCREEN_WIDTH / 16) - (SCREEN_MARGINS * 2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT * 1.5 + r;
  if (firstDraw) {
    drawTitle((2 * SCREEN_WIDTH / 8) + 40, SCREEN_MARGINS, "Current");
    display.drawCircle(x0, y0, r, TEXT_COLOR);
  }

  drawCurrentCoarseAndFineTextIndicator((x0 - r - SCREEN_MARGINS), (y0 + r), TEXT_COLOR);

  if (reedraw) {
    int16_t r1 = r + SCREEN_MARGINS;
    ringMeter(diodoList[selected_diodo_index].getCurrentValue(), diodoList[selected_diodo_index].getMinCurrentValue(), diodoList[selected_diodo_index].getMaxCurrentValue(), x0 - r1, y0 - r1, r1, " mA", GREEN2RED, FINE_CURRENT_DIGITS_RESOLUTION);
    reedrawCurrent = false;
  }

  drawCurrentOvershootIndicator(x0, (y0 + r + MEDIUM_TEXT_FONT_HEIGHT + (4 * SCREEN_MARGINS)));
}

void drawCurrentCoarseAndFineTextIndicator(int16_t x0, int16_t y0, uint16_t color) {
  int16_t x1 = x0 + (SCREEN_MARGINS * 1.5);
  int16_t y1 = y0 + SCREEN_MARGINS;

  drawText(x1, y1, "Coarse A.", TEXT_COLOR, MEDIUM_TEXT_FONT);

  int16_t x3 = x1 + 55 + (SCREEN_MARGINS * 2);
  int16_t x4 = x3 + (SCREEN_MARGINS * 1.5);
  drawText(x4, y1, "Fine A.", TEXT_COLOR, MEDIUM_TEXT_FONT);

  blinkSelectedCurrentCoarseAdjustment(x0, y1);
  blinkSelectedCurrentFineAdjustment(x3, y1);
}

void blinkSelectedCurrentCoarseAdjustment(int16_t x0, int16_t y0) {
  int16_t x1 = x0 + (SCREEN_MARGINS * 1.5);
  int16_t y1 = y0 + MEDIUM_TEXT_FONT_HEIGHT;
  int16_t x2 = x1 + 55;
  if (selected_state == CURRENT_COARSE_ADJUSTMENT_STATE) {
    if (blink) {
      drawSelectedArrow(x0, y0, BACKGROUND_COLOR);
      drawSelectedLine(x1, y1, x2, BACKGROUND_COLOR);
    } else {
      drawSelectedArrow(x0, y0, SELECTED_COLOR);
      drawSelectedLine(x1, y1, x2, TEXT_COLOR);
    }
  } else {
    drawSelectedArrow(x0, y0, BACKGROUND_COLOR);
    drawSelectedLine(x1, y1, x2, BACKGROUND_COLOR);
  }
}

void blinkSelectedCurrentFineAdjustment(int16_t x0, int16_t y0) {
  int16_t x1 = x0 + (SCREEN_MARGINS * 1.5);
  int16_t y1 = y0 + MEDIUM_TEXT_FONT_HEIGHT;
  int16_t x2 = x1 + 40;
  if (selected_state == CURRENT_FINE_ADJUSTMENT_STATE) {
    if (blink) {
      drawSelectedArrow(x0, y0, BACKGROUND_COLOR);
      drawSelectedLine(x1, y1, x2, BACKGROUND_COLOR);
    } else {
      drawSelectedArrow(x0, y0, SELECTED_COLOR);
      drawSelectedLine(x1, y1, x2, TEXT_COLOR);
    }
  } else {
    drawSelectedArrow(x0, y0, BACKGROUND_COLOR);
    drawSelectedLine(x1, y1, x2, BACKGROUND_COLOR);
  }
}

void drawCurrentOvershootIndicator(int16_t x0, int16_t y0) {
  display.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);

  display.drawCentreString("Running State:", x0, y0 - (MEDIUM_TEXT_FONT_HEIGHT / 2), MEDIUM_TEXT_FONT); // Value in middle

  uint16_t color = TFT_GREEN;
  char* text = "On Target";
  if (currentOvershoot) {
    color = TFT_RED;
    text = "Overshoot";
  }

  int16_t r = SCREEN_MARGINS * 1.5;
  int16_t y1 = y0 + MEDIUM_TEXT_FONT_HEIGHT + r;
  display.fillCircle(x0, y1, r, color);


  int16_t y2 = y1 + r + SCREEN_MARGINS;
  display.drawCentreString(text, x0, y2, MEDIUM_TEXT_FONT);
}

void drawTempCoarseAndFineTextIndicator(int16_t x0, int16_t y0, uint16_t color) {
  int16_t x1 = x0 + (SCREEN_MARGINS * 1.5);
  int16_t y1 = y0 + SCREEN_MARGINS;

  drawText(x1, y1, "Coarse A.", TEXT_COLOR, MEDIUM_TEXT_FONT);

  int16_t x3 = x1 + 55 + (SCREEN_MARGINS * 2);
  int16_t x4 = x3 + (SCREEN_MARGINS * 1.5);
  drawText(x4, y1, "Fine A.", TEXT_COLOR, MEDIUM_TEXT_FONT);

  blinkSelectedTempCoarseAdjustment(x0, y1);
  blinkSelectedTempFineAdjustment(x3, y1);
}

void blinkSelectedTempCoarseAdjustment(int16_t x0, int16_t y0) {
  int16_t x1 = x0 + (SCREEN_MARGINS * 1.5);
  int16_t y1 = y0 + MEDIUM_TEXT_FONT_HEIGHT;
  int16_t x2 = x1 + 55;
  if (selected_state == TEMP_COARSE_ADJUSTMENT_STATE) {
    if (blink) {
      drawSelectedArrow(x0, y0, BACKGROUND_COLOR);
      drawSelectedLine(x1, y1, x2, BACKGROUND_COLOR);
    } else {
      drawSelectedArrow(x0, y0, SELECTED_COLOR);
      drawSelectedLine(x1, y1, x2, TEXT_COLOR);
    }
  } else {
    drawSelectedArrow(x0, y0, BACKGROUND_COLOR);
    drawSelectedLine(x1, y1, x2, BACKGROUND_COLOR);
  }
}

void blinkSelectedTempFineAdjustment(int16_t x0, int16_t y0) {
  int16_t x1 = x0 + (SCREEN_MARGINS * 1.5);
  int16_t y1 = y0 + MEDIUM_TEXT_FONT_HEIGHT;
  int16_t x2 = x1 + 40;
  if (selected_state == TEMP_FINE_ADJUSTMENT_STATE) {
    if (blink) {
      drawSelectedArrow(x0, y0, BACKGROUND_COLOR);
      drawSelectedLine(x1, y1, x2, BACKGROUND_COLOR);
    } else {
      drawSelectedArrow(x0, y0, SELECTED_COLOR);
      drawSelectedLine(x1, y1, x2, TEXT_COLOR);
    }
  } else {
    drawSelectedArrow(x0, y0, BACKGROUND_COLOR);
    drawSelectedLine(x1, y1, x2, BACKGROUND_COLOR);
  }
}

void drawTempInterface(bool firstDraw, bool reedraw) {
  int16_t x0 = 6.5 * SCREEN_WIDTH / 8;
  int16_t r = (3 * SCREEN_WIDTH / 16) - (SCREEN_MARGINS * 2);
  int16_t y0 = SCREEN_MARGINS + LARGE_TEXT_FONT_HEIGHT * 1.5 + r;
  if (firstDraw) {
    drawTitle((5 * SCREEN_WIDTH / 8) + 17, SCREEN_MARGINS, "Temperature");
    display.drawCircle(x0, y0, r, TEXT_COLOR);
  }

  drawTempCoarseAndFineTextIndicator((x0 - r - SCREEN_MARGINS), (y0 + r), TEXT_COLOR);

  if (reedraw) {
    int16_t r1 = r + SCREEN_MARGINS;
    ringMeter(diodoList[selected_diodo_index].getTempValue(), diodoList[selected_diodo_index].getMinTempValue(), diodoList[selected_diodo_index].getMaxTempValue(), x0 - r1, y0 - r1, r1, "C", GREEN2RED, FINE_TEMP_DIGITS_RESOLUTION);
    reedrawTemp = false;
  }

  drawTempOvershootIndicator(x0, (y0 + r + MEDIUM_TEXT_FONT_HEIGHT + (4 * SCREEN_MARGINS)));
}

void drawTempOvershootIndicator(int16_t x0, int16_t y0) {
  display.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);

  display.drawCentreString("Running State:", x0, y0 - (MEDIUM_TEXT_FONT_HEIGHT / 2), MEDIUM_TEXT_FONT); // Value in middle

  uint16_t color = TFT_GREEN;
  char* text = "On Target";
  if (tempOvershoot) {
    color = TFT_RED;
    text = "Overshoot";
  }

  int16_t r = SCREEN_MARGINS * 1.5;
  int16_t y1 = y0 + MEDIUM_TEXT_FONT_HEIGHT + r;
  display.fillCircle(x0, y1, r, color);


  int16_t y2 = y1 + r + SCREEN_MARGINS;
  display.drawCentreString(text, x0, y2, MEDIUM_TEXT_FONT);
}

void setEncoderButtonClickedHandler() {
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    //Serial.print("Button: ");
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
  blinkCount = 0;
  blink = false;
  reedrawDiode = true;
  reedrawCurrent = true;
  reedrawTemp = true;
}

void selectedValueChangeHandler() {
  if (selected_state != NO_SELECTION_STATE) {
    blinkCount++;
    if (blinkCount >= (blinkCycles * 2)) {
      blinkCount = 0;
      blink = false;
    } else if (blinkCount >= blinkCycles) {
      blink = true;
    }

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
        //Serial.println("NO_SELECTION_STATE");
        break;
    }
  }
}

void diodeSelectorSelected() {
  int16_t increaseValue = -encoder->getValue();
  if (increaseValue != 0) {
    int8_t index = selected_diodo_index + increaseValue;
    if (index >= diodeListSize) {
      index = 0;
    } else if (index <= -1) {
      index = diodeListSize - 1;
    }
    selected_diodo_index = index;
    reedrawDiode = true;
    //Serial.print("Selected Diodo: ");
    //Serial.println(diodoList[selected_diodo_index].getName());
  }
}

void changeCoarseCurrentSelected() {
  int16_t increaseValue = -encoder->getValue();
  if (increaseValue > 0) {
    increaseCurrentByValue(1);
  } else if (increaseValue < 0) {
    increaseCurrentByValue(-1);
  }
}

void changeFineCurrentSelected() {
  int16_t increaseValue = -encoder->getValue();
  if (increaseValue > 0) {
    increaseCurrentByValue(pow(10, -FINE_CURRENT_DIGITS_RESOLUTION));
  } else if (increaseValue < 0) {
    increaseCurrentByValue(-pow(10, -FINE_CURRENT_DIGITS_RESOLUTION));
  }
}

void increaseCurrentByValue(float increaseValue) {
  //Serial.print("Increase Current Value: ");
  //Serial.println(increaseValue);
  float value = diodoList[selected_diodo_index].getCurrentValue() + increaseValue;
  if (value >= diodoList[selected_diodo_index].getMaxCurrentValue()) {
    value = diodoList[selected_diodo_index].getMaxCurrentValue();
  } else if (value <= diodoList[selected_diodo_index].getMinCurrentValue()) {
    value = diodoList[selected_diodo_index].getMinCurrentValue();
  }
  diodoList[selected_diodo_index].setCurrentValue(value);
  reedrawCurrent = true;
  //Serial.print("Current Value: ");
  //Serial.println(diodoList[selected_diodo_index].getCurrentValue());
}

void changeCoarseTempSelected() {
  int16_t increaseValue = -encoder->getValue();
  if (increaseValue > 0) {
    increaseTempByValue(1);
  } else if (increaseValue < 0) {
    increaseTempByValue(-1);
  }
}

void changeFineTempSelected() {
  int16_t increaseValue = -encoder->getValue();
  if (increaseValue > 0) {
    increaseTempByValue(pow(10, -FINE_TEMP_DIGITS_RESOLUTION));
  } else if (increaseValue < 0) {
    increaseTempByValue(-pow(10, -FINE_TEMP_DIGITS_RESOLUTION));
  }
}

void increaseTempByValue(float increaseValue) {
  //Serial.print("Increase Temp Value: ");
  //Serial.println(increaseValue);
  float value = diodoList[selected_diodo_index].getTempValue() + increaseValue;
  if (value >= diodoList[selected_diodo_index].getMaxTempValue()) {
    value = diodoList[selected_diodo_index].getMaxTempValue();
  } else if (value <= diodoList[selected_diodo_index].getMinTempValue()) {
    value = diodoList[selected_diodo_index].getMinTempValue();
  }
  diodoList[selected_diodo_index].setTempValue(value);
  reedrawTemp = true;
  //Serial.print("Temp Value: ");
  //Serial.println(diodoList[selected_diodo_index].getTempValue());
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
  for (int i = -angle + inc / 2; i < angle - inc / 2; i += inc) {
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
  display.drawCentreString(buf, x, y - (LARGE_TEXT_FONT_HEIGHT / 2), LARGE_TEXT_FONT); // Value in middle

  display.setTextSize(1);
  display.setTextPadding(0);
  // Print units, if the meter is large then use big font 4, othewise use 2
  display.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  display.drawCentreString(units, x, y + (LARGE_TEXT_FONT_HEIGHT / 2), MEDIUM_TEXT_FONT); // Units display
  // Calculate and return right hand side x coordinate
  return x + r;
}


// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value) {
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
