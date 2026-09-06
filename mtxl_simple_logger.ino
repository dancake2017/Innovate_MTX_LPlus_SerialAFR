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
*/

#define SERIAL_ISP Serial2
#define ISP_BAUD 19200
#define DEBUG_BAUD 115200
#define ANALOG_PIN A0
#define SAMPLE_INTERVAL_MS 100   // 10 samples/sec

// Factory-default analog output calibration (Innovate's own manual,
// independently confirmed by rusEFI's own ego.cpp calibration table)
const float V_MIN = 0.0;
const float V_MAX = 5.0;
const float AFR_AT_V_MIN = 7.35;
const float AFR_AT_V_MAX = 22.39;

#define RAW_BUF_SIZE 64
uint8_t rawBuf[RAW_BUF_SIZE];
uint8_t rawBufLen = 0;

unsigned long lastSampleTime = 0;

float voltageToAfr(float voltage) {
  float clamped = constrain(voltage, V_MIN, V_MAX);
  float fraction = (clamped - V_MIN) / (V_MAX - V_MIN);
  return AFR_AT_V_MIN + fraction * (AFR_AT_V_MAX - AFR_AT_V_MIN);
}

void setup() {
  Serial.begin(DEBUG_BAUD);
  while (!Serial) { ; }

  SERIAL_ISP.begin(ISP_BAUD, SERIAL_8N1);
  analogReference(DEFAULT);

  Serial.println("MTX-L simple logger (raw hex + analog AFR) starting...");
  Serial.println("Wait for display to show stable 22.4 before starting CO2 test.");
  Serial.println();
}

void loop() {
  // continuously collect raw bytes into a rolling buffer
  while (SERIAL_ISP.available()) {
    uint8_t b = SERIAL_ISP.read();
    if (rawBufLen < RAW_BUF_SIZE) {
      rawBuf[rawBufLen++] = b;
    } else {
      // buffer full since last print - shift left, drop oldest byte
      memmove(rawBuf, rawBuf + 1, RAW_BUF_SIZE - 1);
      rawBuf[RAW_BUF_SIZE - 1] = b;
    }
  }

  unsigned long now = millis();
  if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = now;

    int adcValue = analogRead(ANALOG_PIN);
    float voltage = (adcValue / 1023.0) * 5.0;
    float refAfr = voltageToAfr(voltage);

    Serial.print("t=");
    Serial.print(now);
    Serial.print("ms  A0=");
    Serial.print(voltage, 2);
    Serial.print("V  RefAFR=");
    Serial.print(refAfr, 2);
    Serial.print("  raw: ");
    for (uint8_t i = 0; i < rawBufLen; i++) {
      if (rawBuf[i] < 0x10) Serial.print('0');
      Serial.print(rawBuf[i], HEX);
      Serial.print(' ');
    }
    Serial.println();

    rawBufLen = 0;   // reset the buffer for the next sample window
  }
}
