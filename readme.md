# CubeConnect WLED usermod

A single-button scrolling message menu for the local WLED 17 checkout and an
8×8 non-serpentine WS2812B matrix. This is a functionality scaffold; sending a
message is simulated with three red flashes. There is no network transport yet.

## Setup

Configure WLED for 64 LEDs and an 8×8 panel with Serpentine disabled. Set the
panel orientation to match the physical wiring. The renderer addresses logical
pixels as `y * 8 + x`; WLED applies its configured LED mapping at output.

Under **Config → Usermods → CubeConnect**, enable the module and select
**button-pin**. Connect a normally-open momentary button between that GPIO and
GND. An internal pull-up is used; input-only GPIO34–39 on classic ESP32 are not
supported. `-1` disables the input. Pin conflicts are reported in WLED's Info
panel. Existing GPIO settings from the button-flash scaffold remain compatible.

## Controls

- Hold for **800 ms** from the normal display: scroll **Menu** right to left once,
  then enter the first message.
- The selected message scrolls repeatedly in its configured color on black.
- Short click: advance from **1. some sample message 1** to **2. sample text 2**,
  then wrap back to the first message. Each click restarts the selected scroll.
- Hold for **800 ms** on a message: simulate sending it with three red flashes
  (250 ms red / 250 ms black), close the menu, and reveal the WLED animation.
- A short click outside the menu does nothing.

Clicks are recognized on release. Long presses fire once per hold. Presses
started during the intro or confirmation are ignored, and a button held at boot
must be released before it can trigger an action. Text scrolls at
the configured speed; slow rendering extends the sequence rather than skipping columns.

## Editable menu settings

Under **Config → Usermods → CubeConnect**:

- **Intro → Text / Color** edits the opening text (default `Menu`) and its color.
- **Message1 → Text / Color** and **Message2 → Text / Color** edit each message
  and its color independently. The default colors are white. Numbering is part
  of the editable text; the usermod does not insert a number automatically.
- **Scroll Speed** is a slider from **1 to 30 columns/second**. Higher is faster.
  The default is 11. This speed applies to the intro and both messages.

Save to persist changes across restarts. Updating text, color, or speed closes
an active menu; hold the button again to see the new settings. Each text supports
up to 96 printable ASCII characters. Empty/all-space text and malformed colors
retain the previous value. Overlong text is truncated, unsupported bytes become
`?`, and speed is clamped to the supported range. Black is an allowed text color
and makes that screen's text invisible. Confirmation flashes remain red.

The settings-page source also escapes text values, allowing quotes, ampersands,
and angle brackets to survive reopening the form as literal text.

## Implementation and limits

`cube-connect.cpp` contains the usermod, sample messages, timing constants, and
registration. `library.json` links it into the `esp32dev` target via the local
WLED `platformio_override.ini`.

The output overlay hides WLED's display without changing its segment buffers,
preset, effect, or colors. Effects continue running underneath. Dashboard changes
made during the menu therefore remain in effect when it closes. Master brightness
and power remain under WLED control: turn WLED on to see the menu. Disable other
clock/pixel overlays when testing this standalone menu.

Reconfiguration, disabling the usermod, OTA updates, and realtime input cancel the
menu. Realtime streaming is not overridden. The private usermod/pin-owner ID is
127, unused by this checkout; coordinate an ID before combining unrelated external
usermods. This code targets this checkout's separate output-buffer rendering
contract and is not asserted compatible with older WLED releases.

The font is WLED's bundled fixed 5×8 bitmap font, attributed in its source to
[idispatch/raster-fonts](https://github.com/idispatch/raster-fonts). Lifecycle and
settings follow the [official usermod example](https://github.com/wled/wled-usermod-example).

## Validation

The menu update passed a host simulation using the actual usermod source and
bundled font/ArduinoJson, with mocked GPIO, clock, and output buffer, under address
and undefined-behavior sanitizers. Checks covered intro gating, message cycling
and wrap, repeated scrolling, clipped text, confirmation count, held buttons,
bounce, timer rollover, cancellation, and display restoration.

No new firmware build or physical-device test was performed for the menu update.
Editable settings were also checked for persistence, per-screen colors, speed,
input limits, migration from older config, and idle-click suppression.
The earlier flash-only firmware binary does not contain this menu; compile the
updated source when ready for the next hardware test.
