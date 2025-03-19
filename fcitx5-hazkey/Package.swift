// swift-tools-version: 6.0
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
  name: "hazkey",
  products: [
    // Products define the executables and libraries a package produces, making them visible to other packages.
    .library(
      name: "hazkey",
      type: .static,
      targets: ["fcitx5Hazkey"])
  ],
  dependencies: [
    .package(
      url: "https://github.com/7ka-Hiira/AzooKeyKanaKanjiConverter",
      branch: "20dcb75")
  ],
  targets: [
    // Targets are the basic building blocks of a package, defining a module or a test suite.
    // Targets can depend on other targets in this package and products from dependencies.
    .target(
      name: "fcitx5Hazkey",
      dependencies: [
        .product(
          name: "KanaKanjiConverterModule",
          package: "AzooKeyKanaKanjiConverter"),
        .product(
          name: "SwiftUtils",
          package: "AzooKeyKanaKanjiConverter"),
      ]
    ),
    .testTarget(
      name: "fcitx5Hazkey-Tests",
      dependencies: [
        "fcitx5Hazkey"
      ]
    ),
  ]
)
