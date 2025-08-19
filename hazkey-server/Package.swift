// swift-tools-version: 6.1
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "hazkey-server",
    products: [
        // Products define the executables and libraries a package produces, making them visible to other packages.
        .executable(
            name: "hazkey-server",
            targets: ["hazkey-server"])
    ],
    dependencies: [
        .package(
            url: "https://github.com/7ka-hiira/AzooKeyKanaKanjiConverter",
            branch: "4f8e158",
            traits: [.trait(name: "Zenzai")]),
        .package(url: "https://github.com/apple/swift-protobuf.git", from: "1.27.0"),
    ],
    targets: [
        // Targets are the basic building blocks of a package, defining a module or a test suite.
        // Targets can depend on other targets in this package and products from dependencies.
        .executableTarget(
            name: "hazkey-server",
            dependencies: [
                .product(
                    name: "KanaKanjiConverterModule",
                    package: "AzooKeyKanaKanjiConverter"),
                .product(
                    name: "SwiftUtils",
                    package: "AzooKeyKanaKanjiConverter"),
                .product(name: "SwiftProtobuf", package: "swift-protobuf"),
            ],
            swiftSettings: [.interoperabilityMode(.Cxx)],
            linkerSettings: [
                .unsafeFlags(["-L", "llama-stub"]),
                .unsafeFlags(["-Xlinker", "-rpath", "-Xlinker", "$ORIGIN/llama"]),
                .unsafeFlags(["-Xlinker", "-rpath", "-Xlinker", "$ORIGIN/llama-stub"]),
            ],
        ),
        .testTarget(
            name: "hazkey-server-tests",
            dependencies: [
                "hazkey-server",
                .product(name: "SwiftProtobuf", package: "swift-protobuf"),
            ],
            swiftSettings: [.interoperabilityMode(.Cxx)],
        ),
    ]
)
