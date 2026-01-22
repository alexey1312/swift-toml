# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Test Commands

```bash
# Build the package
swift build

# Run tests
swift test

# Run a single test
swift test --filter TOMLDecoderTests/decodeSimpleTypes

# Run tests for a specific test suite
swift test --filter TOMLDecoderTests
```

### Integration Tests (toml-test compliance)

```bash
cd Tests/Integration
make              # build and run all tests
make test-decoder # decoder tests only
make test-encoder # encoder tests only
```

## Architecture

This is a Swift TOML parser/encoder built on [toml++](https://github.com/marzer/tomlplusplus) (C++17) with full Swift Codable support.

### Core Components

- **CTomlPlusPlus** (`Sources/CTomlPlusPlus/`): Pure C bridge layer around toml++ (C++17). Exposes a C-only API (`ctoml.h`) that Swift imports without requiring C++ interop mode. The C++ implementation (`ctoml.cpp`) is hidden behind `extern "C"` functions.

- **TOMLDecoder** (`Sources/TOML/Decoder.swift`): Swift `Decoder` implementation. Parses TOML via `ctoml_parse()`, converts to `TOMLValue` intermediate representation, then decodes to Swift types. Supports:
  - `DateDecodingStrategy`: iso8601, secondsSince1970, millisecondsSince1970
  - `KeyDecodingStrategy`: useDefaultKeys, convertFromSnakeCase
  - `DecodingLimits`: maxInputSize, maxDepth, maxTableKeys, maxArrayLength, maxStringLength

- **TOMLEncoder** (`Sources/TOML/Encoder.swift`): Swift `Encoder` implementation. Converts Swift types to `TOMLValue`, then serializes to TOML string. Supports:
  - `DateEncodingStrategy`: iso8601, localDateTime, localDate, localTime, secondsSince1970, millisecondsSince1970
  - `KeyEncodingStrategy`: useDefaultKeys, convertToSnakeCase
  - `OutputFormatting`: sortedKeys, prettyPrinted

- **TOMLValue** (`Sources/TOML/Value.swift`): Enum representing all TOML value types (string, integer, float, boolean, offsetDateTime, localDateTime, localDate, localTime, array, table).

- **LocalDateTime/LocalDate/LocalTime** (`Sources/TOML/LocalDateTime.swift`): Custom types for TOML's timezone-less date/time values that don't map directly to Foundation's `Date`.

### Key Patterns

- Tests use Swift Testing framework (`@Test`, `@Suite`, `#expect`)
- The decoder uses three container types: `TOMLKeyedDecodingContainer`, `TOMLUnkeyedDecodingContainer`, `TOMLSingleValueDecodingContainer`
- Key conversion (snake_case ↔ camelCase) is handled at decode/encode time via strategy options
- The C layer is minimal - just parsing and type extraction; all Swift Codable logic is in Swift
- Pure C bridge pattern: C++ internals hidden behind `extern "C"` functions, allowing Swift to import without C++ interop

### Requirements

- Swift 6.0+
- No special configuration needed for consumers - just add the dependency
