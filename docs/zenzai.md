# Zenzaiを使用したかな漢字変換

Zenzaiを利用したニューラル漢字変換を使用することができます。

**この機能は開発中です。不安定な動作をする場合があります。また、環境によっては変換速度が著しく低下する場合があります。**

Zenzaiについての詳細はこちらを参照してください
https://github.com/ensan-hcl/AzooKeyKanaKanjiConverter/blob/develop/Docs/zenzai.md

# 機能

## 文脈変換
設定でZenzai contextual conversionにチェックを入れると、文脈を考慮した変換を有効にすることができます。
例えば以下のような2つの文があるとします。
```
1. 私は葉月です。
2. 私はハヅキです。
```
この時、それぞれの分に続けて「あのこもはづきです」と入力すると、文脈によってこのような変換が行われます。
```
1. 私は葉月です。あの子も葉月です。
2. 私はハヅキです。あの子もハヅキです。
```
*文脈を取得できないアプリケーションが多く、その場合は間違った文脈で変換され精度が低下するため、デフォルトでは無効になっています。*
2024年10月現在の対応状況の一例を以下に示します。
- 対応: LibreOffice, Kate, Firefox, gedit
- 非対応: Chromium系ブラウザ (Edge, Vialdiなど), Electrn製 (VSCodeなど), Zed, ほとんどのターミナルエミュレーター

## プロフィール
設定でZenzai Profileを入力すると、プロフィールを考慮した変換を有効にすることができます。
これにより、ユーザーに関連のある単語の順位が少し上がります。最長25文字で、それ以上入力すると最後の25文字が使用されます。

# 有効化方法

## Vulkanのインストール

Zenzaiを使用するためには、Vulkanドライバが必要です。ご利用の環境に合わせてインストールしてください。

### Arch Linuxの場合
GPUに応じたvulkan-driverを提供するパッケージが必要です。以下は一例です。
- NVIDIA GPU: nvida-utils, vulkan-nouveau
- AMD GPU: vulkan-radeon, amdvlk, vulkan-amdgpu-pro
- Intel GPU: vulkan-intel
- 詳細は[ArchWiki 英語版](https://wiki.archlinux.org/title/Vulkan#Installation) [日本語版](https://wiki.archlinux.jp/index.php/Vulkan#.E3.82.A4.E3.83.B3.E3.82.B9.E3.83.88.E3.83.BC.E3.83.AB)を参照してください

### Ubuntu Desktopの場合
通常プリインストールされています。

### その他の環境の場合
ディストリビューションのドキュメントやWiki等を参照してください。

Readmeの手順に従って、fcitx5-hazkeyをインストール・設定します。

## 設定

fcitx5-configtoolを起動、左のリストからhazkeyを選択し、中央列の下から2つ目のボタンで設定画面を開きます。

Enable Zenzai (Experimental) / Zenzaiを有効化 (実験的) にチェックを入れると、Zenzaiを利用したかな漢字変換が有効になります。

Inference Limit / 推論制限 には、推論の最大値を設定します。大きくするほど変換精度が向上しますが、変換速度が低下します。

## 確認

fcitx5 -rdを実行し、hazkeyで入力する際に、キーを押すたびに以下のようなログが出力されていれば、Zenzaiが有効になっています。
```
update constraint: PrefixConstraint(constraint: "あずきア", hasEOS: false)
Constrained draft modeling 0.027788996696472168
Evaluate Candidate(text: "あずきアイス", value: -33.1063, correspondingCount: 9, lastMid: 64, data: [(ruby: アズキ, word: あずき, cid: (1285, 1285), mid: 437, value: -12.7852+0.0=-12.7852, metadata: (isLearned: false, isFromUserDictionary: false)), (ruby: アイス, word: アイス, cid: (1285, 1285), mid: 64, value: -10.0484+0.0=-10.0484, metadata: (isLearned: false, isFromUserDictionary: false))], actions: [], inputable: true)
pos max: 16
passed: -0.46214294
```
