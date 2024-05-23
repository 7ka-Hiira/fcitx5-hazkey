// swift-tools-version: 5.10
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "azookey-kkc",
    products: [
        // Products define the executables and libraries a package produces, making them visible to other packages.
        .library(
            name: "azookey-kkc",
            type: .dynamic,
            targets: ["azookey-kkc"]),
    ],
    dependencies: [
        .package(url: "https://github.com/ensan-hcl/AzooKeyKanaKanjiConverter", branch: "develop"),
    ],
    targets: [
        // Targets are the basic building blocks of a package, defining a module or a test suite.
        // Targets can depend on other targets in this package and products from dependencies.
        //.systemLibrary(
        //    name: "fcitx5",
        //    pkgConfig: "fcitx5"
        //),
        .target(
            name: "azookey-kkc",
            dependencies: [
                .product(name: "KanaKanjiConverterModuleWithDefaultDictionary", package: "AzooKeyKanaKanjiConverter")
            ]
        ),
        .testTarget(
            name: "azookey-kkc-Tests",
            dependencies: [
                "azookey-kkc",
                //.product(name: "KanaKanjiConverterModuleWithDefaultDictionary", package: "AzooKeyKanaKanjiConverter")
            ]
        ),
    ]
)
