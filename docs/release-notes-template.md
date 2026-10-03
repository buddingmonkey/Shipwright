## What this release holds

This release ships the **Android build only**. One APK runs on an Android phone or tablet, a Samsung Galaxy XR and a Meta Quest 3 or 3S.

For Windows, Linux and macOS, use the release from the main Ship of Harkinian project at <https://github.com/HarbourMasters/Shipwright/releases>. This fork builds and tests those platforms on every change, but it does not publish them. Two sets of desktop binaries from two places would only put users on a build that nobody supports.

## What is new in this release

### Foldable and dual-screen phones

- **Foldables (Pixel Fold, Galaxy Z Fold):** on the large inner screen, the game turns with the device. In portrait, the game is at the top and the touch controls are in the space below. The outer screen stays in landscape.
- **Dual-screen phones (Ayn Thor):** the game starts on the main screen. To move it to the other screen, go to **Settings > Graphics > Game Screen**. The app keeps your choice.
- **Dual-screen phones (Ayn Thor):** the screen that does not show the game shows the original cover art. To show a different image or a black screen, go to **Settings > Graphics > Other Screen Image**. The choices are Cover, Hero, Fanart, Back, Box and Black. The app keeps your choice. When the game moves to the other screen, the image moves to the screen that the game left, and your controller stays connected to the game.
- On tall windows, the HUD stays at the edges of the window.

### Menu access

- Press both sticks in (**L3 + R3**) at the same time to open or close the menu. This works with a gamepad on a phone or tablet, with a gamepad on the Galaxy XR, and with the Touch controllers on the Meta Quest.
- **Phone or tablet:** while a gamepad is connected, the menu button is hidden. To show it, go to **Settings > Controls > Menu Button > Show Menu Button With Gamepad**.
- **Headsets:** the menu button stays on the window. To hide it while controllers are connected, clear **Settings > Controls > Menu Button > Show Menu Button With Controllers**.
- You can use the menu with a gamepad: the D-pad moves from item to item, **A** selects and **B** goes back. This is on by default. To change it, go to **Settings > General > Menu Controller Navigation**.
- On a narrow screen or at a large menu scale, the top bar of the menu wraps onto two rows. Before, the items at the right end were cut off, and you could not reach them by touch. The side bar is now wide enough for its labels. This applies, for example, to the second screen of the Ayn Thor.

## Before you start

Ship of Harkinian does not contain the game. You must supply your own Ocarina of Time ROM. The supported versions are listed in [`docs/supportedHashes.json`](https://github.com/buddingmonkey/Shipwright/blob/xr-integration/docs/supportedHashes.json). A `.z64`, `.n64` or `.v64` file is accepted. The app asks for the ROM on the first start and makes `oot.o2r` from it. A Master Quest ROM is optional and makes `oot-mq.o2r`.

## First start

The app keeps your saves, `shipofharkinian.json` and the `mods` folder in `Android/media/com.harbormasters.soh` in internal storage.

To load the game, copy your ROM anywhere on the device (`Download` is fine) and start the app. Answer **Yes** to *"No O2R files found. Generate one now?"*. The system file picker opens; choose the ROM. After the extraction, the app asks *"Extract another?"*: answer **Yes** to add the other of vanilla and Master Quest, or **No** to play.

## Updates

Install the new APK over the old one. **Do not uninstall first**: an uninstall deletes your saves, your settings and `oot.o2r`. An update in place keeps all of them, and it does not make you extract the ROM again.

## Controls

- **Phone or tablet:** on-screen touch controls, or a paired gamepad. With a gamepad, press both sticks in (**L3 + R3**) to open the menu.
- **Meta Quest:** the Touch controllers play the game. Hands point at the menu and move the window; they do not play the game.
- **Galaxy XR:** pair a Bluetooth gamepad to play. Hands point at the menu and move the window; they do not play the game.

In a headset, the game is a window in your room. Open the menu with the button on the window, or press both sticks in (**L3 + R3**). **Quit** is in the menu.

In a headset, the system file picker (for the ROM or a mod) may show no close button. Use the system Back action to close it without a choice.

## One APK covers all devices

There is no separate headset file. The same package declares the phone, the Android XR and the Horizon OS features, and it starts in the correct mode on each one.

The APK is `arm64-v8a` only. It runs on every shipping phone and headset. It does not run on an x86_64 emulator.

## Apple platforms

iOS, iPadOS and visionOS are not in this release. Apple permits no sideloading: build them from the source in Xcode with a free Apple ID. See the README.
