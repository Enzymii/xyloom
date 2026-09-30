<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Xyloom

*A little world for someone to live in.*

Xyloom is a small companion world. Its current app, **Cottage**, is an offline prototype for FoloToy AI Passport, with matsujun in a house-and-garden setting.

## Names

| Name | Role |
| --- | --- |
| **Xyloom** | Overall project and product. Its Chinese name, **Mengyu**, means “a little corner in a dream” / “a little world in a corner”; see the [Chinese README](README.zh_CN.md) for the original spelling. |
| **Cottage** | The current Passport app. |
| **Nookling** | Reserved terminology for the character system; not an implemented feature. |
| **matsujun** | The default character; the name uses Japanese romaji. |

## Current scope

The Milestone A prototype includes the house and garden, character display, dialogs, button input, camera transitions, backlight standby, and a status page showing battery level, uptime, and character state. See the [application architecture](docs/application/passport-v0.md) for implementation boundaries and deferred work, and the [validation record](docs/application/passport-v0-validation.md) for verification status.

## Project structure

| Path | Contents |
| --- | --- |
| `main/main.c` | Cottage startup and hardware adapter. |
| `main/passport/` | Cottage input, world state, and view. |
| `assets/images/passport/`, `assets/fonts/passport/` | Application artwork and fonts. |
| `components/bsp/` | FoloToy AI Passport board support. |
| `docs/application/` | Prototype architecture and validation records. |
| `tests/`, `tools/` | Tests, asset preparation, and validation tools. |

The firmware retains the existing `FoloToy-AI-Passport` CMake project identifier and directory layout. For hardware and development guidance, see the [upstream documentation index](docs/README.md) and [build instructions](docs/development/engineering/build-and-test.md).

## Licenses

Code uses the existing [MIT license](LICENSE). Fonts retain their SIL Open Font License. Application illustrations are not covered by the code's MIT license; see [asset sources and permissions](assets/README.md#passport-v0-materials).
