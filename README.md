![Ship of Harkinian](docs/shiptitle.darkmode.png#gh-dark-mode-only)
![Ship of Harkinian](docs/shiptitle.lightmode.png#gh-light-mode-only)

* [Website](https://www.shipofharkinian.com)
* [Discord](https://discord.com/invite/harbourmasters)

If you're having any trouble after reading through this `README`, feel free to ask for help in the Support text channels. Please keep in mind that we do not condone piracy.

# Mobile and XR fork

This fork adds platforms that the upstream Ship of Harkinian does not build:

| Platform | How it plays |
| --- | --- |
| **Android** phone and tablet | on-screen touch controls or a gamepad |
| **Meta Quest 3 / 3S** | the game in a window in your room, Touch controllers |
| **Samsung Galaxy XR** | the game in a window in your room, a paired gamepad |
| **iPhone / iPad** | on-screen touch controls or a gamepad |
| **Apple Vision Pro** | the game in a volume in the Shared Space, a paired gamepad |

**On Windows, Linux or macOS, use [HarbourMasters/Shipwright](https://github.com/HarbourMasters/Shipwright/releases)
instead.** That repository is the canonical port.

### 1. Get a supported ROM

Check your dump at <https://ship.equipment/>, or compare its SHA-1 with
[docs/supportedHashes.json](docs/supportedHashes.json). A `.z64`, `.n64` or `.v64` file is
accepted. A Master Quest ROM is optional.

### 2. Install the app

**Android, Meta Quest and Galaxy XR:** get `SoH-<version>-android-arm64.apk` from
[Releases](../../releases). One APK covers all three. It is `arm64-v8a` only, so it runs on every
shipping phone and headset but not on an x86_64 emulator.

* **Phone or tablet:** copy the APK to the device and open it, or run `adb install -r <apk>`.
* **Meta Quest 3 / 3S:** turn on developer mode for the headset in the Meta Horizon phone app,
  connect by USB, accept the prompt in the headset, then run `adb install -r <apk>`. The app is in
  the library under *Unknown Sources*.
* **Samsung Galaxy XR:** turn on Developer options and USB debugging in Settings, then run
  `adb install -r <apk>`.

To update, install the new APK over the old one. **Do not uninstall first.** An uninstall deletes
your saves, your settings and `oot.o2r`.

**iPhone, iPad and Apple Vision Pro:** Apple permits no sideloading, so there is no download.
Build the app on a Mac and run it on your device from Xcode. See
[iOS](docs/BUILDING.md#ios) and [visionOS](docs/BUILDING.md#visionos-apple-vision-pro) in
docs/BUILDING.md. A free Apple ID is enough. Its profile expires after 7 days, and you then build
again.

### 3. Give the app the ROM

The app makes `oot.o2r` from your ROM on the first start. This is necessary one time only.

* **Android, Quest, Galaxy XR:** copy the ROM anywhere on the device; `Download` is fine. Start
  the app and answer **Yes** to *"No O2R files found. Generate one now?"*. The system file picker
  opens. Choose the ROM.
* **iPhone, iPad, Vision Pro:** start the app once. It makes a `Ship of Harkinian` folder under
  *On My iPhone* / *On My iPad* / *On My Apple Vision Pro* in the Files app. Copy the ROM into
  that folder, then answer **Yes** to *"No O2R files found. Generate one now?"* and **Yes** to
  *"ROMs found in application directory"*.

After the extraction the app asks *"Extract another?"*. Answer **Yes** to add the other of vanilla
and Master Quest, or **No** to play.

Saves, `shipofharkinian.json` and the `mods` folder are in `Android/media/com.harbormasters.soh`
on Android and in the app's folder in the Files app on Apple devices.

### Controls

* **Phone and tablet:** on-screen touch controls. They hide while a gamepad is connected.
* **Meta Quest:** the Touch controllers play the game.
* **Galaxy XR and Vision Pro:** pair a gamepad to play.
* In a headset, hands (look and pinch on Vision Pro) point at the menu and move the window. They
  do not play the game. Open the menu with the button on the window (the **Menu** button under the
  volume on Vision Pro). *Settings > Graphics > Diorama Depth* sets how deep the world looks
  behind the window.

---

# Quick Start

The Ship does not include any copyrighted assets.  You are required to provide a supported copy of the game.

### 1. Verify your ROM dump
You can verify you have dumped a supported copy of the game by using the compatibility checker at https://ship.equipment/. If you'd prefer to manually validate your ROM dump, you can cross-reference its `sha1` hash with the hashes [here](docs/supportedHashes.json).

### 2. Download The Ship of Harkinian from [Releases](https://github.com/HarbourMasters/Shipwright/releases)

### 3. Launch the Game!
#### Windows
* Extract the zip
* Launch `soh.exe`

#### Linux
* Execute `soh.appimage`.  You may have to `chmod +x` the appimage via terminal.
* When prompted, select your supported copy of the game.
* Saves, settings and `oot.o2r` are stored in `~/.local/share/soh/`. If the folder you launch from already has `shipofharkinian.json`, `oot.o2r` or `oot-mq.o2r`, that folder is used instead. Set `SHIP_HOME` to choose another folder.

#### macOS
* Run `soh.app`. When prompted, select your supported copy of the game.
* You should see a notification saying `Processing OTR`, then, once the process is complete, you should get a notification saying `OTR Successfully Generated`, then the game should start.

### 4. Play!

Congratulations, you are now sailing with the Ship of Harkinian! Have fun!

# Configuration

### Default keyboard configuration
| N64 | A | B | Z | Start | Analog stick | C buttons | D-Pad |
| - | - | - | - | - | - | - | - |
| Keyboard | X | C | Z | Space | WASD | Arrow keys | TFGH |

### Other shortcuts
| Keys | Action |
| - | - |
| ESC | Toggle menu |
| F2 | Toggle capture mouse input |
| F5 | Save state |
| F6 | Change state |
| F7 | Load state |
| F9 | Toggle Text-to-Speech |
| F11 | Fullscreen |
| Tab | Toggle Alternate assets |
| Ctrl+R | Reset |

# Project Overview
Ship of Harkinian (SOH) is built atop a custom library dubbed libultraship (LUS). Back in the N64 days, there was an SDK distributed to developers named libultra; LUS is designed to mimic the functionality of libultra on modern hardware. In addition, we are dependent on the source code provided by the OOT decompilation project.

In order for the game to function, you will require a **legally acquired** ROM for Ocarina of Time. Click [here](https://ship.equipment/) to check the compatibility of your specific rom. Any copyrighted assets are extracted from the ROM and reformatted as a .o2r archive file which the code uses.

### Graphics Backends
Currently, there are three rendering APIs supported: DirectX11 (Windows), OpenGL (all platforms), and Metal (MacOS). You can change which API to use in the `Settings` menu of the menubar, which requires a restart.  If you're having an issue with crashing, you can change the API in the `shipofharkinian.json` file by finding the line `gfxbackend:""` and changing the value to `sdl` for OpenGL. DirectX 11 is the default on Windows.

# Custom Assets

Custom assets are packed in `.otr` archive files. To use custom assets, place them in the `mods` folder.

If you're interested in creating and/or packing your own custom asset `.otr` files, check out the following tools:
* [**retro - OTR generator**](https://github.com/HarbourMasters64/retro)
* [**fast64 - Blender plugin**](https://github.com/HarbourMasters/fast64)

# Development
### Building

If you want to manually compile SoH, please consult the [building instructions](docs/BUILDING.md).

### Playtesting
If you want to playtest a continuous integration build, you can find them at the links below. Keep in mind that these are for playtesting only, and you will likely encounter bugs and possibly crashes. 

* [Windows](https://nightly.link/HarbourMasters/Shipwright/workflows/generate-builds/develop/soh-windows.zip)
* [macOS](https://nightly.link/HarbourMasters/Shipwright/workflows/generate-builds/develop/soh-mac.zip)
* [Linux](https://nightly.link/HarbourMasters/Shipwright/workflows/generate-builds/develop/soh-linux.zip)

### Further Reading
More detailed documentation can be found in the 'docs' directory, including the aforementioned [building instructions](docs/BUILDING.md).

* [Credits](docs/CREDITS.md)
* [Custom Music](docs/CUSTOM_MUSIC.md)
* [Formatting](docs/FORMATTING.md)
* [Controller Mapping](docs/GAME_CONTROLLER_DB.md)
* [Modding](docs/MODDING.md)
* [Versioning](docs/VERSIONING.md)

<a href="https://github.com/Kenix3/libultraship/">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="./docs/poweredbylus.darkmode.png">
    <img alt="Powered by libultraship" src="./docs/poweredbylus.lightmode.png">
  </picture>
</a>
