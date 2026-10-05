# Installing Ship of Harkinian (Mobile and XR) on Apple Vision Pro

This fork of [Ship of Harkinian](https://github.com/HarbourMasters/Shipwright), Harbour Masters' native Ocarina of Time port, adds Android, Meta Quest, Galaxy XR, iPhone, iPad and Apple Vision Pro builds. On Vision Pro the app is a native SwiftUI and RealityKit shell that shows the game in a volume in the Shared Space, rendered with Metal. The game is drawn once per eye, and the app uses the position of your head to show depth in the window.

## What you need

- A Mac with Xcode 26 or newer and the visionOS SDK, and CMake 3.28 or newer
- An Apple ID. A free one is enough; its profile expires after 7 days, and you then build again.
- Apple Vision Pro
- A paired gamepad. It's needed to play; look and pinch only drive the menu.
- Your own Ocarina of Time ROM (`.z64`, `.n64` or `.v64`). A Master Quest ROM is optional.

## Your game files

This repository and the app contain no game assets. The app makes `oot.o2r` from your ROM on the first start, on the headset; this is needed one time only.

1. Check your dump at <https://ship.equipment/>, or compare its SHA-1 with [docs/supportedHashes.json](docs/supportedHashes.json).
2. Install the app (below) and start it. Answer **Yes** to *"No O2R files found. Generate one now?"*. The app shows *"No ROM Found"* and names its folder in the Files app.
3. In the Files app, copy the ROM into *On My Apple Vision Pro > Ship of Harkinian* (the Master Quest ROM too, if you have one).
4. Back in the app, select **Search Again**. Answer **Yes** to *"ROMs found in application directory"*, then **Yes** to *"All files have been processed. Run SoH?"*.

Saves, `shipofharkinian.json` and the `mods` folder are in the same folder in the Files app.

## Build from source

There is no download for Apple Vision Pro; you build the app on a Mac and run it from Xcode. CMake generates the Xcode project once, and Xcode builds, signs and installs it.

From a checkout of this repository, with its submodules (`git submodule update --init --recursive` if you don't have them yet):

```bash
cmake -S . -B build-visionos -G Xcode \
  -DCMAKE_TOOLCHAIN_FILE=CMake/ios.toolchain.cmake \
  -DPLATFORM=VISIONOS \
  -DDEPLOYMENT_TARGET=2.0 \
  -DCMAKE_IGNORE_PREFIX_PATH="/opt/homebrew;/usr/local;/opt/local" \
  -DPROJECT_ID=com.yourname.soh \
  -DIOS_DEVELOPMENT_TEAM=YOURTEAMID

cmake --build build-visionos --config Release --target GenerateSohOtr

open build-visionos/Ship.xcodeproj
```

`IOS_DEVELOPMENT_TEAM` is your 10-character Apple Developer Team ID (from <https://developer.apple.com/account>), and `PROJECT_ID` a bundle identifier your team owns. A free Apple ID added under *Xcode > Settings > Accounts* works.

1. Pair the headset under *Window > Devices and Simulators*.
2. Select the **soh** scheme, choose your Vision Pro, and press **Run** (⌘R).

The generated scheme builds Release; a Debug app makes the on-device extraction far slower. `-DPLATFORM=SIMULATOR_VISIONOS` makes a Simulator project, which needs no signing; the Simulator shows one view, so it can't show the stereo result. The full instructions for every platform are in [docs/BUILDING.md](docs/BUILDING.md#visionos-apple-vision-pro).

## Notes

- The paired gamepad plays the game. Look and pinch point at the menu; the **Menu** button under the volume opens it.
- The system window bar moves and resizes the volume. *Settings > Graphics > Diorama Depth* sets how deep the world looks behind the window.
- The system file picker (for the ROM or a mod) may show no close button. Use the system Back action to close it without choosing anything.
- On Windows, Linux or macOS, use [HarbourMasters/Shipwright](https://github.com/HarbourMasters/Shipwright/releases) instead; it is the canonical port.
