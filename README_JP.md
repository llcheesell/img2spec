# img2spec - 画像からスペクトログラム音声へ

**English** → [README.md](README.md)

画像（PNG/JPG）をスペクトログラムとして解釈し、音声に変換します。Griffin-Lim アルゴリズムによる位相再構成と ISTFT で高品質な音声を合成します。

## スクリーンショット

### アプリケーション画面
![メインウィンドウ](docs/images/screenshot-main.png)
*画像プレビュー・周波数ガイド・パラメータ一式を持つメインウィンドウ*

### 出力結果
![スペクトログラム結果](docs/images/spectrogram-result.png)
*Adobe Audition で可視化した生成音声のスペクトログラム*

## 機能

- **クロスプラットフォーム**: Windows / macOS 対応
- **画像形式**: PNG, JPG（色は自動でグレースケールに変換）
- **音声出力**: WAV。以下を選択可能:
  - サンプルレート: 44.1kHz, 48kHz, 96kHz
  - ビット深度: 16bit PCM, 24bit PCM, 32bit Float
  - モノラル / ステレオ（L/R 複製）
- **DSP パラメータ**:
  - FFT サイズ: 1024, 2048, 4096
  - ホップサイズ: NFFT/2, NFFT/4, NFFT/8
  - 周波数スケール: リニア / 対数（20Hz–20kHz で設定可能）
  - 輝度マッピング: minDb, ガンマ補正
  - Griffin-Lim 反復回数（16–256）
  - 正規化、出力ゲイン、セーフティリミッター
- **操作性**:
  - 画像プレビュー上の周波数ガイド（対数モード時）
  - 再生時間のリアルタイム表示。**目標時間**の指定も可能（時間軸リサンプル）
  - **サウンドプレビュー**: エクスポート前に再生。再生位置ヘッダ（現在/合計）とプレイヘッド表示。「Stop Preview」で停止
  - 画像のドラッグ＆ドロップ
  - レンダリング中の詳細な進捗ダイアログ

## インストール

### ソースの取得

[Releases](https://github.com/YOUR_USERNAME/img2spec/releases) から最新のソースを取得するか、リポジトリをクローンしてください。

```bash
git clone https://github.com/YOUR_USERNAME/img2spec.git
cd img2spec
```

**注意**: macOS のコード署名の都合上、ビルド済みバイナリは提供していません。ソースからビルドしてください。

## 技術スタック

- **GUI**: Qt6
- **FFT**: kissfft (BSD)
- **音声 I/O**: libsndfile (PCM 各種対応)
- **画像読み込み**: stb_image (PNG/JPG)
- **ビルド**: CMake 3.20+

## ビルド

### macOS

```bash
brew install qt6 cmake
cd /path/to/img2spec
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6
cmake --build . --config Release
# 実行
./img2spec.app/Contents/MacOS/img2spec
```

### Windows

1. Qt6 と CMake、Visual Studio 2019 以降（C++）をインストール
2. 開発者コマンドプロンプトで:

```bash
cd C:\path\to\img2spec
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2019_64
cmake --build . --config Release
# 実行
Release\img2spec.exe
```

## 使い方

### クイックスタート

1. **起動**: 上記のとおりビルドして実行（macOS の例: `./build/img2spec.app/Contents/MacOS/img2spec`）
2. **画像を開く**: 「Open Image...」またはドラッグ＆ドロップ。色画像は自動でグレースケールに変換されます。
3. **再生時間**: パラメータ下に表示。「画像幅×ホップサイズ÷サンプルレート」で計算。**「Set target duration」**にチェックを入れると、指定秒数に時間リサンプルされます。
4. **パラメータ**: サンプルレート・FFT・ホップ・周波数スケール（対数推奨）・Min dB・ガンマ・Griffin-Lim 反復数などを調整。
5. **周波数ガイド**: 対数モード時、画像上に 50Hz〜15kHz などの目安が表示されます。
6. **サウンドプレビュー**: 「Preview」でその場で再生。再生ヘッダ（現在/合計）とプレイヘッドで位置を確認。「Stop Preview」で停止。
7. **レンダー**: 「Render & Export WAV...」で WAV を保存。進捗ダイアログでステップを確認できます。
8. **再生**: 出力 WAV を任意のプレイヤーや DAW で再生。

### 主なパラメータ

- **FFT Size**: 大きいほど周波数分解能が上がり処理は重くなります
- **Hop Size**: 小さいほど時間分解能が滑らかで、音声は長くなります
- **Frequency Scale**: リニア＝均一、対数＝低域を広く（聴感に近い）
- **Min dB / Gamma**: ダイナミックレンジと明るさ
- **Griffin-Lim Iterations**: 多いほど位相推定が安定（64 前後で十分なことが多い）
- **Set target duration**: チェック時、指定秒数に合わせて時間軸リサンプル

## 制限事項

- 推奨画像サイズの目安: 4096×4096 以下
- 32bit 出力は Float32 形式（Int32 ではありません）

## ライセンス

Qt6 (LGPL v3), kissfft (BSD), libsndfile (LGPL v2.1+), stb_image (Public Domain) を使用しています。

## トラブルシューティング

- **Qt6 が見つからない**: `cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6` で Qt6 のパスを指定
- **画像が表示されない**: コンソールに "Preview updated" が出ているか確認。対応形式は PNG/JPG
- **プレビューで音が出ない**: デフォルトの再生デバイスがあるか、サンプルレートやステレオ設定を変えて試す
- **WAV が無音**: 画像が暗すぎる、Min dB が低すぎる場合があります。ガンマを上げる・Min dB を -60 程度にして試す

詳細は [TROUBLESHOOTING.md](TROUBLESHOOTING.md) を参照してください。

## プロジェクト構成

```
img2spec/
├── app/          # メインウィンドウ・プレビューウィジェット
├── core/         # 画像読み込み・スペクトログラム・STFT・Griffin-Lim・WAV 出力
├── docs/         # スクリーンショット・アイコン・ICON_PROMPT
├── resources/    # Windows 用 .rc / .ico（任意）
├── CMakeLists.txt
├── README.md     # 英語
└── README_JP.md  # 日本語（本ファイル）
```

## Credits

llcheesell と Claude Code、Cursor による共同開発。
