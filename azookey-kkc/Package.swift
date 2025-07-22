// swift-tools-version: 6.1
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
  name: "hazkey",
  products: [
    // Products define the executables and libraries a package produces, making them visible to other packages.
    .library(
      name: "hazkey",
      type: .dynamic,
      targets: ["hazkey-kkc"])
  ],
  dependencies: [
    .package(
      url: "https://github.com/azooKey/AzooKeyKanaKanjiConverter",
      branch: "3f93209",
      traits: [.trait(name: "Zenzai")])
  ],
  targets: [
    // Targets are the basic building blocks of a package, defining a module or a test suite.
    // Targets can depend on other targets in this package and products from dependencies.
    .target(
      name: "hazkey-kkc",
      dependencies: [
        .product(
          name: "KanaKanjiConverterModule",
          package: "AzooKeyKanaKanjiConverter"),
        .product(
          name: "SwiftUtils",
          package: "AzooKeyKanaKanjiConverter"),
      ],
      swiftSettings: [.interoperabilityMode(.Cxx)],
      linkerSettings: [
          .unsafeFlags([
              "-L../llama.cpp/build/src",
              "-L../llama.cpp/build/ggml/src",
              "-L../llama.cpp/build/ggml/src/ggml-vulkan"
          ]),
          .linkedLibrary("llama"),
          .linkedLibrary("ggml"),
          .linkedLibrary("ggml-base"),
          .linkedLibrary("ggml-cpu"),
          .linkedLibrary("ggml-vulkan"),
          .linkedLibrary("vulkan"),
          .linkedLibrary("gomp")
      ],
    ),
    .testTarget(
      name: "hazkey-kkc-Tests",
      dependencies: [
        "hazkey-kkc"
      ]
    ),
  ]
)
