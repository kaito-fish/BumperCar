# Flybar 12V GLOW BUMPER CAR — ESP32 Web Controller

**Flybar 12V GLOW BUMPER CAR** をESP32でハックし、Web Bluetooth API を利用してウェブブラウザから直接遠隔操作できるようにしたプロジェクトです。
専用アプリのインストールは不要で、PC やスマートフォンのブラウザからそのまま操縦できます。

## 特徴

- **Web Bluetooth API 対応** — Chrome / Edge 等のブラウザから直接 ESP32 と BLE 通信
- **省電力・省メモリ** — `NimBLE-Arduino` を採用し、通常の BLE ライブラリより大幅に省リソース
- **直感的な操作 UI** — 画面ボタンのタップ / クリック、またはキーボード `W` `A` `S` `D` で操作
- **安全設計** — BLE 切断時に ESP32 側で自動ブレーキ → 再アドバタイズを実行し、暴走を防止
- **機密情報の分離** — `.env` + `load_env.py` により UUID やデバイス名をハードコードせず管理

## ディレクトリ構成

```text
.
├── src/
│   └── main.cpp           # ESP32 ファームウェア
├── web/
│   ├── BPcontroller.html  # Web コントローラー UI
│   ├── script.js          # Web Bluetooth・UI 制御ロジック
│   └── style.css          # スタイルシート
├── platformio.ini         # PlatformIO 設定
├── load_env.py            # .env → ビルドフラグ変換スクリプト
└── .env.example           # 環境変数のひな形
```

## 配線（ESP32 DevKit → HY2204M-A-12V）

HY2204M-A-12Vへ入っている **細い信号線** は、通常時 約 2.7 V・操作時に 0 V へ落ちる **アクティブ LOW 入力** です（PWM ではない）。
ESP32 からは信号線へ電圧を出さず、**必要なときだけ GPIO を LOW にして GND へ落とす** ことで、物理スイッチの押下を再現します。

### ピンアサイン

| 信号線の色 | ESP32 GPIO | 役割 |
| :--- | :---: | :--- |
| 🟠 橙 (Orange) | `12` | 右前 (RF) |
| 🔵 青 (Blue) | `14` | 右後 (RR) |
| 🟢 緑 (Green) | `27` | 左前 (LF) |
| ⚪ 白 (White) | `26` | 左後 (LR) |

### コマンドとモーター状態

| コマンド | RF (12) | RR (14) | LF (27) | LR (26) |
| :---: | :---: | :---: | :---: | :---: |
| `forward` | HIGH | LOW | HIGH | LOW |
| `back` | LOW | HIGH | LOW | HIGH |
| `right` | LOW | HIGH | HIGH | LOW |
| `left` | HIGH | LOW | LOW | HIGH |
| `brake` | LOW | LOW | LOW | LOW |

## 使い方

### 1. ファームウェアの書き込み（ESP32）

1. **VSCode + PlatformIO** 拡張機能をインストール
2. `.env.example` をコピーして `.env` を作成
3. `.env` に `BLE_DEVICE_NAME` や UUID を記入（デフォルト値でも動作可）
4. PlatformIO でプロジェクトを開き、ESP32 へビルド＆書き込み（Upload）

### 2. Web コントローラーの操作

> Web Bluetooth はセキュアコンテキスト（**HTTPS** / **localhost** / **file://**）でのみ動作します。フォルダから HTML を直接開いても OK です。

1. `web/BPcontroller.html` をブラウザで直接開く（フォルダからダブルクリックでも OK）
2. **「Bluetooth接続」** ボタンをクリック
3. ポップアップから `ESP32_BumperCar` を選択してペアリング
4. 画面ボタンまたはキーボード `W` `A` `S` `D` で操作

## 使用技術

| カテゴリ | 技術 |
| :--- | :--- |
| マイコン | ESP-WROOM-32 |
| フレームワーク | Arduino (PlatformIO) |
| BLE ライブラリ | [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) |
| フロントエンド | HTML / CSS / JavaScript (Web Bluetooth API) |
