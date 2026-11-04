# CivSwitch ESP32-S3

An unofficial ESP32-S3 (port of [Gezine's CivSwitch](https://github.com/Gezine/CivSwitch) USB controller-and-keyboard automation. The firmware stores the 87-message CivSwitch session-loader payload and types it into Civilization VII's lobby chat. Once flashed, it can run from the board's BOOT button without a computer.

Buy this board if you need to get one
https://www.amazon.com/dp/B0F5QCK6X5?ref=ppx_yo2ov_dt_b_fed_asin_title

This repository distributes documentation and compiled firmware only. Download the `.bin` files from [Releases](https://github.com/flex36ty/civswitch-esp32-s3/releases/latest). GitHub's automatically generated “Source code” archives contain repository documentation, not the firmware project.

## Features

- Composite USB gamepad and keyboard, using the board's native USB connector.
- Full 87-message sequence stored on the device.
- Repeated USB reports and a 2-second wait after opening chat; these changes corrected missing initial characters and extra leading digits in on-console testing.
- Hold BOOT for 2 seconds to start a 10-second countdown, then release.
- Press BOOT during the countdown or typing to stop and release controls.
- USB disconnection stops an active run. Power-on and RST do not automatically start typing.
- Start, stop and monitor the board from a local browser using the compiled controller included in the release.

## Hardware and game requirements

- ESP32-S3 **N16R8**: 16 MB flash, 8 MB OPI PSRAM, with native **USB** and USB-to-UART **COM** connectors. Tested board uses UART RX GPIO44, TX GPIO43 and BOOT GPIO0.
- This build targets that configuration, not all ESP32 or ESP32-S3 boards.
- A USB data cable from the board's USB connector to the Switch dock.
- Civilization VII **Switch 1 physical cartridge edition, game version 1.0.0**. See [upstream requirements](https://github.com/Gezine/CivSwitch) for console firmware compatibility.
- System language and USB keyboard layout set to **English (US)**; Caps Lock off.

## Which binary?

| File | Flash address | Use |
| --- | --- | --- |
| `civswitch-esp32-s3-n16r8-v1.0.0-full.bin` | `0x0` | First installation or replacing another firmware. Includes bootloader, partitions and application. Replaces the board's flash contents. |
| `civswitch-esp32-s3-n16r8-v1.0.0-app.bin` | `0x10000` | Updating a board already configured with this project's 16 MB / 3 MB application partition layout. |

Do not flash the application-only file at `0x0`. Neither file is a drag-and-drop UF2.

## Control the board from your browser

After flashing, connect **COM → computer** and **USB → Switch dock** with data cables. Download [the compiled controller](https://github.com/flex36ty/civswitch-esp32-s3/releases/download/v1.0.0/civswitch-controller-cpython313.pyc) and install **CPython 3.13**. Open a terminal in the download folder. This controller build requires that Python minor version.

**Windows PowerShell**

```powershell
py -3.13 -m venv .civswitch
.\.civswitch\Scripts\python.exe -m pip install pyserial==3.5
.\.civswitch\Scripts\python.exe civswitch-controller-cpython313.pyc
```

**macOS or Linux Terminal**

```sh
python3.13 -m venv .civswitch
.civswitch/bin/python -m pip install pyserial==3.5
.civswitch/bin/python civswitch-controller-cpython313.pyc
```

1. Leave the terminal running and open [the local control page](http://127.0.0.1:8768/) on that same computer. On subsequent launches, run only the last command.
2. Check that the page says **USB connected**. Open and focus the game's lobby chat, then click **Start CivSwitch macro**. Typing starts after 10 seconds.
3. Watch the step counter or click **Stop / release controls** to cancel. The counter tracks keyboard and controller steps across the 87 messages. When finished, press **Ctrl+C** in the terminal to close the server and free the serial port.

The controller automatically selects a single recognized serial adapter. If it asks for a port, append `--port COM5`, `--port /dev/ttyUSB0`, or `--port /dev/cu.usbserial-XXXX` to the last command, substituting your board's port. If the port is busy, disconnect any serial flasher or stop the other controller process.

## Flash from a web browser (Windows, macOS or Linux)

No Python installation is needed. Use desktop Chrome or Edge and a USB data cable.

1. Download **one `.bin` file** from the [release assets](https://github.com/flex36ty/civswitch-esp32-s3/releases/tag/v1.0.0). For a new board, choose the file ending in `-full.bin`.
2. Plug the board's **COM** connector into the computer. Stop any local typing server or serial monitor using that port; closing its browser tab alone does not release the port.
3. Open [Espressif's browser flasher](https://espressif.github.io/esptool-js/). In **Program**, connect and select the board's USB serial port. Leave **WebUSB (CH340)** unchecked for this board's COM connector.
4. In the **Flash Address / File** row, select your downloaded file. Enter **`0x0` for `-full.bin`**, or **`0x10000` for `-app.bin`** on a board with the matching layout. Use only one row. Leave **Erase all** unchecked.
5. Start programming and wait for the flasher to report successful verification. Disconnect from the flasher, then tap **RST** if the board has not restarted.
6. Connect the native **USB** connector to the Switch dock and follow [Use without a computer](#use-without-a-computer). If the flasher cannot connect, see [Flash troubleshooting](#flash-troubleshooting).

## Flash from Windows PowerShell

Install Python 3, download the full binary, and open PowerShell in its download folder. Close any typing server or serial monitor using the board. Connect **COM** to the PC.

```powershell
py -m venv .venv
.\.venv\Scripts\python.exe -m pip install "esptool==4.12.0"
.\.venv\Scripts\python.exe -m serial.tools.list_ports
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM5 --baud 921600 --before default_reset --after hard_reset write_flash 0x0 civswitch-esp32-s3-n16r8-v1.0.0-full.bin
```

Replace `COM5` with the board's port. For an application-only update, replace the final address and filename with:

```text
0x10000 civswitch-esp32-s3-n16r8-v1.0.0-app.bin
```

## Flash from macOS or Linux Terminal

Install Python 3, download the full binary, and open Terminal in its download folder. Connect **COM** to the computer and close other software using its serial port.

```sh
python3 -m venv .venv
.venv/bin/python -m pip install 'esptool==4.12.0'
.venv/bin/python -m serial.tools.list_ports
```

On Linux, replace `/dev/ttyUSB0` with the listed board port:

```sh
.venv/bin/python -m esptool --chip esp32s3 --port /dev/ttyUSB0 --baud 921600 --before default_reset --after hard_reset write_flash 0x0 civswitch-esp32-s3-n16r8-v1.0.0-full.bin
```

On macOS, replace `/dev/cu.usbserial-XXXX` with the listed board port:

```sh
.venv/bin/python -m esptool --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 921600 --before default_reset --after hard_reset write_flash 0x0 civswitch-esp32-s3-n16r8-v1.0.0-full.bin
```

For an application-only update, use `0x10000` and the `-app.bin` filename instead. On Ubuntu, install `python3-venv` if needed. For serial permission errors on Ubuntu/Debian, run `sudo usermod -aG dialout "$USER"`, then log out and back in. A CH343 driver may be needed if the COM connector does not appear.

These commands pin esptool 4.12.0 to match the syntax used here. See [Espressif esptool](https://github.com/espressif/esptool) for other versions.

## Flash troubleshooting

- Success ends with **Hash of data verified** and a reset.
- If connection fails: hold BOOT, tap RST, release BOOT, and retry. This enters download mode.
- If the port is busy, close the typing server or serial monitor.
- If writes fail, try `--baud 115200` and a different data cable.
- After flashing, reconnect native USB to the Switch dock if it does not enumerate.

## Use without a computer

1. Connect the board's **USB** connector to the Switch dock. The dock powers the board; COM can remain unplugged.
2. In Civilization VII, open **Multiplayer → Wireless → Create Game → Host Lobby → Chat**. Focus the chat input and clear any existing text.
3. With the board already powered and connected, hold **BOOT for 2 seconds**, then release. Typing begins after the 10-second countdown.
4. Leave the controls alone. The first labels should read **1/87**, **2/87**, **3/87**. The macro stops after the final message.
5. To cancel, press BOOT. Release it before attempting another start. RST restarts the board without automatically typing.

Do not hold BOOT while powering on or pressing RST unless you intend to enter download mode. A completed macro means the keystroke sequence was sent; verify the loader result on the console separately. This firmware does not run the upstream persistent installer.

## Validation

The two-message test produced exact text on the user's console after the USB report and delay fixes. The user subsequently reported the full payload working well. The BOOT-button feature was compiled and flashed, but its physical start/stop behavior still needs an on-console check. Compatibility with other boards and console versions is not established.

## Credits and third-party notices

- **[Gezine / CivSwitch](https://github.com/Gezine/CivSwitch)** — original project, payload, PicoSwitch macro generator and USB descriptors; MIT license, copyright 2026 Gezine.
- **[Element2424 / Pico-Switch-Controller](https://github.com/Element2424/Pico-Switch-Controller)** — controller project credited by upstream PicoSwitch.
- **[Ha Thach / TinyUSB](https://github.com/hathach/tinyusb)** — USB stack and HID driver; MIT license. The port uses the TinyUSB 0.20.0 HID driver adapted for two interfaces and the Espressif USB API.
- **[Espressif Arduino-ESP32](https://github.com/espressif/arduino-esp32/tree/3.3.8)** and ESP-IDF — ESP32 platform/toolchain; their respective upstream licenses apply, including LGPL-2.1 for Arduino-ESP32. Upstream source and license files are available at that link.
- **[Netflix-N-Hack](https://github.com/NetflixNHack/Netflix-N-Hack)** and **[ufm42's fork](https://github.com/ufm42/Netflix-N-Hack)** — exploit-core references credited by CivSwitch.
- **[flex36ty](https://github.com/flex36ty)** — ESP32-S3 adaptation, testing and release distribution, developed with OpenAI Codex assistance.

See `THIRD-PARTY-NOTICES.txt` for the retained CivSwitch and TinyUSB MIT notices. This port is not an official release from the upstream authors, Nintendo or Firaxis.
