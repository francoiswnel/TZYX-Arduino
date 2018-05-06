#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

// TouchSensor configuration.
#define XP 8
#define XM A2
#define YP A3
#define YM 9

// TouchSensor calibration.
#define CALIBRATIONLEFT   923
#define CALIBRATIONRIGHT  102
#define CALIBRATIONTOP    934
#define CALIBRATIONBOTTOM 70

// Define colours.
#define BLACK     0x0000
#define WHITE     0xFFFF
#define ORANGE    0xFC00
#define DARKGREY  0x18E3
#define LIGHTGREY 0x8410

// Global constants.
const byte OutputWindowHeight = 80;

// Custom classes.
class KeypadLabel {
  public:
    KeypadLabel(String label) : _label(label) {
      switch (_label.length()) {
        default:
        case 1:
          _paddingLeft = 25;
          break;
        case 3:
          _paddingLeft = 13;
          break;
        case 4:
          _paddingLeft = 8;
          break;
        case 5:
          _paddingLeft = 30;
          break;
      }
    }

    String getLabel() {
      return _label;
    }

    byte getPaddingLeft() {
      return _paddingLeft;
    }

  private:
    String _label;
    byte _paddingLeft;
};

// Forward declarations.
void setup();
void loop();
void initialiseDisplay();
void drawInterface();
void drawOutputWindow();
void drawKeypad();
void drawKeypadHorizontalLines(byte keyHeight, byte firstHorizontalLine, unsigned int lastHorizontalLine, unsigned int keypadGridLinesColour);
void drawKeypadVerticalLines(byte keyHeight, byte firstHorizontalLine, byte firstVerticalLine, byte middleVerticalLine, byte lastVerticalLine, unsigned int keypadGridLinesColour);
void drawKeypadLabels(byte keyWidth, byte keyHeight, byte keypadLabelPaddingTop, unsigned int keypadLabelTextColour, unsigned int keypadEnterKeyLabelTextColour);
void waitForTouch();
bool touchSensorPressed();
void readTouchSensor();

// Global variables.
MCUFRIEND_kbv Display;
TouchScreen TouchSensor(XP, YP, XM, YM, 300);
TSPoint TouchPoint;

float TRegister = 0.0;
float ZRegister = 0.0;
float YRegister = 0.0;
float XRegister = 0.0;

KeypadLabel* KeypadLabels[6][4] = {
  {
    new KeypadLabel("1/X"),
    new KeypadLabel("ROOT"),
    new KeypadLabel("Y^X"),
    new KeypadLabel("ROLL")
  },
  {
    new KeypadLabel("ENTER"),
    new KeypadLabel(""),
    new KeypadLabel("SWAP"),
    new KeypadLabel("CLR")
  },
  {
    new KeypadLabel("7"),
    new KeypadLabel("8"),
    new KeypadLabel("9"),
    new KeypadLabel("/")
  },
  {
    new KeypadLabel("4"),
    new KeypadLabel("5"),
    new KeypadLabel("6"),
    new KeypadLabel("*")
  },
  {
    new KeypadLabel("1"),
    new KeypadLabel("2"),
    new KeypadLabel("3"),
    new KeypadLabel("-")
  },
  {
    new KeypadLabel("0"),
    new KeypadLabel("."),
    new KeypadLabel("NEG"),
    new KeypadLabel("+")
  }
};

void setup() {
  initialiseDisplay();
  drawInterface();
}

void loop() {
  waitForTouch();

  byte selectedKey = determineSelectedKey();

  drawOutputWindow();
}

void waitForTouch() {
  while (!touchSensorPressed()) {}
  while (touchSensorPressed()) {}
  while (!touchSensorPressed()) {}
}

bool touchSensorPressed() {
  byte count = 0;
  bool currentPressedState = false;
  bool previousPressedState = false;

  while (count < 10) {
    readTouchSensor();
    currentPressedState = (TouchPoint.z > 20);

    if (currentPressedState == previousPressedState) {
      count++;
    }
    else {
      count = 0;
    }

    previousPressedState = currentPressedState;
    delay(5);
  }

  return previousPressedState;
}

void readTouchSensor() {
  TouchPoint = TouchSensor.getPoint();
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);
  digitalWrite(XM, HIGH);
}

byte determineSelectedKey() {
  unsigned int inputX = map(TouchPoint.x, CALIBRATIONLEFT, CALIBRATIONRIGHT, 0, 240);
  unsigned int inputY = map(TouchPoint.y, CALIBRATIONTOP, CALIBRATIONBOTTOM, 0, 320);

  byte keypadVerticalHalf = 200;
  byte keypadFirstQuarter = 60;
  byte keypadHorizontalHalf = 120;
  byte keypadThirdQuarter = 180;

  if (inputY < OutputWindowHeight) {
    return 255;
  } else if (inputY < keypadVerticalHalf) {
    if (inputX < keypadHorizontalHalf) {
      if (inputX < keypadFirstQuarter) {

      } else {

      }
    } else {
      if (inputX < keypadFirstQuarter) {

      } else {

      }
    }
  } else {
    if (inputX < keypadHorizontalHalf) {
      if (inputX < keypadThirdQuarter) {

      } else {

      }
    } else {
      if (inputX < keypadThirdQuarter) {

      } else {

      }
    }
  }
}

void initialiseDisplay() {
  Display.reset();
  unsigned int displayId = Display.readID();
  Display.begin(displayId);
  Display.setRotation(0);
}

void drawInterface() {
  drawOutputWindow();
  drawKeypad();
}

void drawOutputWindow() {
  unsigned int outputWindowTextColour = DARKGREY;
  unsigned int outputWindowBackgroundColour = WHITE;


  byte maximumRegisterAsStringLength = 15;
  byte textPaddingTop = 8;

  byte outputWindowBottom = OutputWindowHeight - 1;
  byte numberOfTrailingDecimals = maximumRegisterAsStringLength - 2;

  String TRegisterAsString = String(TRegister, numberOfTrailingDecimals).substring(0, maximumRegisterAsStringLength);
  String ZRegisterAsString = String(ZRegister, numberOfTrailingDecimals).substring(0, maximumRegisterAsStringLength);
  String YRegisterAsString = String(YRegister, numberOfTrailingDecimals).substring(0, maximumRegisterAsStringLength);
  String XRegisterAsString = String(XRegister, numberOfTrailingDecimals).substring(0, maximumRegisterAsStringLength);

  Display.fillRect(0, 0, Display.width(), outputWindowBottom, outputWindowBackgroundColour);
  Display.setTextSize(2);
  Display.setTextColor(outputWindowTextColour);
  Display.setCursor(0, textPaddingTop);
  Display.println(" T: " + TRegisterAsString);
  Display.println(" Z: " + ZRegisterAsString);
  Display.println(" Y: " + YRegisterAsString);
  Display.println(" X: " + XRegisterAsString);
}

void drawKeypad() {
  unsigned int keypadLabelTextColour = WHITE;
  unsigned int keypadEnterKeyLabelTextColour = ORANGE;
  unsigned int keypadGridLinesColour = BLACK;
  unsigned int keypadBackgroundColour = DARKGREY;

  byte keyWidth = 60;
  byte keyHeight = 40;
  byte keypadHeight = 240;
  byte keypadLabelPaddingTop = 14;

  byte firstHorizontalLine = Display.height() - 1 - keypadHeight;
  unsigned int lastHorizontalLine = Display.height() - 1 - keyHeight;
  byte firstVerticalLine = keyWidth - 1;
  byte middleVerticalLine = firstVerticalLine + keyWidth;
  byte lastVerticalLine = middleVerticalLine + keyWidth;

  Display.fillRect(0, firstHorizontalLine, Display.width(), Display.height(), keypadBackgroundColour);
  drawKeypadHorizontalLines(keyHeight, firstHorizontalLine, lastHorizontalLine, keypadGridLinesColour);
  drawKeypadVerticalLines(keyHeight, firstHorizontalLine, firstVerticalLine, middleVerticalLine, lastVerticalLine, keypadGridLinesColour);

  drawKeypadLabels(keyWidth, keyHeight, firstHorizontalLine + keypadLabelPaddingTop, keypadLabelTextColour, keypadEnterKeyLabelTextColour);
}

void drawKeypadHorizontalLines(byte keyHeight, byte firstHorizontalLine, unsigned int lastHorizontalLine, unsigned int keypadGridLinesColour) {
  for (unsigned int y = firstHorizontalLine; y <= lastHorizontalLine; y += keyHeight) {
    Display.drawLine(0, y, Display.width() - 1, y, keypadGridLinesColour);
  }
}

void drawKeypadVerticalLines(byte keyHeight, byte firstHorizontalLine, byte firstVerticalLine, byte middleVerticalLine, byte lastVerticalLine, unsigned int keypadGridLinesColour) {
  byte enterKeyTop = firstHorizontalLine + keyHeight;
  byte enterKeyBottom = enterKeyTop + keyHeight;

  Display.drawLine(firstVerticalLine, firstHorizontalLine, firstVerticalLine, enterKeyTop, keypadGridLinesColour);
  Display.drawLine(firstVerticalLine, enterKeyBottom, firstVerticalLine, Display.height() - 1, keypadGridLinesColour);
  Display.drawLine(middleVerticalLine, firstHorizontalLine, middleVerticalLine, Display.height() - 1, keypadGridLinesColour);
  Display.drawLine(lastVerticalLine, firstHorizontalLine, lastVerticalLine, Display.height() - 1, keypadGridLinesColour);
}

void drawKeypadLabels(byte keyWidth, byte keyHeight, byte keypadLabelPaddingTop, unsigned int keypadLabelTextColour, unsigned int keypadEnterKeyLabelTextColour) {
  Display.setTextSize(2);

  for (byte i = 0; i < 6; i++) {
    for (byte j = 0; j < 4; j++) {
      if ((i == 1) && (j == 1)) continue;

      Display.setTextColor(((i == 1) && (j == 0)) ? keypadEnterKeyLabelTextColour : keypadLabelTextColour);
      Display.setCursor((keyWidth * j) + KeypadLabels[i][j]->getPaddingLeft(), (keyHeight * i) + keypadLabelPaddingTop);
      Display.println(KeypadLabels[i][j]->getLabel());
    }
  }
}

