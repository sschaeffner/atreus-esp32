#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include <driver/adc.h>
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
  KEY_ESC,  KEY_TAB,  MOD_GUI,  MOD_SHIFT,  KEY_BACKSPACE,    MOD_CTRL,   KEY_SPACE,  0,        KEY_MINUS,  KEY_SINGLE_APOSTR,  KEY_ENTER
};

bool keys_pressed_old[ROWS * COLUMNS] = { 0 };
bool keys_pressed[ROWS * COLUMNS] = { 0 };

bool changesToSend = false;
uint8_t modifiers_sent = 0x00;
uint8_t keys_pressed_sent[6] = { 0 };

BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;

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
  }

  void onDisconnect(BLEServer* pServer){
    connected = false;
    BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    desc->setNotifications(false);
  }
};

/*
 * This callback is connect with output report. In keyboard output report report special keys changes, like CAPSLOCK, NUMLOCK
 * We can add digital pins with LED to show status
 * bit 0 - NUM LOCK
 * bit 1 - CAPS LOCK
 * bit 2 - SCROLL LOCK
 */
 class MyOutputCallbacks : public BLECharacteristicCallbacks {
 void onWrite(BLECharacteristic* me){
    uint8_t* value = (uint8_t*)(me->getValue().c_str());
    ESP_LOGI(LOG_TAG, "special keys: %d", *value);
  }
};

void taskServer(void*){

    BLEDevice::init("sAtreus");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyCallbacks());

    hid = new BLEHIDDevice(pServer);
    input = hid->inputReport(1); // <-- input REPORTID from report map
    output = hid->outputReport(1); // <-- output REPORTID from report map

    output->setCallbacks(new MyOutputCallbacks());

    std::string name = "simon";
    hid->manufacturer()->setValue(name);

    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    hid->hidInfo(0x00,0x02);

  BLESecurity *pSecurity = new BLESecurity();
//  pSecurity->setKeySize();
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
      REPORT_SIZE(1),     0x01,       //   1 byte (Modifier)
      REPORT_COUNT(1),    0x08,
      HIDINPUT(1),           0x02,       //   Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position
      REPORT_COUNT(1),    0x01,       //   1 byte (Reserved)
      REPORT_SIZE(1),     0x08,
      HIDINPUT(1),           0x01,       //   Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position
      REPORT_COUNT(1),    0x06,       //   6 bytes (Keys)
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

        addCurrentKey(layer0[(row * COLUMNS) + column]);
        changesToSend = true;

      } else if (!keys_pressed[(row * COLUMNS) + column] && keys_pressed_old[(row * COLUMNS) + column]) {
        // KEY_UP
        Serial.printf("KEY_UP row %i column %i\n", row, column);

        removeCurrentKey(layer0[(row * COLUMNS) + column]);
        changesToSend = true;
      }
    }
  }

  if (changesToSend) {
    Serial.printf("\n\n\n");
    uint8_t msg[] = {modifiers_sent, 0, keys_pressed_sent[0], keys_pressed_sent[1],keys_pressed_sent[2],keys_pressed_sent[3],keys_pressed_sent[4],keys_pressed_sent[5]};

    Serial.printf("modifiers sent: %02x\n", modifiers_sent);
    Serial.printf("keys sent: %i %i %i %i %i %i\n", keys_pressed_sent[0], keys_pressed_sent[1],keys_pressed_sent[2],keys_pressed_sent[3],keys_pressed_sent[4],keys_pressed_sent[5]);

    input->setValue(msg,sizeof(msg));
    input->notify();
  }

  delay(1);
}

void readRow(int rowToRead) {
  for (int row = 0; row < ROWS; row++) {
    digitalWrite(row_pins[row], row != rowToRead); // LOW for rowToRead, HIGH for other rows
  }
  delayMicroseconds(100);
}

void resetCurrentKeys() {
  modifiers_sent = 0x00;
  memset(keys_pressed_sent, 0, 6);
}


void addCurrentKey(int key) {
  Serial.printf("key=%i\n", key);

  uint8_t upper = (key >> 8);
  if (upper == 0x00) {
    // normal key

    for (int i = 0; i < 6; i++) {
      if (keys_pressed_sent[i] == 0x00) {
        keys_pressed_sent[i] = key;
        break;
      }
    }

  } else if (upper == 0x01) {
    // modifier
    uint8_t lower = key;
    modifiers_sent |= lower;
  }
}

void removeCurrentKey(int key) {
  uint8_t upper = (key >> 8);
  if (upper == 0x00) {
    // normal key

    for (int i = 0; i < 6; i++) {
      if (keys_pressed_sent[i] == key) {
        keys_pressed_sent[i] = 0x00;
        break;
      }
    }

  } else if (upper == 0x01) {
    // modifier
    uint8_t lower = key;
    modifiers_sent &= ~lower;
  }
}