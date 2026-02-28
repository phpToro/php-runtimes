# php-runtimes

Pre-built PHP static libraries for [phpToro](https://github.com/phpToro/phptoro).

## Targets

| Target | Platform |
|---|---|
| `macos-arm64` | macOS (Apple Silicon) |
| `ios-arm64` | iOS device |
| `ios-arm64-sim` | iOS Simulator |
| `android-arm64` | Android device |
| `android-x86_64` | Android emulator |

## Download

Each target is available as a separate download from [Releases](https://github.com/phpToro/php-runtimes/releases).

Download only what you need — for example, a desktop-only app only needs `macos-arm64`.

## What's included

Each archive contains:

- `lib/` — Static libraries (`libphp.a`, `libxml2.a`, `libsqlite3.a`, `libiconv.a`, `libssl.a`, `libcrypto.a`, `libsodium.a`)
- `include/` — PHP header files for compiling extensions

## Versioning

Releases follow the format `v{php_version}-{build}`, e.g. `v8.5.3-1`.

- The PHP version matches the upstream PHP release
- The build number increments when extensions or dependencies change

## Building locally

```bash
./build.sh --php=8.5.3 --target=macos-arm64
```

See `./build.sh --help` for all options.
