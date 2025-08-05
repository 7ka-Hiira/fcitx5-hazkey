// swift-tools-version: 6.1
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
  name: "hazkey_server",
  products: [
    // Products define the executables and libraries a package produces, making them visible to other packages.
    .executable(
      name: "hazkey_server",
      targets: ["hazkey_server"])
  ],
  dependencies: [
    .package(
      url: "https://github.com/7ka-hiira/AzooKeyKanaKanjiConverter",
      branch: "5a154b6",
      traits: [.trait(name: "Zenzai")]),
    .package(url: "https://github.com/apple/swift-protobuf.git", from: "1.27.0"),
  ],
  targets: [
    // Targets are the basic building blocks of a package, defining a module or a test suite.
    // Targets can depend on other targets in this package and products from dependencies.
    .executableTarget(
      name: "hazkey_server",
      dependencies: [
        .product(
          name: "KanaKanjiConverterModule",
          package: "AzooKeyKanaKanjiConverter"),
        .product(
          name: "SwiftUtils",
          package: "AzooKeyKanaKanjiConverter"),
        .product(name: "SwiftProtobuf", package: "swift-protobuf"),
      ],
      resources: [.copy("Protocol/hazkey_server.proto")],
      swiftSettings: [.interoperabilityMode(.Cxx)],
      linkerSettings: [
        .unsafeFlags([
          "-Lllama.cpp/build/src",
          "-Lllama.cpp/build/ggml/src",
          "-Lllama.cpp/build/ggml/src/ggml-vulkan",
        ]),
        .linkedLibrary("llama"),
        .linkedLibrary("ggml"),
        .linkedLibrary("ggml-base"),
        .linkedLibrary("ggml-cpu"),
        .linkedLibrary("ggml-vulkan"),
        .linkedLibrary("vulkan"),
        .linkedLibrary("gomp"),
      ],
    ),
    .testTarget(
      name: "hazkey_server-Tests",
      dependencies: [
        "hazkey_server",
        .product(name: "SwiftProtobuf", package: "swift-protobuf"),
      ],
      swiftSettings: [.interoperabilityMode(.Cxx)],
    ),
  ]
)
