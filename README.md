# php-runtimes

Pre-built PHP runtime for [phpToro](https://github.com/phpToro/phptoro).

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

## What's included

Each archive contains:

```
lib/
  libphp.a              — PHP engine
  libphptoro_ext.a      — phpToro runtime (SAPI, plugin system, UI directives, cJSON)
  libphptoro_yoga.a     — Yoga flexbox layout engine
  libxml2.a             — XML parser
  libsqlite3.a          — SQLite
  libiconv.a            — Character encoding
  libssl.a / libcrypto.a — OpenSSL
  libsodium.a           — Cryptography
include/
  phptoro_sapi.h        — PHP request/response lifecycle
  phptoro_plugin.h      — Native plugin registry
  phptoro_ext.h         — phptoro() PHP function
  phptoro_ui.h          — UI directive plugin
  phptoro_yoga.h        — Yoga layout engine
  cJSON.h               — JSON parser
  php/                  — PHP headers
```

## Versioning

Releases follow the format `v{php_version}-{build}`, e.g. `v8.5.3-5`.

## Building locally

```bash
./build.sh --php=8.5.3 --target=macos-arm64
```

See `./build.sh --help` for all options.
