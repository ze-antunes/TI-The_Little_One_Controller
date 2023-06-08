#include <Esplora.h>
#include <ArduinoJson.h>

// Variables will change:
int buttonState[5];
int lastButtonState[5] = { -1, -1, -1, -1, -1 };
unsigned long lastDebounceTime[5] = { 0, 0, 0, 0, 0 };
unsigned long debounceDelay = 200;

String msg[] = { "JUMP", "ATTACK", "PAUSE", "INTERACTION", "INVENTORY" };
bool printStop = false;
bool fsrRead = false;

const byte CH_TINKERKIT_INA = 8;

StaticJsonDocument<64> jsonDocument;

int previousSliderValue = -1;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int xValue = Esplora.readJoystickX();
  int yValue = Esplora.readJoystickY();

  int buttonJoystick = Esplora.readJoystickSwitch();
  int buttonCross = Esplora.readButton(SWITCH_1);
  int buttonSquare = Esplora.readButton(SWITCH_2);
  int buttonTriangle = Esplora.readButton(SWITCH_3);
  int buttonCircle = Esplora.readButton(SWITCH_4);

  int pins[] = { buttonCross, buttonSquare, buttonTriangle, buttonCircle, buttonJoystick };

  buttonDebounce(pins);

  int fsrValue = readTinkerkitInput(0);  // Replace A0 with the appropriate analog pin for your FSR sensor

  jsonDocument.clear();
  jsonDocument["fsrValue"] = fsrValue;

  if (fsrValue <= 100) {
    fsrRead = false;
  } else {
    fsrRead = true;
    Serial.println(jsonMessage("", previousSliderValue));
  }

  if (Serial.available() > 0) {
    String message = Serial.readString();
    Serial.println(message);
    if (message == "H") {
      Esplora.tone(440);  // Play sound on the buzzer
      delay(300);
      Esplora.noTone();
    }
  }

  int sliderValue = Esplora.readSlider();

  if (sliderValue != previousSliderValue) {
    Serial.println(jsonMessage("", sliderValue));
    previousSliderValue = sliderValue;
  }

  String combinedMsg = combinedButtonMessage();

  if (combinedMsg == "" && xValue <= -100) {
    Serial.println(jsonMessage("LEFT", previousSliderValue));
    delay(10);
    printStop = false;
  } else if (combinedMsg == "" && xValue >= 100) {
    Serial.println(jsonMessage("RIGHT", previousSliderValue));
    delay(10);
    printStop = false;
  } else if (combinedMsg != "" && xValue <= -100) {
    Serial.println(jsonMessage("LEFT " + combinedMsg, previousSliderValue));
    delay(10);
    printStop = false;
  } else if (combinedMsg != "" && xValue >= 100) {
    Serial.println(jsonMessage("RIGHT " + combinedMsg, previousSliderValue));
    delay(10);
    printStop = false;
  } else if (xValue > -100 && xValue < 100 && !printStop) {
    Serial.println(jsonMessage("STOP", previousSliderValue));
    delay(10);
    printStop = true;
  }
}

void buttonDebounce(int pins[]) {
  for (int i = 0; i < 5; i++) {
    if (pins[i] == 0 && i != lastButtonState[i]) {
      lastButtonState[i] = i;
      lastDebounceTime[i] = millis();
      printButtonMessage(i);
    }
  }

  for (int i = 0; i < 5; i++) {
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      lastButtonState[i] = -1;
    }
  }
}

void printButtonMessage(int buttonIndex) {
  Serial.println(jsonMessage(msg[buttonIndex], previousSliderValue));
  delay(10);
}

String combinedButtonMessage() {
  String combinedMsg = "";

  for (int i = 0; i < 5; i++) {
    if (lastButtonState[i] != -1) {
      combinedMsg += msg[i] + " ";
    }
  }

  return combinedMsg;
}

int readTinkerkitInput(byte whichInput) {  // return 0-1023 from Tinkerkit Input A or B
  return readChannel(whichInput + CH_TINKERKIT_INA);
}  //   as defined above

int readChannel(byte channel) {                  // as Esplora.readChannel is a private function
  digitalWrite(A0, (channel & 1) ? HIGH : LOW);  //  we declare our own as a hack
  digitalWrite(A1, (channel & 2) ? HIGH : LOW);  //
  digitalWrite(A2, (channel & 4) ? HIGH : LOW);  // digitalWrite sets address lines for the input
  digitalWrite(A3, (channel & 8) ? HIGH : LOW);
  return analogRead(A4);  // analogRead gets value from MUX chip
}

String jsonMessage(String action, int sliderValue) {

  jsonDocument["action"] = action;
  jsonDocument["sliderValue"] = sliderValue;

  // Serialize the JSON object to a string
  String jsonString;
  serializeJson(jsonDocument, jsonString);

  return jsonString;
}
