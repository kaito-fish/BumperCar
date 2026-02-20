#include <Arduino.h>
#include <NimBLEDevice.h>

// =========================================================
//  ESP32 バンパーカー ファームウェア
//  - NimBLE v2.x による省メモリ BLE 通信
//  - 切断時の自動ブレーキ & 再アドバタイズ
//  - 構造体マップ方式のコマンド処理
// =========================================================

// --- ピン定義 ---
const int RF = 12;  // 橙：右前
const int RR = 14;  // 青：右後
const int LF = 27;  // 緑：左前
const int LR = 26;  // 白：左後

// --- BLE UUID (.env から読み込み) ---
#define SERVICE_UUID           BLE_SERVICE_UUID
#define CHARACTERISTIC_UUID_RX BLE_CHARACTERISTIC_UUID_RX

// --- モーターコマンドの構造体定義 ---
struct MotorCommand {
  const char* name;
  uint8_t rfState, rrState, lfState, lrState;
};

// コマンド → ピン状態のマッピングテーブル
static const MotorCommand COMMANDS[] = {
  // name       RF    RR    LF    LR
  { "forward",  HIGH, LOW,  HIGH, LOW  },
  { "back",     LOW,  HIGH, LOW,  HIGH },
  { "right",    LOW,  HIGH, HIGH, LOW  },
  { "left",     HIGH, LOW,  LOW,  HIGH },
};
static const int NUM_COMMANDS = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

// --- グローバル変数 ---
static NimBLEServer* pServer = nullptr;

// --- プロトタイプ宣言 ---
void brake();
void handle_message(const String& message);

// --- BLE サーバーコールバック (接続 / 切断) ---
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    Serial.printf("[BLE] Client connected (addr: %s)\n",
                  connInfo.getAddress().toString().c_str());
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    Serial.printf("[BLE] Client disconnected (reason=%d) -> brake + re-advertise\n", reason);
    brake();
    NimBLEDevice::startAdvertising();
  }
};

// --- BLE 受信コールバック ---
class RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    std::string raw = pCharacteristic->getValue();
    if (raw.empty()) return;

    String rxValue = String(raw.c_str());
    Serial.printf("[BLE] Raw: [%s] (len=%d)\n", rxValue.c_str(), rxValue.length());

    rxValue.trim();
    Serial.printf("[BLE] Trimmed: [%s]\n", rxValue.c_str());

    handle_message(rxValue);
  }
};

// =========================================================
//  setup
// =========================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Bumper Car System Booting ---");

  // モーターピン初期化
  const int pins[] = { RF, RR, LF, LR };
  for (int pin : pins) {
    pinMode(pin, OUTPUT);
  }
  brake();

  // NimBLE 初期化
  NimBLEDevice::init(BLE_DEVICE_NAME);

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* pService = pServer->createService(SERVICE_UUID);

  NimBLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    NIMBLE_PROPERTY::WRITE
  );
  pRxCharacteristic->setCallbacks(new RxCallbacks());

  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("Waiting for Web Bluetooth connection...");
}

// =========================================================
//  loop
// =========================================================
void loop() {
  delay(100);
}

// =========================================================
//  モーター制御
// =========================================================
void brake() {
  digitalWrite(RF, LOW);
  digitalWrite(RR, LOW);
  digitalWrite(LF, LOW);
  digitalWrite(LR, LOW);
}

void handle_message(const String& message) {
  Serial.printf("-> Executing Command: %s\n", message.c_str());

  // マッピングテーブルから一致するコマンドを検索
  for (int i = 0; i < NUM_COMMANDS; i++) {
    if (message == COMMANDS[i].name) {
      // 安全のため先に全ピン LOW にしてからセット
      brake();
      delayMicroseconds(500);

      digitalWrite(RF, COMMANDS[i].rfState);
      digitalWrite(RR, COMMANDS[i].rrState);
      digitalWrite(LF, COMMANDS[i].lfState);
      digitalWrite(LR, COMMANDS[i].lrState);
      return;
    }
  }

  if (message == "brake") {
    brake();
    return;
  }

  Serial.println("-> [Warning] Unknown command ignored.");
}