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

- `lib/` — Static libraries (`libphp.a`, `libphptoro_ext.a`, `libxml2.a`, `libsqlite3.a`, `libiconv.a`, `libssl.a`, `libcrypto.a`, `libsodium.a`)
- `include/` — PHP and phpToro headers
- `src/` — Source files compiled by the platform project (Yoga layout engine)

### libphptoro_ext.a

The phpToro runtime library bundles:

| File | Purpose |
|---|---|
| `phptoro_sapi.c` | PHP Embed SAPI (request/response lifecycle) |
| `phptoro_ext.c` | `phptoro()` PHP function and `PhpToroPlugin` class |
| `phptoro_plugin.c` | Native plugin registry and dispatcher |
| `phptoro_ui.c` | UI directive plugin (alert, navigate, flash, haptic, etc.) |
| `phptoro_phpinfo.c` | Branded phpinfo() output |
| `cJSON.c` | JSON parser (MIT, v1.7.18) |

### src/

Source files that require platform-specific dependencies:

| File | Purpose | Dependency |
|---|---|---|
| `phptoro_yoga.c` | Flexbox layout engine | Yoga C++ |

These are compiled by the platform project (Xcode, Gradle) which provides the required headers.

## Versioning

Releases follow the format `v{php_version}-{build}`, e.g. `v8.5.3-5`.

## Building locally

```bash
./build.sh --php=8.5.3 --target=macos-arm64
```

See `./build.sh --help` for all options.
