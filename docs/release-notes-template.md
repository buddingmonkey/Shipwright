## What this release holds

This release ships the **Android build only**. One APK runs on an Android phone or tablet, a
Samsung Galaxy XR and a Meta Quest 3 or 3S.

For Windows, Linux and macOS, use the release from the main Ship of Harkinian project at
<https://github.com/HarbourMasters/Shipwright/releases>. This fork builds and tests those
platforms on every change, but it does not publish them. Two sets of desktop binaries from two
places would only put users on a build that nobody supports.

## Before you start

Ship of Harkinian does not contain the game. You must supply your own Ocarina of Time ROM. The
supported versions are listed in
[`docs/supportedHashes.json`](https://github.com/buddingmonkey/Shipwright/blob/xr-integration/docs/supportedHashes.json).
A `.z64`, `.n64` or `.v64` file is accepted. The app asks for the ROM on the first start and makes
`oot.o2r` from it. A Master Quest ROM is optional and makes `oot-mq.o2r`.

## First start

The app keeps your saves, `shipofharkinian.json` and the `mods` folder in
`Android/media/com.harbormasters.soh` in internal storage.

To load the game, copy your ROM anywhere on the device (`Download` is fine) and start the app.
Answer **Yes** to *"No O2R files found. Generate one now?"*. The system file picker opens; choose
the ROM. After the extraction, the app asks *"Extract another?"*: answer **Yes** to add the other
of vanilla and Master Quest, or **No** to play.

## Updates

Install the new APK over the old one. **Do not uninstall first**: an uninstall deletes your saves,
your settings and `oot.o2r`. An update in place keeps all of them, and it does not make you
extract the ROM again.

## Controls

- **Phone or tablet:** on-screen touch controls, or a paired gamepad.
- **Meta Quest:** the Touch controllers play the game. Hands point at the menu and move the window;
  they do not play the game.
- **Galaxy XR:** pair a Bluetooth gamepad to play. Hands point at the menu and move the window;
  they do not play the game.

In a headset, the game is a window in your room. Open the menu with the button on the window.
**Quit** is in the menu.

## One APK covers all devices

There is no separate headset file. The same package declares the phone, the Android XR and the
Horizon OS features, and it starts in the correct mode on each one.

The APK is `arm64-v8a` only. It runs on every shipping phone and headset. It does not run on an
x86_64 emulator.

## Apple platforms

iOS, iPadOS and visionOS are not in this release. Apple permits no sideloading: build them from the
source in Xcode with a free Apple ID. See the README.
