# fcitx5-hazkey
Hazkey input method for fcitx5

[AzooKeyKanaKanjiConverter](https://github.com/ensan-hcl/AzooKeyKanaKanjiConverter)を利用したIMEです

## Installation

### Ubuntu/Debian系
[リリースページ](https://github.com/7ka-Hiira/fcitx5-hazkey/releases/latest)からdebファイルをダウンロードしてインストールしてください
```sh
$ apt install ./fcitx5-hazkey_VERSION_ARCH.deb # VERSIONとARCHはダウンロードしたファイル名に合わせてください
```

### Arch Linux系
AURからインストールできます

- binパッケージ(推奨)
```sh
$ yay -S fcitx5-hazkey-bin # yayの場合
```

- 通常パッケージ
```sh
$ yay -S fcitx5-hazkey  # yayの場合
```

## Build from source
以下をインストールする必要があります
  - fcitx5 development headers >= 5.0.4
  - swift >= 5.10
  - cmake >= 3.21
  - vulkan development headers
  - gettext

### 1. リポジトリのクローン
```sh
$ git clone https://github.com/7ka-Hiira/fcitx5-hazkey.git
```

### 2. ビルド
error: unrecognized command-line option のようなエラーが出た場合は、もう一度実行してください
```sh
$ cd fcitx5-hazkey
$ mkdir build
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/usr . # run twice if error occurs
$ make
$ sudo make install
```

### 3. fcitx5 の設定
fcitx5-configtool を起動し、右のリストからhazkey を選択し、左矢印ボタンで追加します
表示されない場合は、以下のコマンドでfcitx5 を再起動してください
```sh
$ fcitx5 -rd
```

## Zenzaiの設定
[Zenzai setup](./docs/zenzai.md)

## ライセンス
[MIT License](./LICENSE)
