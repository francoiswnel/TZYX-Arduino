/*
    TZYX by Francois W. Nel
    A stack logic calculator.
    Makes use of a 320x240 TFT LCD touch shield by Mcufriend.com.
*/

#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

typedef byte(*KeyFunction) ();

/* TouchSensor configuration */
const byte XP = 8;
const byte XM = A2;
const byte YP = A3;
const byte YM = 9;

/* TouchSensor calibration */
const unsigned int CalibrationLeft = 923;
const unsigned int CalibrationRight = 102;
const unsigned int CalibrationTop = 934;
const unsigned int CalibrationBottom = 70;

/* Colours */
const unsigned int Black = 0x0000;
const unsigned int White = 0xFFFF;
const unsigned int Grey = 0x18E3;
const unsigned int Orange = 0xFC00;

/* Global constants */
const byte DisplayWidth = 240;
const unsigned int DisplayHeight = 320;

const byte OutputWindowHeight = 80;
const byte OutputWindowHeightInKeys = 2;
const byte OutputWindowTextPadding = 17;
const byte TRegisterPaddingTop = 7;
const byte RegisterPaddingLeft = 12;
const byte MaximumRegisterAsStringLength = 15;
const unsigned int OutputWindowTextColour = Grey;
const unsigned int OutputWindowBackgroundColour = White;

const byte KeypadWidth = 240;
const byte KeypadHeight = 240;
const byte KeypadWidthInKeys = 4;
const byte KeypadHeightInKeys = 6;
const byte KeyWidth = 60;
const byte KeyHeight = 40;
const byte LabelPaddingTop = 14;

const unsigned int KeyLabelTextColour = White;
const unsigned int EnterKeyLabelTextColour = Orange;
const unsigned int KeypadGridLinesColour = Black;
const unsigned int KeypadBackgroundColour = Grey;

/* Derived global constants */
const byte ZRegisterPaddingTop = TRegisterPaddingTop + OutputWindowTextPadding;
const byte YRegisterPaddingTop = ZRegisterPaddingTop + OutputWindowTextPadding;
const byte XRegisterPaddingTop = YRegisterPaddingTop + OutputWindowTextPadding;
const byte OutputWindowBottom = OutputWindowHeight - 1;
const byte NumberOfTrailingDecimals = MaximumRegisterAsStringLength - 2;
const byte MaximumInputAsStringLength = MaximumRegisterAsStringLength - 1;

const byte FirstHorizontalLine = DisplayHeight - 1 - KeypadHeight;
const unsigned int LastHorizontalLine = DisplayHeight - 1 - KeyHeight;

const byte FirstVerticalLine = KeyWidth - 1;
const byte MiddleVerticalLine = FirstVerticalLine + KeyWidth;
const byte LastVerticalLine = MiddleVerticalLine + KeyWidth;

const byte EnterKeyTop = FirstHorizontalLine + KeyHeight;
const byte EnterKeyBottom = EnterKeyTop + KeyHeight;

/* Structs and classes */
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
    String _label;
    byte _labelPaddingLeft;
};

/* Global variables */
MCUFRIEND_kbv Display;
TouchScreen TouchSensor(XP, YP, XM, YM, 300);
TSPoint RawInput;
Coordinate CalibratedInput;
Coordinate SelectedKey;
byte InputCursorPosition;
bool InInputMode;
bool InWarningMode;

/* The stack */
float TRegister = 0.0;
float ZRegister = 0.0;
float YRegister = 0.0;
float XRegister = 0.0;
float LastXRegister = 0.0;
String Input = "";
String ErrorMessage = "";

/* Forward declarations */
void setup();
void loop();

void initialiseDisplay();
void drawInterface();
void drawOutputWindow();
void drawOutput();
void drawKeypad();
void drawKeypadHorizontalLines();
void drawKeypadVerticalLines();
void drawKeys();

void waitForTouch();
bool touchSensorPressed();
void readTouchSensor();
bool determineSelectedKey();
void performKeyFunction();

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

void negateX();
byte invertKey();
byte rootKey();
byte exponentKey();
byte rollKey();
byte enterKey();
byte swapKey();
byte clearKey();
byte divisionKey();
byte multiplicationKey();
byte subtractionKey();
byte additionKey();
byte radixKey();
byte negationKey();
byte number0Key();
byte number1Key();
byte number2Key();
byte number3Key();
byte number4Key();
byte number5Key();
byte number6Key();
byte number7Key();
byte number8Key();
byte number9Key();

/* Keypad definitions */
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

KeyFunction KeyFunctions[KeypadHeightInKeys][KeypadWidthInKeys] {
  {
    invertKey,
    rootKey,
    exponentKey,
    rollKey,
  },
  {
    enterKey,
    enterKey,
    swapKey,
    clearKey,
  },
  {
    number7Key,
    number8Key,
    number9Key,
    divisionKey,
  },
  {
    number4Key,
    number5Key,
    number6Key,
    multiplicationKey,
  },
  {
    number1Key,
    number2Key,
    number3Key,
    subtractionKey,
  },
  {
    number0Key,
    radixKey,
    negationKey,
    additionKey,
  },
};

/* Main functions */
void setup() {
  initialiseDisplay();
  drawInterface();
  InInputMode = false;
}

void loop() {
  waitForTouch();

  bool validInput = determineSelectedKey();

  if (validInput) performKeyFunction();
}

/* Interface functions */
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
  Display.fillRect(0, 0, DisplayWidth, OutputWindowBottom, OutputWindowBackgroundColour);
  drawOutput();
}

void drawOutput() {
  Display.setTextSize(2);
  Display.setTextColor(OutputWindowTextColour, OutputWindowBackgroundColour);

  if (InWarningMode) {
    Display.fillRect(0, XRegisterPaddingTop, DisplayWidth, OutputWindowTextPadding, OutputWindowBackgroundColour);
    Display.setCursor(RegisterPaddingLeft, XRegisterPaddingTop);
    Display.print("Error: " + ErrorMessage);
  }
  else if (InInputMode) {
    String InputSubstringForDisplay = (Input.length() <= MaximumInputAsStringLength) ? Input : Input.substring(Input.length() - MaximumInputAsStringLength, Input.length());

    Display.fillRect(0, XRegisterPaddingTop, DisplayWidth, OutputWindowTextPadding, OutputWindowBackgroundColour);
    Display.setCursor(RegisterPaddingLeft, XRegisterPaddingTop);
    Display.print("X: " + InputSubstringForDisplay);

    int characterWidth = 12;
    InputCursorPosition = (4 * characterWidth) + InputSubstringForDisplay.length() * characterWidth;
  }
  else {
    String TRegisterAsString = String(TRegister, NumberOfTrailingDecimals).substring(0, MaximumRegisterAsStringLength);
    String ZRegisterAsString = String(ZRegister, NumberOfTrailingDecimals).substring(0, MaximumRegisterAsStringLength);
    String YRegisterAsString = String(YRegister, NumberOfTrailingDecimals).substring(0, MaximumRegisterAsStringLength);
    String XRegisterAsString = String(XRegister, NumberOfTrailingDecimals).substring(0, MaximumRegisterAsStringLength);

    Display.setCursor(RegisterPaddingLeft, TRegisterPaddingTop);
    Display.println("T: " + TRegisterAsString);
    Display.setCursor(RegisterPaddingLeft, ZRegisterPaddingTop);
    Display.println("Z: " + ZRegisterAsString);
    Display.setCursor(RegisterPaddingLeft, YRegisterPaddingTop);
    Display.println("Y: " + YRegisterAsString);
    Display.setCursor(RegisterPaddingLeft, XRegisterPaddingTop);
    Display.println("X: " + XRegisterAsString);
  }
}

void drawKeypad() {
  Display.fillRect(0, FirstHorizontalLine, DisplayWidth, DisplayHeight, KeypadBackgroundColour);
  drawKeypadHorizontalLines();
  drawKeypadVerticalLines();
  drawKeys();
}

void drawKeypadHorizontalLines() {
  for (unsigned int y = FirstHorizontalLine; y <= LastHorizontalLine; y += KeyHeight) {
    Display.drawLine(0, y, DisplayWidth - 1, y, KeypadGridLinesColour);
  }
}

void drawKeypadVerticalLines() {
  Display.drawLine(FirstVerticalLine, FirstHorizontalLine, FirstVerticalLine, EnterKeyTop, KeypadGridLinesColour);
  Display.drawLine(FirstVerticalLine, EnterKeyBottom, FirstVerticalLine, DisplayHeight - 1, KeypadGridLinesColour);
  Display.drawLine(MiddleVerticalLine, FirstHorizontalLine, MiddleVerticalLine, DisplayHeight - 1, KeypadGridLinesColour);
  Display.drawLine(LastVerticalLine, FirstHorizontalLine, LastVerticalLine, DisplayHeight - 1, KeypadGridLinesColour);
}

void drawKeys() {
  Display.setTextSize(2);

  for (byte i = 0; i < 6; i++) {
    for (byte j = 0; j < 4; j++) {
      if ((i == 1) && (j == 1)) continue;

      Display.setTextColor(((i == 1) && (j == 0)) ? EnterKeyLabelTextColour : KeyLabelTextColour);
      Display.setCursor((KeyWidth * j) + Keys[i][j]->getLabelPaddingLeft(), (KeyHeight * i) + FirstHorizontalLine + LabelPaddingTop);
      Display.println(Keys[i][j]->getLabel());
    }
  }
}

/* User interaction functions */
void waitForTouch() {
  while (!touchSensorPressed()) {
    blinkCursor();
  }
  while (touchSensorPressed()) {
    blinkCursor();
  }
  while (!touchSensorPressed()) {
    blinkCursor();
  }
}

void blinkCursor() {
  if (InInputMode) {
    unsigned int seconds = millis() / 1000;
    Display.setTextSize(2);

    if (((seconds) % 2) == 0) {
      Display.setTextColor(OutputWindowTextColour, OutputWindowBackgroundColour);
    }
    else {
      Display.setTextColor(OutputWindowBackgroundColour, OutputWindowBackgroundColour);
    }

    Display.setCursor(InputCursorPosition, XRegisterPaddingTop);
    Display.print("_");
  }
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

bool determineSelectedKey() {
  CalibratedInput.x = map(RawInput.x, CalibrationLeft, CalibrationRight, 0, DisplayWidth);
  CalibratedInput.y = map(RawInput.y, CalibrationTop, CalibrationBottom, 0, DisplayHeight);

  bool inputIsOverKeypad = !(CalibratedInput.y < OutputWindowHeight);

  if (inputIsOverKeypad) {
    SelectedKey.x = map(CalibratedInput.x, 0, KeypadWidth, 0, KeypadWidthInKeys);
    SelectedKey.y = map(CalibratedInput.y, 0, KeypadHeight, 0, KeypadHeightInKeys) - OutputWindowHeightInKeys;
  }

  return inputIsOverKeypad;
}

void performKeyFunction() {
  KeyFunctions[SelectedKey.y][SelectedKey.x]();
}

void enterInputMode() {
  InInputMode = true;
}

void exitInputMode() {
  Input = "";
  InInputMode = false;
}

/* Key functions */
byte invertKey() {
  drawOutput();
  return 1;
}

byte rootKey() {
  drawOutput();
  return 1;
}

byte exponentKey() {
  drawOutput();
  return 1;
}

byte rollKey() {
  drawOutput();
  return 1;
}

byte enterKey() {
  drawOutput();
  return 1;
}

byte swapKey() {
  drawOutput();
  return 1;
}

byte clearKey() {
  drawOutput();
  return 1;
}

byte divisionKey() {
  drawOutput();
  return 1;
}

byte multiplicationKey() {
  drawOutput();
  return 1;
}

byte subtractionKey() {
  drawOutput();
  return 1;
}

byte additionKey() {
  drawOutput();
  return 1;
}

byte radixKey() {
  drawOutput();
  return 1;
}

byte negationKey() {
  if (InInputMode) Input = "-" + Input;
  else negateX();

  drawOutput();

  return 1;
}

byte number0Key() {
  enterInputMode();
  Input.concat("0");
  drawOutput();
  return 1;
}

byte number1Key() {
  enterInputMode();
  Input.concat("1");
  drawOutput();
  return 1;
}

byte number2Key() {
  enterInputMode();
  Input.concat("2");
  drawOutput();
  return 1;
}

byte number3Key() {
  enterInputMode();
  Input.concat("3");
  drawOutput();
  return 1;
}

byte number4Key() {
  enterInputMode();
  Input.concat("4");
  drawOutput();
  return 1;
}

byte number5Key() {
  enterInputMode();
  Input.concat("5");
  drawOutput();
  return 1;
}

byte number6Key() {
  enterInputMode();
  Input.concat("6");
  drawOutput();
  return 1;
}

byte number7Key() {
  enterInputMode();
  Input.concat("7");
  drawOutput();
  return 1;
}

byte number8Key() {
  enterInputMode();
  Input.concat("8");
  drawOutput();
  return 1;
}

byte number9Key() {
  enterInputMode();
  Input.concat("9");
  drawOutput();
  return 1;
}

/* Calculator functions */
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

void negateX() {
  LastXRegister = XRegister;
  XRegister = 0 - XRegister;
}
