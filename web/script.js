const SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const CHAR_UUID_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";

let characteristicRX;
let currentCmd = ""; 

async function connectBLE() {
  try {
    document.getElementById('status').innerText = "デバイスを探しています...";
    const device = await navigator.bluetooth.requestDevice({
      filters: [{ name: 'ESP32_BumperCar' }],
      optionalServices: [SERVICE_UUID]
    });

    device.addEventListener('gattserverdisconnected', () => {
      document.getElementById('status').innerText = "切断されました (テストモード)";
      document.getElementById('status').style.color = "red";
      characteristicRX = null;
    });
    const server = await device.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);
    characteristicRX = await service.getCharacteristic(CHAR_UUID_RX);

    document.getElementById('status').innerText = "接続完了！ (ESP32_BumperCar)";
    document.getElementById('status').style.color = "green";
  } catch (error) {
    console.error(error);
    document.getElementById('status').innerText = "接続エラー: " + error.message;
  }
}

// 画面の表示を更新する専用の関数
function updateUI(cmd) {
  const display = document.getElementById('actionDisplay');
  
  // 一旦すべてのボタンのアクティブ状態を解除
  document.querySelectorAll('.btn').forEach(b => b.classList.remove('active'));

  // コマンドに応じて表示とボタンの色を変更
  if (cmd === 'forward') {
    display.innerText = '⬆️ 前進';
    document.getElementById('btn-w')?.classList.add('active');
  } else if (cmd === 'back') {
    display.innerText = '⬇️ 後退';
    document.getElementById('btn-s')?.classList.add('active');
  } else if (cmd === 'left') {
    display.innerText = '⬅️ 左旋回';
    document.getElementById('btn-a')?.classList.add('active');
  } else if (cmd === 'right') {
    display.innerText = '➡️ 右旋回';
    document.getElementById('btn-d')?.classList.add('active');
  } else if (cmd === 'brake') {
    display.innerText = '⏹️ 停止中';
  }
}

// コマンド処理のメイン関数
async function sendCmd(cmd) {
  // 同じコマンドの連続発火を防ぐ
  if (cmd === currentCmd && cmd !== 'brake') return;
  currentCmd = cmd;

  // 1. Bluetooth接続の有無に関わらず、まずは画面のUIを更新する
  updateUI(cmd);

  // 2. Bluetoothが接続されている場合のみ、ESP32へデータを送信する
  if (characteristicRX) {
    try {
      const encoder = new TextEncoder();
      await characteristicRX.writeValue(encoder.encode(cmd));
      console.log("BLE送信完了:", cmd);
    } catch (error) {
      console.error("BLE送信エラー:", error);
    }
  } else {
    // 未接続時はコンソールにログだけ出す
    console.log("テスト動作 (未接続):", cmd);
  }
}

// キーボードイベント
window.addEventListener('keydown', (e) => {
  if (e.repeat) return; 
  switch(e.key.toLowerCase()) {
    case 'w': sendCmd('forward'); break;
    case 's': sendCmd('back'); break;
    case 'a': sendCmd('left'); break;
    case 'd': sendCmd('right'); break;
  }
});

window.addEventListener('keyup', (e) => {
  const k = e.key.toLowerCase();
  if (['w', 'a', 's', 'd'].includes(k)) {
    sendCmd('brake');
  }
});