# fcitx5-hazkey
Hazkey input method for fcitx5

[AzooKeyKanaKanjiConverter](https://github.com/ensan-hcl/AzooKeyKanaKanjiConverter)を利用したIMEです。

## インストール方法
以下をインストールする必要があります。
  - fcitx5 >= 5.0.4
  - swift >= 5.10
  - cmake
  - pkgconf 
  - gettext

### 1. リポジトリのクローン
```sh
$ git clone https://github.com/7ka-Hiira/fcitx5-hazkey.git
```

### 2. インストールスクリプトの実行
```
$ cd fcitx5-hazkey
$ INSTALL_PREFIX=/usr ./build_install.sh
```

### 3. fcitx5 の設定
fcitx5-configtool を起動し、右のリストからhazkey を選択し、左矢印ボタンで追加してください。
表示されない場合は、以下のコマンドでfcitx5 を再起動してください。
```sh
$ fcitx5 -rd
```

## ライセンス
[MIT License](./LICENSE)
