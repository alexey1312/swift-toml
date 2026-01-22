#!/bin/bash
set -euo pipefail

VERSION="${1:-1.0.0}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/.toml-build"
BUNDLE_DIR="$PROJECT_ROOT/toml.artifactbundle"

# Clean up
rm -rf "$BUILD_DIR"
rm -rf "$BUNDLE_DIR"
mkdir -p "$BUILD_DIR"

echo "=== Building TOML C Library v$VERSION ==="
echo "Project root: $PROJECT_ROOT"
echo ""

# Common compiler flags
COMMON_FLAGS="-std=c++17 -O2 -DTOML_HEADER_ONLY=1 -DNDEBUG=1"
INCLUDE_FLAGS="-I$PROJECT_ROOT/Sources/CTomlPlusPlus"

# Deployment targets
MACOS_DEPLOYMENT_TARGET="10.15"
IOS_DEPLOYMENT_TARGET="13.0"
CATALYST_DEPLOYMENT_TARGET="14.0"

# Build for macOS (universal binary)
build_macos() {
    echo "Building for macOS..."

    # ARM64
    echo "  Compiling arm64..."
    clang++ -c $COMMON_FLAGS -arch arm64 \
        -mmacosx-version-min=$MACOS_DEPLOYMENT_TARGET \
        $INCLUDE_FLAGS \
        "$PROJECT_ROOT/Sources/CTomlPlusPlus/ctoml.cpp" \
        -o "$BUILD_DIR/ctoml-arm64.o"

    # x86_64
    echo "  Compiling x86_64..."
    clang++ -c $COMMON_FLAGS -arch x86_64 \
        -mmacosx-version-min=$MACOS_DEPLOYMENT_TARGET \
        $INCLUDE_FLAGS \
        "$PROJECT_ROOT/Sources/CTomlPlusPlus/ctoml.cpp" \
        -o "$BUILD_DIR/ctoml-x86_64.o"

    # Create static libraries for each arch
    ar rcs "$BUILD_DIR/libtoml-arm64.a" "$BUILD_DIR/ctoml-arm64.o"
    ar rcs "$BUILD_DIR/libtoml-x86_64.a" "$BUILD_DIR/ctoml-x86_64.o"

    # Create universal binary
    echo "  Creating universal binary..."
    mkdir -p "$BUNDLE_DIR/macos-universal"
    lipo -create \
        "$BUILD_DIR/libtoml-arm64.a" \
        "$BUILD_DIR/libtoml-x86_64.a" \
        -output "$BUNDLE_DIR/macos-universal/libtoml.a"

    echo "  macOS build complete: $BUNDLE_DIR/macos-universal/libtoml.a"
}

# Build for iOS device (arm64)
build_ios_device() {
    echo "Building for iOS device..."

    # Get iOS SDK path
    IOS_SDK=$(xcrun --sdk iphoneos --show-sdk-path)

    echo "  Compiling arm64..."
    clang++ -c $COMMON_FLAGS -arch arm64 \
        -isysroot "$IOS_SDK" \
        -target arm64-apple-ios13.0 \
        $INCLUDE_FLAGS \
        "$PROJECT_ROOT/Sources/CTomlPlusPlus/ctoml.cpp" \
        -o "$BUILD_DIR/ctoml-ios-arm64.o"

    mkdir -p "$BUNDLE_DIR/ios-arm64"
    ar rcs "$BUNDLE_DIR/ios-arm64/libtoml.a" "$BUILD_DIR/ctoml-ios-arm64.o"

    echo "  iOS device build complete: $BUNDLE_DIR/ios-arm64/libtoml.a"
}

# Build for iOS Simulator (arm64 + x86_64)
build_ios_simulator() {
    echo "Building for iOS Simulator..."

    # Get iOS Simulator SDK path
    SIM_SDK=$(xcrun --sdk iphonesimulator --show-sdk-path)

    # ARM64 (Apple Silicon Macs)
    echo "  Compiling arm64..."
    clang++ -c $COMMON_FLAGS -arch arm64 \
        -isysroot "$SIM_SDK" \
        -target arm64-apple-ios13.0-simulator \
        $INCLUDE_FLAGS \
        "$PROJECT_ROOT/Sources/CTomlPlusPlus/ctoml.cpp" \
        -o "$BUILD_DIR/ctoml-sim-arm64.o"

    # x86_64 (Intel Macs)
    echo "  Compiling x86_64..."
    clang++ -c $COMMON_FLAGS -arch x86_64 \
        -isysroot "$SIM_SDK" \
        -target x86_64-apple-ios13.0-simulator \
        $INCLUDE_FLAGS \
        "$PROJECT_ROOT/Sources/CTomlPlusPlus/ctoml.cpp" \
        -o "$BUILD_DIR/ctoml-sim-x86_64.o"

    # Create static libraries for each arch
    ar rcs "$BUILD_DIR/libtoml-sim-arm64.a" "$BUILD_DIR/ctoml-sim-arm64.o"
    ar rcs "$BUILD_DIR/libtoml-sim-x86_64.a" "$BUILD_DIR/ctoml-sim-x86_64.o"

    # Create universal binary
    echo "  Creating universal binary..."
    mkdir -p "$BUNDLE_DIR/ios-simulator"
    lipo -create \
        "$BUILD_DIR/libtoml-sim-arm64.a" \
        "$BUILD_DIR/libtoml-sim-x86_64.a" \
        -output "$BUNDLE_DIR/ios-simulator/libtoml.a"

    echo "  iOS Simulator build complete: $BUNDLE_DIR/ios-simulator/libtoml.a"
}

# Build for macOS Catalyst
build_maccatalyst() {
    echo "Building for Mac Catalyst..."

    # Get macOS SDK path
    MACOS_SDK=$(xcrun --sdk macosx --show-sdk-path)

    # ARM64
    echo "  Compiling arm64..."
    clang++ -c $COMMON_FLAGS -arch arm64 \
        -isysroot "$MACOS_SDK" \
        -target arm64-apple-ios14.0-macabi \
        -iframework "$MACOS_SDK/System/iOSSupport/System/Library/Frameworks" \
        $INCLUDE_FLAGS \
        "$PROJECT_ROOT/Sources/CTomlPlusPlus/ctoml.cpp" \
        -o "$BUILD_DIR/ctoml-catalyst-arm64.o"

    # x86_64
    echo "  Compiling x86_64..."
    clang++ -c $COMMON_FLAGS -arch x86_64 \
        -isysroot "$MACOS_SDK" \
        -target x86_64-apple-ios14.0-macabi \
        -iframework "$MACOS_SDK/System/iOSSupport/System/Library/Frameworks" \
        $INCLUDE_FLAGS \
        "$PROJECT_ROOT/Sources/CTomlPlusPlus/ctoml.cpp" \
        -o "$BUILD_DIR/ctoml-catalyst-x86_64.o"

    # Create static libraries for each arch
    ar rcs "$BUILD_DIR/libtoml-catalyst-arm64.a" "$BUILD_DIR/ctoml-catalyst-arm64.o"
    ar rcs "$BUILD_DIR/libtoml-catalyst-x86_64.a" "$BUILD_DIR/ctoml-catalyst-x86_64.o"

    # Create universal binary
    echo "  Creating universal binary..."
    mkdir -p "$BUNDLE_DIR/maccatalyst-universal"
    lipo -create \
        "$BUILD_DIR/libtoml-catalyst-arm64.a" \
        "$BUILD_DIR/libtoml-catalyst-x86_64.a" \
        -output "$BUNDLE_DIR/maccatalyst-universal/libtoml.a"

    echo "  Mac Catalyst build complete: $BUNDLE_DIR/maccatalyst-universal/libtoml.a"
}

# Create artifact bundle metadata
create_bundle() {
    echo ""
    echo "Creating artifact bundle..."

    # Copy headers
    mkdir -p "$BUNDLE_DIR/include"
    cp "$PROJECT_ROOT/Sources/CTomlPlusPlus/include/ctoml.h" "$BUNDLE_DIR/include/"

    # Create module.modulemap
    cat > "$BUNDLE_DIR/include/module.modulemap" << 'EOF'
module CToml {
    header "ctoml.h"
    export *
}
EOF

    # Create info.json
    cat > "$BUNDLE_DIR/info.json" << EOF
{
    "schemaVersion": "1.0",
    "artifacts": {
        "CToml": {
            "type": "staticLibrary",
            "version": "$VERSION",
            "variants": [
                {
                    "path": "macos-universal/libtoml.a",
                    "supportedTriples": [
                        "arm64-apple-macosx",
                        "x86_64-apple-macosx"
                    ],
                    "staticLibraryMetadata": {
                        "headerPaths": ["include"],
                        "moduleMapPath": "include/module.modulemap",
                        "linkedLibraries": ["c++"]
                    }
                },
                {
                    "path": "ios-arm64/libtoml.a",
                    "supportedTriples": [
                        "arm64-apple-ios"
                    ],
                    "staticLibraryMetadata": {
                        "headerPaths": ["include"],
                        "moduleMapPath": "include/module.modulemap",
                        "linkedLibraries": ["c++"]
                    }
                },
                {
                    "path": "ios-simulator/libtoml.a",
                    "supportedTriples": [
                        "arm64-apple-ios-simulator",
                        "x86_64-apple-ios-simulator"
                    ],
                    "staticLibraryMetadata": {
                        "headerPaths": ["include"],
                        "moduleMapPath": "include/module.modulemap",
                        "linkedLibraries": ["c++"]
                    }
                },
                {
                    "path": "maccatalyst-universal/libtoml.a",
                    "supportedTriples": [
                        "arm64-apple-ios-macabi",
                        "x86_64-apple-ios-macabi"
                    ],
                    "staticLibraryMetadata": {
                        "headerPaths": ["include"],
                        "moduleMapPath": "include/module.modulemap",
                        "linkedLibraries": ["c++"]
                    }
                }
            ]
        }
    }
}
EOF

    echo "  info.json created"
    echo "  Headers copied"
}

# Show usage
usage() {
    echo "Usage: $0 [VERSION] [TARGETS...]"
    echo ""
    echo "Arguments:"
    echo "  VERSION   Version string (default: 1.0.0)"
    echo "  TARGETS   One or more of: macos, ios, simulator, catalyst, all"
    echo "            (default: all)"
    echo ""
    echo "Examples:"
    echo "  $0                     # Build all targets with version 1.0.0"
    echo "  $0 2.0.0               # Build all targets with version 2.0.0"
    echo "  $0 1.0.0 macos         # Build only macOS"
    echo "  $0 1.0.0 ios simulator # Build iOS device and simulator"
    echo ""
}

# Main build logic
main() {
    # Parse targets
    shift || true  # Skip version argument
    TARGETS=("$@")

    # Default to all if no targets specified
    if [ ${#TARGETS[@]} -eq 0 ]; then
        TARGETS=("all")
    fi

    # Build requested targets
    for target in "${TARGETS[@]}"; do
        case "$target" in
            macos)
                build_macos
                ;;
            ios)
                build_ios_device
                ;;
            simulator)
                build_ios_simulator
                ;;
            catalyst)
                build_maccatalyst
                ;;
            all)
                build_macos
                build_ios_device
                build_ios_simulator
                build_maccatalyst
                ;;
            help|--help|-h)
                usage
                exit 0
                ;;
            *)
                echo "Unknown target: $target"
                usage
                exit 1
                ;;
        esac
    done

    # Create bundle metadata
    create_bundle

    # Clean up build directory
    rm -rf "$BUILD_DIR"

    echo ""
    echo "=== Build Complete ==="
    echo "Artifact bundle: $BUNDLE_DIR"
    echo ""
    echo "Contents:"
    find "$BUNDLE_DIR" -type f | sort | while read -r file; do
        size=$(du -h "$file" | cut -f1)
        echo "  $size  ${file#$BUNDLE_DIR/}"
    done
}

main "$@"
