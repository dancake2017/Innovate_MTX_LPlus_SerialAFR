# MTX-L Simple Logger

Arduino Mega sketch that logs the raw serial output of an Innovate MTX-L
wideband gauge alongside an AFR reference read from the gauge's analog output.

It does no protocol decoding (no status, heater, calibration or anchor-byte
handling). Each sample pairs:

1. A reference AFR read from the MTX-L's analog output, using Innovate's
   documented default calibration (0 V = 7.35 AFR, 5 V = 22.39 AFR).
2. The raw hex bytes received from the MTX-L's digital OUT stream during the
   same sampling window.

Both are printed together with a timestamp, so real AFR values can be matched
against real bytes offline, from a clean capture, without interpreting the
protocol while logging. The sketch was written for a CO2 sweep test.

## Wiring (Mega, Serial2)

| MTX-L                           | Mega          | Notes                                       |
| ------------------------------- | ------------- | ------------------------------------------- |
| Yellow wire (Analog Output 1)   | A0            |                                             |
| Serial OUT                      | Pin 17 (RX2)  |                                             |
| Serial IN                       | Pin 16 (TX2)  | Optional; only needed to send commands      |
| GND                             | GND           |                                             |

> **Check the serial signal levels before connecting.** If the MTX-L serial
> port uses RS-232 levels rather than 5 V TTL, it must go through a level
> shifter (for example a MAX3232) and not directly to the Mega's pins.

## Usage

1. Open `mtxl_simple_logger.ino` in the Arduino IDE (it will offer to move the
   file into a `mtxl_simple_logger` folder) and upload it to a Mega.
2. Open the Serial Monitor at 115200 baud, or run a logger on the host against
   the same port.
3. Wait for the MTX-L display to show a stable 22.4 (free air, fully warmed
   up) before starting the test. That gives a known starting point in the log.

## Output format

One line every 100 ms:

```
t=12345ms  A0=4.85V  RefAFR=22.19  raw: F7 93 5C 76 7F F7 93 5C 76 7F
```

| Field    | Meaning                                                  |
| -------- | -------------------------------------------------------- |
| `t`      | Milliseconds since the Mega started                      |
| `A0`     | Voltage measured on the analog output                    |
| `RefAFR` | AFR calculated from that voltage                         |
| `raw`    | Bytes received on Serial2 since the previous line, in hex |

Save the whole log (or redirect it to a file) and decode it after the test.

## Settings

Defined at the top of the sketch:

| Setting              | Default | Meaning                          |
| -------------------- | ------- | -------------------------------- |
| `ISP_BAUD`           | 19200   | MTX-L serial baud rate           |
| `DEBUG_BAUD`         | 115200  | USB serial baud rate             |
| `ANALOG_PIN`         | A0      | Analog reference input           |
| `SAMPLE_INTERVAL_MS` | 100     | Time between log lines           |

## Limitations

- The voltage conversion assumes the Mega's supply is exactly 5.00 V. Any
  difference shifts the reference AFR by the same proportion.
- The analog output tops out at 22.39 AFR, so the free-air reading is a
  clamped value.
- A serial packet can be split across two log lines; join the `raw` fields in
  order before decoding.

---

2026-11-15: test Claude integration
