# Zenzaiを使用したかな漢字変換

Zenzaiを利用したニューラル漢字変換を使用することができます。

**この機能は開発中です。不安定な動作をする場合があります。また、環境によっては変換速度が著しく低下する場合があります。**

Zenzaiについての詳細はこちらを参照してください
https://github.com/ensan-hcl/AzooKeyKanaKanjiConverter/blob/develop/Docs/zenzai.md

# Vulkanのインストール

Zenzaiを使用するためには、Vulkanドライバが必要です。ご利用の環境に合わせてインストールしてください。

例えば、Arch Linuxでは、GPUに応じたvulkan-driverを提供するパッケージが必要です。

Ubuntu Desktopには通常プリインストールされています。


# 有効化

Readmeの手順に従って、fcitx5-hazkeyをインストール・設定します。

## 設定

fcitx5-configtoolを起動し、左のリストからhazkeyを選択し、中央列の下から2つ目のボタンで設定画面を開きます。

Enable Zenzai (Experimental) / Zenzaiを有効化 (実験的) にチェックを入れると、Zenzaiを利用したかな漢字変換が有効になります。

Inference Limit / 推論制限 には、推論の最大値を設定します。大きくするほど変換精度が向上しますが、変換速度が低下します。

## 確認

あずーきー　と入力したときの第一変換候補が「アズーキー」であれば、Zenzaiが有効になっています。Zenzaiが無効の場合は「azooKey」になります。
