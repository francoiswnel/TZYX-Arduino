#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

// TouchSensor configuration.
const byte XP = 8;
const byte XM = A2;
const byte YP = A3;
const byte YM = 9;

// TouchSensor calibration.
const unsigned int CalibrationLeft = 923;
const unsigned int CalibrationRight = 102;
const unsigned int CalibrationTop = 934;
const unsigned int CalibrationBottom = 70;

// Define colours.
const unsigned int Black = 0x0000;
const unsigned int White = 0xFFFF;
const unsigned int Grey = 0x18E3;
const unsigned int Orange = 0xFC00;

// Constant dimensions.
const byte DisplayWidth = 240;
const unsigned int DisplayHeight = 320;
const byte OutputWindowHeight = 80;
const byte OutputWindowHeightInKeys = 2;
const byte KeypadWidth = 240;
const byte KeypadHeight = 240;
const byte KeypadWidthInKeys = 4;
const byte KeypadHeightInKeys = 6;

// Custom classes.
struct Coordinate {
  unsigned int x;
  unsigned int y;
};

class Key {
  public:
    Key(String label) : _label(label) {
      switch (_label.length()) {
        default:
        case 1:
          _labelPaddingLeft = 25;
          break;
        case 3:
          _labelPaddingLeft = 13;
          break;
        case 4:
          _labelPaddingLeft = 8;
          break;
        case 5:
          _labelPaddingLeft = 30;
          break;
      }
    }

    String getLabel() {
      return _label;
    }

    byte getLabelPaddingLeft() {
      return _labelPaddingLeft;
    }

  private:
    Coordinate _topLeft;
    Coordinate _bottomRight;
    String _label;
    byte _labelPaddingLeft;
};

// Global variables.
MCUFRIEND_kbv Display;
TouchScreen TouchSensor(XP, YP, XM, YM, 300);
TSPoint RawInput;
Coordinate CalibratedInput;
Coordinate SelectedKey;

float TRegister = 0.0;
float ZRegister = 0.0;
float YRegister = 0.0;
float XRegister = 0.0;
float LastXRegister = 0.0;

Key* Keys[KeypadHeightInKeys][KeypadWidthInKeys] = {
  {
    new Key("1/X"),
    new Key("ROOT"),
    new Key("Y^X"),
    new Key("ROLL")
  },
  {
    new Key("ENTER"),
    new Key("ENTER"),
    new Key("SWAP"),
    new Key("CLR")
  },
  {
    new Key("7"),
    new Key("8"),
    new Key("9"),
    new Key("/")
  },
  {
    new Key("4"),
    new Key("5"),
    new Key("6"),
    new Key("*")
  },
  {
    new Key("1"),
    new Key("2"),
    new Key("3"),
    new Key("-")
  },
  {
    new Key("0"),
    new Key("."),
    new Key("NEG"),
    new Key("+")
  }
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
void drawKeys(byte keyWidth, byte keyHeight, byte KeyPaddingTop, unsigned int KeyTextColour, unsigned int keypadEnterKeyLabelTextColour);
void waitForTouch();
bool touchSensorPressed();
void readTouchSensor();
void determineSelectedKey();
void push(float NewXRegister = XRegister);
float pop();
void rollDown();
void swapXAndY();
void clearX();
void invertX();
void addXToY();
void subtractYFromX();
void multiplyXByY();
void divideYByX();
void xthRootOfY();
void yToPowerOfX();

void setup() {
  initialiseDisplay();
  drawInterface();
}

void loop() {
  waitForTouch();

  determineSelectedKey();
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
  unsigned int outputWindowTextColour = Grey;
  unsigned int outputWindowBackgroundColour = White;

  byte maximumRegisterAsStringLength = 15;
  byte textPaddingTop = 8;

  byte outputWindowBottom = OutputWindowHeight - 1;
  byte numberOfTrailingDecimals = maximumRegisterAsStringLength - 2;

  String TRegisterAsString = String(TRegister, numberOfTrailingDecimals).substring(0, maximumRegisterAsStringLength);
  String ZRegisterAsString = String(ZRegister, numberOfTrailingDecimals).substring(0, maximumRegisterAsStringLength);
  String YRegisterAsString = String(YRegister, numberOfTrailingDecimals).substring(0, maximumRegisterAsStringLength);
  String XRegisterAsString = String(XRegister, numberOfTrailingDecimals).substring(0, maximumRegisterAsStringLength);

  Display.fillRect(0, 0, DisplayWidth, outputWindowBottom, outputWindowBackgroundColour);
  Display.setTextSize(2);
  Display.setTextColor(outputWindowTextColour);
  Display.setCursor(0, textPaddingTop);
  Display.println(" T: " + TRegisterAsString);
  Display.println(" Z: " + ZRegisterAsString);
  Display.println(" Y: " + YRegisterAsString);
  Display.println(" X: " + XRegisterAsString);
}

void drawKeypad() {
  unsigned int KeyTextColour = White;
  unsigned int keypadEnterKeyLabelTextColour = Orange;
  unsigned int keypadGridLinesColour = Black;
  unsigned int keypadBackgroundColour = Grey;

  byte keyWidth = 60;
  byte keyHeight = 40;
  byte KeyPaddingTop = 14;

  byte firstHorizontalLine = DisplayHeight - 1 - KeypadHeight;
  unsigned int lastHorizontalLine = DisplayHeight - 1 - keyHeight;
  byte firstVerticalLine = keyWidth - 1;
  byte middleVerticalLine = firstVerticalLine + keyWidth;
  byte lastVerticalLine = middleVerticalLine + keyWidth;

  Display.fillRect(0, firstHorizontalLine, DisplayWidth, DisplayHeight, keypadBackgroundColour);
  drawKeypadHorizontalLines(keyHeight, firstHorizontalLine, lastHorizontalLine, keypadGridLinesColour);
  drawKeypadVerticalLines(keyHeight, firstHorizontalLine, firstVerticalLine, middleVerticalLine, lastVerticalLine, keypadGridLinesColour);

  drawKeys(keyWidth, keyHeight, firstHorizontalLine + KeyPaddingTop, KeyTextColour, keypadEnterKeyLabelTextColour);
}

void drawKeypadHorizontalLines(byte keyHeight, byte firstHorizontalLine, unsigned int lastHorizontalLine, unsigned int keypadGridLinesColour) {
  for (unsigned int y = firstHorizontalLine; y <= lastHorizontalLine; y += keyHeight) {
    Display.drawLine(0, y, DisplayWidth - 1, y, keypadGridLinesColour);
  }
}

void drawKeypadVerticalLines(byte keyHeight, byte firstHorizontalLine, byte firstVerticalLine, byte middleVerticalLine, byte lastVerticalLine, unsigned int keypadGridLinesColour) {
  byte enterKeyTop = firstHorizontalLine + keyHeight;
  byte enterKeyBottom = enterKeyTop + keyHeight;

  Display.drawLine(firstVerticalLine, firstHorizontalLine, firstVerticalLine, enterKeyTop, keypadGridLinesColour);
  Display.drawLine(firstVerticalLine, enterKeyBottom, firstVerticalLine, DisplayHeight - 1, keypadGridLinesColour);
  Display.drawLine(middleVerticalLine, firstHorizontalLine, middleVerticalLine, DisplayHeight - 1, keypadGridLinesColour);
  Display.drawLine(lastVerticalLine, firstHorizontalLine, lastVerticalLine, DisplayHeight - 1, keypadGridLinesColour);
}

void drawKeys(byte keyWidth, byte keyHeight, byte KeyPaddingTop, unsigned int KeyTextColour, unsigned int keypadEnterKeyLabelTextColour) {
  Display.setTextSize(2);

  for (byte i = 0; i < 6; i++) {
    for (byte j = 0; j < 4; j++) {
      if ((i == 1) && (j == 1)) continue;

      Display.setTextColor(((i == 1) && (j == 0)) ? keypadEnterKeyLabelTextColour : KeyTextColour);
      Display.setCursor((keyWidth * j) + Keys[i][j]->getLabelPaddingLeft(), (keyHeight * i) + KeyPaddingTop);
      Display.println(Keys[i][j]->getLabel());
    }
  }
}

void waitForTouch() {
  while (!touchSensorPressed()) {}
  while (touchSensorPressed()) {}
  while (!touchSensorPressed()) {}
}

bool touchSensorPressed() {
  byte count = 0;
  byte loopDelay = 5;
  byte durationToWait = 25;
  byte durationToWaitDividedByLoopDelay = durationToWait / loopDelay;
  bool currentPressedState = false;
  bool previousPressedState = false;

  while (count < durationToWaitDividedByLoopDelay) {
    readTouchSensor();
    currentPressedState = (RawInput.z > 20);

    if (currentPressedState == previousPressedState) {
      count++;
    }
    else {
      count = 0;
    }

    previousPressedState = currentPressedState;
    delay(loopDelay);
  }

  return previousPressedState;
}

void readTouchSensor() {
  RawInput = TouchSensor.getPoint();
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);
  digitalWrite(XM, HIGH);
}

void determineSelectedKey() {
  CalibratedInput.x = map(RawInput.x, CalibrationLeft, CalibrationRight, 0, DisplayWidth);
  CalibratedInput.y = map(RawInput.y, CalibrationTop, CalibrationBottom, 0, DisplayHeight);

  bool inputIsOverKeypad = !(CalibratedInput.y < OutputWindowHeight);

  if (inputIsOverKeypad) {
    SelectedKey.x = map(CalibratedInput.x, 0, KeypadWidth, 0, KeypadWidthInKeys);
    SelectedKey.y = map(CalibratedInput.y, 0, KeypadHeight, 0, KeypadHeightInKeys) - OutputWindowHeightInKeys;
  }
}

void push(float NewXRegister = XRegister) {
  LastXRegister = XRegister;
  TRegister = ZRegister;
  ZRegister = YRegister;
  YRegister = XRegister;
  XRegister = NewXRegister;
}

float pop() {
  LastXRegister = XRegister;
  XRegister = YRegister;
  YRegister = ZRegister;
  ZRegister = TRegister;
  return LastXRegister;
}

void rollDown() {
  LastXRegister = XRegister;
  XRegister = YRegister;
  YRegister = ZRegister;
  ZRegister = TRegister;
  TRegister = LastXRegister;
}

void swapXAndY() {
  LastXRegister = XRegister;
  XRegister = YRegister;
  YRegister = LastXRegister;
}

void clearX() {
  LastXRegister = XRegister;
  XRegister = 0.0;
}

void addXToY() {
  LastXRegister = pop();
  XRegister += LastXRegister;
}

void subtractYFromX() {
  LastXRegister = pop();
  XRegister -= LastXRegister;
}

void multiplyXByY() {
  LastXRegister = pop();
  XRegister *= LastXRegister;
}

void divideYByX() {
  if (XRegister != 0.0) {
    LastXRegister = pop();
    XRegister /= LastXRegister;
  }
}

void invertX() {
  if (XRegister != 0.0) {
    LastXRegister = XRegister;
    XRegister = 1 / XRegister;
  }
}

void xthRootOfY() {
  if ((fmod(XRegister, 2.0) != 0.0) && XRegister != 0.0) {
    LastXRegister = pop();
    XRegister = pow(XRegister, 1 / LastXRegister);
  }
}

void yToPowerOfX() {
  LastXRegister = pop();
  XRegister = pow(XRegister, LastXRegister);
}

