const SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const CHAR_UUID_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";

let characteristicRX = null;
let currentCmd = "";
let bleDevice = null;

// --- コマンド → UI 表示マッピング ---
const CMD_MAP = {
  forward: { label: "⬆️ 前進",  btnId: "btn-w" },
  back:    { label: "⬇️ 後退",  btnId: "btn-s" },
  left:    { label: "⬅️ 左旋回", btnId: "btn-a" },
  right:   { label: "➡️ 右旋回", btnId: "btn-d" },
  brake:   { label: "⏹️ 停止中", btnId: null    },
};

// =========================================================
//  BLE 接続
// =========================================================
async function connectBLE() {
  try {
    document.getElementById("status").innerText = "デバイスを探しています...";

    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ name: "ESP32_BumperCar" }],
      optionalServices: [SERVICE_UUID],
    });

    bleDevice.addEventListener("gattserverdisconnected", onDisconnected);
    await establishConnection();
  } catch (error) {
    console.error(error);
    document.getElementById("status").innerText = "接続エラー: " + error.message;
    document.getElementById("status").style.color = "red";
  }
}

async function establishConnection() {
  const server = await bleDevice.gatt.connect();
  const service = await server.getPrimaryService(SERVICE_UUID);
  characteristicRX = await service.getCharacteristic(CHAR_UUID_RX);

  document.getElementById("status").innerText = "接続完了！ (ESP32_BumperCar)";
  document.getElementById("status").style.color = "green";
}

// --- 切断時: 自動再接続を試行 ---
function onDisconnected() {
  characteristicRX = null;
  document.getElementById("status").innerText = "切断されました — 3秒後に再接続を試みます…";
  document.getElementById("status").style.color = "orange";
  updateUI("brake");

  setTimeout(async () => {
    if (!bleDevice) return;
    try {
      console.log("再接続を試行中...");
      await establishConnection();
    } catch (err) {
      console.error("再接続失敗:", err);
      document.getElementById("status").innerText = "再接続失敗 (テストモード)";
      document.getElementById("status").style.color = "red";
    }
  }, 3000);
}

// =========================================================
//  UI 更新
// =========================================================
function updateUI(cmd) {
  const display = document.getElementById("actionDisplay");

  // 全ボタンのアクティブ状態を解除
  document.querySelectorAll(".btn").forEach((b) => b.classList.remove("active"));

  const entry = CMD_MAP[cmd];
  if (entry) {
    display.innerText = entry.label;
    if (entry.btnId) {
      document.getElementById(entry.btnId)?.classList.add("active");
    }
  }
}

// =========================================================
//  コマンド送信
// =========================================================
async function sendCmd(cmd) {
  // 同じコマンドの連続発火を防ぐ (brake は常に通す)
  if (cmd === currentCmd && cmd !== "brake") return;
  currentCmd = cmd;

  // 1. UI を即時更新
  updateUI(cmd);

  // 2. BLE 接続中のみ ESP32 へ送信
  if (characteristicRX) {
    try {
      const encoder = new TextEncoder();
      await characteristicRX.writeValue(encoder.encode(cmd));
      console.log("BLE送信完了:", cmd);
    } catch (error) {
      console.error("BLE送信エラー:", error);
    }
  } else {
    console.log("テスト動作 (未接続):", cmd);
  }
}

// =========================================================
//  キーボードイベント
// =========================================================
const KEY_CMD = { w: "forward", s: "back", a: "left", d: "right" };

window.addEventListener("keydown", (e) => {
  if (e.repeat) return;
  const cmd = KEY_CMD[e.key.toLowerCase()];
  if (cmd) sendCmd(cmd);
});

window.addEventListener("keyup", (e) => {
  if (KEY_CMD[e.key.toLowerCase()]) {
    sendCmd("brake");
  }
});