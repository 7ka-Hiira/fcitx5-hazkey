# fcitx5-hazkey

Hazkey input method for fcitx5

[AzooKeyKanaKanjiConverter](https://github.com/ensan-hcl/AzooKeyKanaKanjiConverter)を利用したIMEです

## ホームページ

[https://hazkey.hiira.dev](https://hazkey.hiira.dev)

## ドキュメント

[https://hazkey.hiira.dev/docs](https://hazkey.hiira.dev/docs)

## インストール

[インストールガイド](https://hazkey.hiira.dev/docs/install)

現在AURと[debianパッケージ](https://github.com/7ka-Hiira/fcitx5-hazkey/releases/latest)が利用できます。

## ビルド

詳細は[ドキュメントのビルドページを参照してください](https://hazkey.hiira.dev/docs/development/build)。

### 依存関係

- Swift >= 6.1
- fcitx5 >= 5.0.4
- Qt >= 6.2
- CMake >= 3.21 (4.x以降推奨)
- Protobuf >= 3.12 (22.x以降推奨)
- Ninja
- Gettext

### ビルド・インストール手順

ninjaを利用します。

```sh
git clone --recursive https://github.com/7ka-Hiira/fcitx5-hazkey.git
cd fcitx5-hazkey
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -G Ninja ..
ninja
sudo ninja install
```

## ライセンス

[MIT License](./LICENSE)
