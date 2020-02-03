#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <driver/adc.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include "main.h"
#include "keys.h"

#define ROWS 4
#define COLUMNS 11

uint8_t row_pins[] = {2, 4, 5, 13};
uint8_t column_pins[] = {14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26};

uint16_t layer0[] = {
  KEY_Q,    KEY_W,    KEY_E,    KEY_R,      KEY_T,            0,          KEY_Y,      KEY_U,    KEY_I,      KEY_O,              KEY_P,
  KEY_A,    KEY_S,    KEY_D,    KEY_F,      KEY_G,            0,          KEY_H,      KEY_J,    KEY_K,      KEY_L,              KEY_SEMICOLON,
  KEY_Z,    KEY_X,    KEY_C,    KEY_V,      KEY_B,            MOD_ALT,    KEY_N,      KEY_M,    KEY_COMMA,  KEY_DOT,            KEY_SLASH,
  KEY_ESC,  KEY_TAB,  MOD_CTRL, MOD_SHIFT,  KEY_BACKSPACE,    MOD_GUI,   KEY_SPACE,  MOD_L1,   KEY_MINUS,  KEY_SINGLE_APOSTR,  KEY_ENTER
};

uint16_t layer1[] = {
  KEY_EXCLAMATION, KEY_AT, KEY_ARROW_UP, KEY_CURLY_BRACKET_OPEN, KEY_CURLY_BRACKET_CLOSE, 0, KEY_PAGE_UP, KEY_7, KEY_8, KEY_9, KEY_TIMES,
  KEY_HASH, KEY_ARROW_LEFT, KEY_ARROW_DOWN, KEY_ARROW_RIGHT, KEY_DOLLAR, 0, KEY_PAGE_DOWN, KEY_4, KEY_5, KEY_6, KEY_PLUS,
  KEY_SQUARE_BRACKET_OPEN, KEY_SQUARE_BRACKET_CLOSE, KEY_ROUND_BRACKET_OPEN, KEY_ROUND_BRACKET_CLOSE, KEY_AMPERSAND, MOD_ALT, KEY_GRACE_ACCENT, KEY_1, KEY_2, KEY_3, KEY_BACKSLASH,
  0x81, 0x80, MOD_CTRL, MOD_SHIFT, KEY_BACKSPACE, MOD_GUI, KEY_SPACE, MOD_L1, KEY_DOT, KEY_0, KEY_EQUALS
};

bool keys_pressed_old[ROWS * COLUMNS] = { 0 };
bool keys_pressed[ROWS * COLUMNS] = { 0 };

uint8_t layer = 0;

bool changesToSend = false;

uint8_t keyNr = 0;
uint16_t keyCodes[ROWS * COLUMNS] = { 0 };

uint8_t modifiersSent = 0x00;
uint8_t keyCodesSent[6] = { 0 };

BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;

BLECharacteristic* inputConsumer;

uint8_t buttons = 0;
uint8_t button1 = 0;
uint8_t button2 = 0;
uint8_t button3 = 0;
bool connected = false;

class MyCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer){
    connected = true;
    BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    desc->setNotifications(true);

    Serial.println("connected");
  }

  void onDisconnect(BLEServer* pServer){
    connected = false;
    BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    desc->setNotifications(false);

    Serial.println("disconnected");
  }
};

/*
 * This callback is connected with output report. In keyboard output report report special keys changes, like CAPSLOCK, NUMLOCK
 * We can add digital pins with LED to show status
 * bit 0 - NUM LOCK
 * bit 1 - CAPS LOCK
 * bit 2 - SCROLL LOCK
 */
 class MyOutputCallbacks : public BLECharacteristicCallbacks {
 void onWrite(BLECharacteristic* me){
    uint8_t* value = (uint8_t*)(me->getValue().c_str());
    printf("special keys: %d", *value);
  }
};

void taskServer(void*){

  BLEDevice::init("sAtreus");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks());

  hid = new BLEHIDDevice(pServer);
  input = hid->inputReport(1); // <-- input REPORTID from report map
  output = hid->outputReport(1); // <-- output REPORTID from report map
  inputConsumer = hid->inputReport(2);

  output->setCallbacks(new MyOutputCallbacks());

  std::string name = "simon";
  hid->manufacturer()->setValue(name);

  hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  hid->hidInfo(0x00,0x02);

  BLESecurity *pSecurity = new BLESecurity();
  //pSecurity->setKeySize();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  const uint8_t report[] = {
    USAGE_PAGE(1),      0x01,       // Generic Desktop Ctrls
    USAGE(1),           0x06,       // Keyboard
    COLLECTION(1),      0x01,       // Application
    REPORT_ID(1),       0x01,        //   Report ID (1)
    USAGE_PAGE(1),      0x07,       //   Kbrd/Keypad
    USAGE_MINIMUM(1),   0xE0,
    USAGE_MAXIMUM(1),   0xE7,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_SIZE(1),     0x01,       //   1 uint8_t (Modifier)
    REPORT_COUNT(1),    0x08,
    HIDINPUT(1),           0x02,       //   Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position
    REPORT_COUNT(1),    0x01,       //   1 uint8_t (Reserved)
    REPORT_SIZE(1),     0x08,
    HIDINPUT(1),           0x01,       //   Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position
    REPORT_COUNT(1),    0x06,       //   6 uint8_ts (Keys)
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65,       //   101 keys
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(1),   0x65,
    HIDINPUT(1),           0x00,       //   Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position
    REPORT_COUNT(1),    0x05,       //   5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
    REPORT_SIZE(1),     0x01,
    USAGE_PAGE(1),      0x08,       //   LEDs
    USAGE_MINIMUM(1),   0x01,       //   Num Lock
    USAGE_MAXIMUM(1),   0x05,       //   Kana
    HIDOUTPUT(1),          0x02,       //   Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile
    REPORT_COUNT(1),    0x01,       //   3 bits (Padding)
    REPORT_SIZE(1),     0x03,
    HIDOUTPUT(1),          0x01,       //   Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile
    END_COLLECTION(0),
    USAGE_PAGE(1),      0x0c,       // usage page: consumer devices
    USAGE(1),           0x01,       // usage: consumer control
    COLLECTION(1),      0x01,       // collection: application
    REPORT_ID(1),       0x02,       // report id: 2
    LOGICAL_MINIMUM(1), 0x01,       // logical minimum: 1
    LOGICAL_MAXIMUM(2), 0x9c, 0x02, // logical maximum: 0x029c
    USAGE_MINIMUM(1),   0x01,       // usage minimum: 1
    USAGE_MAXIMUM(2),   0x9c, 0x02, // usage maximum: 0x029c
    REPORT_SIZE(1),     0x10,       // report size: 16
    REPORT_COUNT(1),    0x01,       // report count: 1
    HIDINPUT(1),        0x00,       // input: data, array, abs
    END_COLLECTION(0)
  };

  hid->reportMap((uint8_t*)report, sizeof(report));
  hid->startServices();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->start();
  hid->setBatteryLevel(7);

  ESP_LOGD(LOG_TAG, "Advertising started!");
  delay(portMAX_DELAY);
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello Atreus");

  Serial.println("Turning off WiFi...");
  WiFi.mode(WIFI_MODE_NULL);

  Serial.println("Configuring GPIO...");
  for (int row = 0; row < ROWS; row++) {
    int pin = row_pins[row];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
  for (int column = 0; column < COLUMNS; column++) {
    int pin = column_pins[column];
    pinMode(pin, INPUT_PULLUP);
  }

  xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);
}

void loop() {
  //copy from keys_pressed to keys_pressed_old for comparison later
  memcpy(keys_pressed_old, keys_pressed, ROWS * COLUMNS);

  //read key matrix
  for (int row = 0; row < ROWS; row++) {
    readRow(row);

    for (int column = 0; column < COLUMNS; column++) {
      keys_pressed[(row * COLUMNS) + column] = (digitalRead(column_pins[column]) == LOW);
    }
  }

  /*Serial.printf("\n\n\n");
  for (int row = 0; row < ROWS; row++) {
    for (int column = 0; column < COLUMNS; column++) {
      if (keys_pressed[(row * COLUMNS) + column]) {
        Serial.printf("D");
      } else {
        Serial.printf("U");
      }
    }
    Serial.printf("\n");
  }*/

  //check changes
  changesToSend = false;

  for (int row = 0; row < ROWS; row++) {
    for (int column = 0; column < COLUMNS; column++) {

      if (keys_pressed[(row * COLUMNS) + column] && !keys_pressed_old[(row * COLUMNS) + column]) {
        // KEY_DOWN
        Serial.printf("KEY_DOWN row %i column %i\n", row, column);

        addCurrentKey((row * COLUMNS) + column);
        changesToSend = true;

      } else if (!keys_pressed[(row * COLUMNS) + column] && keys_pressed_old[(row * COLUMNS) + column]) {
        // KEY_UP
        Serial.printf("KEY_UP row %i column %i\n", row, column);

        removeCurrentKey((row * COLUMNS) + column);
        changesToSend = true;
      }
    }
  }

  if (changesToSend) {
    resetCurrentSentKeys();
    Serial.printf("\n\n\n");

    int keyCodesSentIndex = 0;
    uint16_t consumerKeyCode = 0;

    for (int i = 0; i < ROWS * COLUMNS; i++) {
      if (keyCodes[i] != 0) {
        uint8_t keyCodeType = keyCodes[i] >> 8;

        if (keyCodeType == 0x00) { // normal key code -> send as is
          keyCodesSent[keyCodesSentIndex] = keyCodes[i];
          ++keyCodesSentIndex;

        } else if (keyCodeType == MOD) { // normal modifier -> send as modifier
          modifiersSent |= keyCodes[i];

        } else if (keyCodeType == 0x02) { // key code with modifier (shift) -> send modifier and key code
          modifiersSent |= (keyCodes[i] >> 8);
          keyCodesSent[keyCodesSentIndex] = keyCodes[i];
          ++keyCodesSentIndex;
        } else if (keyCodeType == CONSUMER) {
          consumerKeyCode = 0xFF & keyCodes[i];
        }
      }
    }

    uint8_t msg[] = {modifiersSent, 0, keyCodesSent[0],keyCodesSent[1],keyCodesSent[2],keyCodesSent[3],keyCodesSent[4],keyCodesSent[5]};

    Serial.printf("modifiers sent: %02x\n", modifiersSent);
    Serial.printf("keys sent: %i %i %i %i %i %i\n", keyCodesSent[0],keyCodesSent[1],keyCodesSent[2],keyCodesSent[3],keyCodesSent[4],keyCodesSent[5]);
    Serial.printf("consumerKeyCode: %i\n", consumerKeyCode);

    input->setValue(msg,sizeof(msg));
    input->notify();
    
    uint8_t msgConsumer[] = {consumerKeyCode, consumerKeyCode >> 8};
    Serial.printf("consumer report: %02x %02x\n", msgConsumer[0], msgConsumer[1]);

    inputConsumer->setValue(msgConsumer, sizeof(msgConsumer));
    inputConsumer->notify();
  }

  delay(1);
}

void readRow(int rowToRead) {
  for (int row = 0; row < ROWS; row++) {
    digitalWrite(row_pins[row], row != rowToRead); // LOW for rowToRead, HIGH for other rows
  }
  delayMicroseconds(100);
}

void resetCurrentSentKeys() {
  modifiersSent = 0x00;
  memset(keyCodesSent, 0, 6);
}

void addCurrentKey(uint8_t keyIndex) {
  Serial.printf("keyIndex=%i layer=%i\nkeyNr=%i\n", keyIndex, layer, keyNr);

  if (keyNr >= 6) {
    Serial.printf("Already sending 6 key codes.\n");
    return;
  }

  uint16_t key = keyIndexToKey(keyIndex);

  if (key == MOD_L1) {
    layer = 1;
  }

  if (key == 0) { // no key code mapped
    return;
  }

  uint8_t keyCodeType = (key >> 8);

  if (keyCodeType == 0x00       // normal key code
      || keyCodeType == 0x02) { // key code with modifier
    ++keyNr;
  } // else -> only modifier -> no increase in key codes

  keyCodes[keyIndex] = key;
}

void removeCurrentKey(uint8_t keyIndex) {
  uint16_t key = keyIndexToKey(keyIndex);

  if (key == MOD_L1) {
    layer = 0;
  }

  if (keyCodes[keyIndex] != 0) {
    uint8_t keyCodeType = (keyCodes[keyIndex] >> 8);

    if (keyCodeType == 0x00       // normal key code
        || keyCodeType == 0x02) { // key code with modifier
      --keyNr;
    } // else -> only modifier -> no decrease in key codes

    keyCodes[keyIndex] = 0;
  }
}

uint16_t keyIndexToKey(uint8_t keyIndex) {
  if (layer == 0) {
    return layer0[keyIndex];
  } else if (layer == 1) {
    return layer1[keyIndex];
  }

  return 0;
}