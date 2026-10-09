<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>


Product direction and the next scoped batches: [vision](docs/application/vision.md) · [roadmap](docs/application/roadmap.md).

# Xyloom

*A little world for someone to live in.*

Xyloom is a small companion world. Its current app, **Cottage**, is a companion prototype for FoloToy AI Passport, with matsujun in a house-and-garden setting.

## Names

| Name | Role |
| --- | --- |
| **Xyloom** | Overall project and product. Its Chinese name, **Mengyu**, means “a little corner in a dream” / “a little world in a corner”; see the [Chinese README](README.zh_CN.md) for the original spelling. |
| **Cottage** | The current Passport app. |
| **Nookling** | Reserved terminology for the character system; not an implemented feature. |
| **matsujun** | The default character; the name uses Japanese romaji. |

## Current scope

The Milestone A prototype includes the house and garden, character display, dialogs, button input, camera transitions, backlight standby, and a status page showing battery level, uptime, and character state. See the [application architecture](docs/application/passport-v0.md) for implementation boundaries and deferred work, and the [validation record](docs/application/passport-v0-validation.md) for verification status.

In the garden, select the watering can and press OK to water manually. The can flies to the flower, pours, and returns. Each completed watering records one cup. The first four cups per quota cycle advance the tulip: seed at 0, sprout at 4, bud at 20, bloom at 56. Today’s cups, daily quota, growth and lifetime cups survive reboot. Bloom remains visible while cup recording continues. The status page offers Wi-Fi setup through a temporary phone-accessible hotspot; the device remembers one network and synchronizes its date online. Watering works immediately offline. Without a trusted date, four growth points refresh after 24 hours of accumulated powered runtime; reboot retains the cycle and quota, and time powered off does not count. First synchronization carries the used quota forward; the next trusted date starts a calendar-day allowance. A Wi-Fi icon stays beside the battery. The garden keeps four small water drops in the header without a backing panel. Plant illustrations and gentle inspection text convey growth; percentages, progress bars, cup totals and timing rules stay off the garden UI.

## Project structure

| Path | Contents |
| --- | --- |
| `main/main.c` | Cottage startup and hardware adapter. |
| `main/passport/` | Cottage input, world state, growth, storage, networking, and view. |
| `assets/images/passport/`, `assets/fonts/passport/` | Application artwork and fonts. |
| `components/bsp/` | FoloToy AI Passport board support. |
| `docs/application/` | Prototype architecture and validation records. |
| `tests/`, `tools/` | Tests, asset preparation, and validation tools. |

The firmware retains the existing `FoloToy-AI-Passport` CMake project identifier and directory layout. For hardware and development guidance, see the [upstream documentation index](docs/README.md) and [build instructions](docs/development/engineering/build-and-test.md).

## Simulator network profile

Run `./tools/validate.sh --simulator` and upload `build/Cottage-simulator-full.bin`
to the local FoloToy simulator. Cottage connects through the computer's network
without home Wi-Fi credentials. Use Status to clear/disconnect or set up/reconnect
the virtual Wi-Fi. This RAM-only development profile must not be flashed to a
physical device. See [profile details](docs/application/passport-v0.md#simulator-networking).

## Licenses

Code uses the existing [MIT license](LICENSE). Fonts retain their SIL Open Font License. Application illustrations are not covered by the code's MIT license; see [asset sources and permissions](assets/README.md#passport-v0-materials).

## MVP life rhythm

Cottage now includes a moonlit garden and warm evening cabin, a seated sleep pose from 22:00 to 07:00, and one daily half-hour walk. Remembered time continues during offline powered use and survives restart; a never-synchronized first boot stays in daytime. Watering remains available during sleep or outings. See the [life contract](docs/application/passport-v0.md#local-life-rhythm-2026-10-09) and exact [validation](docs/application/passport-v0-validation.md) before physical acceptance.
