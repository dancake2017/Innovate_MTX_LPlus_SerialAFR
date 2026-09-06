/*
  MTX-L Simple Logger: Raw Hex + Analog AFR Reference
  --------------------------------------------------------
  Simplified for the CO2 sweep test. No status/HTR/CAL/anchor-byte
  decoding at all - just:
    1) A trustworthy AFR reference read from the MTX-L's analog output
       (documented Innovate calibration: 0V=7.35 AFR, 5V=22.39 AFR)
    2) The raw hex bytes received from the MTX-L's digital OUT stream
       during that same sampling window

  Both printed together, timestamped, so you can pair real AFR values
  against real bytes without needing to interpret the protocol at all
  during logging - that gets done afterward, offline, against a clean
  capture.

  WIRING (Mega, Serial2)
  --------------------------
  MTX-L YELLOW wire (Analog Output 1) --> Mega A0
  MTX-L OUT                           --> Mega pin 17 (RX2)
  MTX-L IN                            --> Mega pin 16 (TX2) [optional,
                                           only needed if you want to
                                           send commands - not required
                                           for this logger]
  MTX-L GND                           --> Mega GND

  USAGE
  -----
  Open Serial Monitor at 115200 baud (or run a Python logger against
  the same port). Wait for the MTX-L display to show a stable 22.4
  (free air, fully warmed up) before starting your CO2 test - that
  gives you a known-good starting reference point in the log.

  Each line looks like:
    t=12345ms  A0=4.85V  RefAFR=22.19  raw: F7 93 5C 76 7F F7 93 5C 76 7F

  "raw" is whatever came in on Serial2 since the last sample - copy the
  whole log (or redirect to a file) and we'll decode it properly after
  the test, once we know exactly which AFR values correspond to which
  stretch of bytes.
