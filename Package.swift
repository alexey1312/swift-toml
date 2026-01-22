// swift-tools-version: 6.0

import PackageDescription

let package = Package(
    name: "TOML",
    platforms: [
        .macOS(.v10_15),
        .macCatalyst(.v13),
        .iOS(.v13),
        .watchOS(.v6),
        .tvOS(.v13),
        .visionOS(.v1),
    ],
    products: [
        .library(
            name: "TOML",
            targets: ["TOML"]
        )
    ],
    targets: [
        // Pre-compiled C library using SE-0482 artifact bundle
        .binaryTarget(
            name: "CToml",
            path: "toml.artifactbundle"
        ),

        // Swift wrapper - NO C++ interop required for consumers!
        .target(
            name: "TOML",
            dependencies: ["CToml"],
            linkerSettings: [
                .linkedLibrary("c++"),
            ]
        ),

        .testTarget(
            name: "TOMLTests",
            dependencies: ["TOML"]
        ),
    ]
)
