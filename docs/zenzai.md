# Zenzaiを使用したかな漢字変換

Zenzaiを利用したニューラル漢字変換を使用することができます。

**この機能は開発中です。不安定な動作をする場合があります。また、環境によっては変換速度が著しく低下する場合があります。**

Zenzaiについての詳細はこちらを参照してください
https://github.com/ensan-hcl/AzooKeyKanaKanjiConverter/blob/develop/Docs/zenzai.md

# Vulkanのインストール

Zenzaiを使用するためには、Vulkanドライバが必要です。ご利用の環境に合わせてインストールしてください。

## Arch Linuxの場合
GPUに応じたvulkan-driverを提供するパッケージが必要です。以下は一例です。
- NVIDIA GPU: nvida-utils, vulkan-nouveau
- AMD GPU: vulkan-radeon, amdvlk, vulkan-amdgpu-pro
- Intel GPU: vulkan-intel
- 詳細は[ArchWiki 英語版](https://wiki.archlinux.org/title/Vulkan#Installation) [日本語版](https://wiki.archlinux.jp/index.php/Vulkan#.E3.82.A4.E3.83.B3.E3.82.B9.E3.83.88.E3.83.BC.E3.83.AB)を参照してください

## Ubuntu Desktopの場合
通常プリインストールされています。

## その他の環境の場合
ディストリビューションのドキュメントやWiki等を参照してください。

# 有効化

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
