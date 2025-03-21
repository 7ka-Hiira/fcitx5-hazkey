# fcitx5-hazkey
Hazkey input method for fcitx5

[AzooKeyKanaKanjiConverter](https://github.com/ensan-hcl/AzooKeyKanaKanjiConverter)を利用したIMEです

## 基本操作
[Hazkey basic (作成中)](./docs/basic.md)

## Zenzaiの設定
ニューラル漢字変換システムZenzaiを使用する場合は、[Zenzai setup](./docs/zenzai.md) の手順に従って設定してください

## インストール

### Ubuntu/Debian系 (x86_64)
[リリースページ](https://github.com/7ka-Hiira/fcitx5-hazkey/releases/latest)からdebファイルをダウンロードし、以下のコマンドでインストールできます
```sh
$ sudo apt install ./fcitx5-hazkey_VERSION_ARCH.deb # VERSIONとARCHはダウンロードしたファイル名に合わせてください
```

### Arch Linux系
AURからインストールできます

- binパッケージ(おすすめ, x86_64のみ)
```sh
$ yay -S fcitx5-hazkey-bin # yayの場合
```

- 通常パッケージ
```sh
$ yay -S fcitx5-hazkey # yayの場合
```

## ソースからインストール
以下をインストールする必要があります
  - clang >= 5.0
  - fcitx5 development headers >= 5.0.4
  - swift >= 6.0
  - cmake >= 3.21
  - vulkan development headers
  - gettext

### 1. リポジトリのクローン
```sh
$ git clone https://github.com/7ka-Hiira/fcitx5-hazkey.git --recursive
```

### 2. ビルド

```sh
$ cd fcitx5-hazkey
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr .. # エラーが発生しますが、無視してもう一度実行します
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
$ make
$ sudo make install
```

### 3. fcitx5 の設定
fcitx5-configtool を起動し、右のリストからhazkey を選択し、左矢印ボタンで追加します

表示されない場合は、以下のコマンドでfcitx5 を再起動してください
```sh
$ fcitx5 -rd
```

## ライセンス
[MIT License](./LICENSE)
