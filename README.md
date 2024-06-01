# fcitx5-hazukey
Hazukey input method for fcitx5

## インストール方法
ビルドには以下のパッケージが必要です
  - fcitx5
  - swift
  - cmake
  - pkgconf 

### 1. リポジトリのクローン
```sh
$ git clone https://github.com/7ka-Hiira/fcitx5-hazukey.git
```

### 2. スクリプトの実行
```
$ cd fcitx5-hazukey
$ INSTALL_PREFIX=/usr ./build_install.sh
```

### 3. fcitx5 の設定
fcitx5-configtool を起動し、右のリストからhazukey を選択し、左矢印ボタンで追加してください。
表示されない場合は、以下のコマンドでfcitx5 を再起動してください。
```sh
$ fcitx5 -rd
```

## ライセンス
[MIT License](./LICENSE)