#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ESP32のピン定義
const int RF = 12; // 橙：右前
const int RR = 14; // 青：右後
const int LF = 27; // 緑：左前
const int LR = 26; // 白：左後

// BLEのUUID（.envから読み込み）
#define SERVICE_UUID           BLE_SERVICE_UUID
#define CHARACTERISTIC_UUID_RX BLE_CHARACTERISTIC_UUID_RX

// プロトタイプ宣言（下にある関数を先にコンパイラに知らせる）
void brake();
void handle_message(String message);

// --- BLE受信コールバック ---
// （※setup()より上に移動し、String型に修正しました）
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      // ESP32 v3.x では getValue() が std::string を返すため、String に変換
      String rxValue = String(pCharacteristic->getValue().c_str());
      
      if (rxValue.length() > 0) {
        Serial.println("=============================");
        Serial.print("Raw BLE Data : [");
        Serial.print(rxValue);
        Serial.print("] (Length: ");
        Serial.print(rxValue.length());
        Serial.println(")");

        rxValue.trim(); // ゴミ文字（改行や空白）を削除

        Serial.print("Trimmed Data : [");
        Serial.print(rxValue);
        Serial.print("] (Length: ");
        Serial.print(rxValue.length());
        Serial.println(")");

        // 制御関数に渡す
        handle_message(rxValue);
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Bumper Car System Booting ---");
  
  pinMode(RF, OUTPUT); pinMode(RR, OUTPUT);
  pinMode(LF, OUTPUT); pinMode(LR, OUTPUT);
  brake();

  BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  
  // クラス定義を上に移動したので、ここでエラーになりません
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  
  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("Waiting for Web Bluetooth connection...");
}

void loop() {
  delay(100);
}

// --- モーター制御関数 ---
void brake() {
  digitalWrite(RF, LOW); digitalWrite(RR, LOW);
  digitalWrite(LF, LOW); digitalWrite(LR, LOW);
  delay(10);
}

void handle_message(String message) {
  Serial.print("-> Executing Command: ");
  Serial.println(message);

  if (message == "forward") {
    digitalWrite(RR, LOW); digitalWrite(LR, LOW); delay(5);
    digitalWrite(RF, HIGH); digitalWrite(LF, HIGH);
  }
  else if (message == "back") {
    digitalWrite(RF, LOW); digitalWrite(LF, LOW); delay(5);
    digitalWrite(RR, HIGH); digitalWrite(LR, HIGH);
  }
  else if (message == "right") {
    digitalWrite(RF, LOW); digitalWrite(LR, LOW); delay(5);
    digitalWrite(RR, HIGH); digitalWrite(LF, HIGH);
  }
  else if (message == "left") {
    digitalWrite(RR, LOW); digitalWrite(LF, LOW); delay(5);
    digitalWrite(RF, HIGH); digitalWrite(LR, HIGH);
  }
  else if (message == "brake") {
    brake();
  }
  else {
    Serial.println("-> [Warning] Unknown command ignored.");
  }
}