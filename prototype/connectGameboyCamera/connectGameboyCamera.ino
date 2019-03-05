// Looking down at the connector on the PCB the gameboy camera pinout is:
// --[] 5V
// --[] START
// --[] SIN
// --[] LOAD
// --[] RESET
// --[] XCK
// --[] READ
// --[] Vout
// --[] GND

const int REG_EXPOSURE_H = 2;
const int REG_EXPOSURE_L = 3;

const int PIXEL_HEIGHT = 128;
const int PIXEL_WIDTH = 128;

const uint16_t EXPOSURE_SUNLIGHT = 0x30;
const uint16_t EXPOSURE_NO_SUN = 0x300;
const uint16_t EXPOSURE_ROOM = 0x800;
const uint16_t EXPOSURE_NIGHT_LIT = 0x2C00;
const uint16_t EXPOSURE_NIGHT_LAMP = 0x5000;
const uint16_t EXPOSURE_NIGHT_TV = 0xF000;

const int PIN_IMAGE = A0;
const int PIN_READ = 2;
const int PIN_XCK = 3;
const int PIN_RESET = 4;
const int PIN_LOAD = 5;
const int PIN_SIN = 6;
const int PIN_START = 7;

unsigned char regDefaults[8] = { 0x80, 0x00, 0x00, 0x04, 0x01, 0x00, 0x01, 0x07 };

uint16_t exposure = 40;

void cam_set_register(uint8_t index, uint8_t value) {
  const int bitCount = 11;
  uint16_t wireData = ((index & 0x7) << 8) | value;

  // Expects MSB first
  for (int i = 0; i < bitCount; i++) {
    // Set data bit
    digitalWrite(PIN_SIN, ((1 << (bitCount - i - 1)) & wireData) > 0 ? HIGH : LOW);
    delayMicroseconds(1);

    bool isLastBit = i == bitCount - 1;

    if (isLastBit) {
      digitalWrite(PIN_LOAD, HIGH);
    }

    // Clock it in
    digitalWrite(PIN_XCK, HIGH);
    delayMicroseconds(1);

    digitalWrite(PIN_XCK, LOW);

    if (isLastBit) {
      digitalWrite(PIN_LOAD, LOW);
    }
  }
}

void cam_reset() {
  digitalWrite(PIN_XCK, HIGH);
  delayMicroseconds(1);

  digitalWrite(PIN_XCK, LOW);
  delayMicroseconds(1);

  // Reset is active low and has an internal 10k ohm pullup
  digitalWrite(PIN_RESET, LOW);
  pinMode(PIN_RESET, OUTPUT);
  delayMicroseconds(1);

  digitalWrite(PIN_XCK, HIGH);
  delayMicroseconds(1);

  pinMode(PIN_RESET, INPUT);
  delayMicroseconds(1);

  digitalWrite(PIN_XCK, LOW);
  delayMicroseconds(1);
}

void cam_set_exposure(uint16_t exposure) {
  cam_set_register(REG_EXPOSURE_H, exposure >> 8);
  cam_set_register(REG_EXPOSURE_L, exposure & 0xff);
}

void cam_config() {
  // IO setup
  digitalWrite(PIN_XCK, LOW);
  digitalWrite(PIN_LOAD, LOW);
  digitalWrite(PIN_SIN, LOW);
  digitalWrite(PIN_START, LOW);

  pinMode(PIN_READ, INPUT);
  pinMode(PIN_XCK, OUTPUT);
  pinMode(PIN_LOAD, OUTPUT);
  pinMode(PIN_SIN, OUTPUT);
  pinMode(PIN_START, OUTPUT);

  cam_reset();

  // Set registers to reasonable values
  for (int i = 0; i < 8; i++) {
    cam_set_register(i, regDefaults[i]);
  }
}

void cam_capture() {
  // Ask for a picture to be taken
  digitalWrite(PIN_START, HIGH);
  delayMicroseconds(1);
  digitalWrite(PIN_XCK, HIGH);
  delayMicroseconds(1);
  digitalWrite(PIN_XCK, LOW);
  delayMicroseconds(1);
  digitalWrite(PIN_START, LOW);

  // Wait for image to be exposed
  while (!digitalRead(PIN_READ)) {
    digitalWrite(PIN_XCK, HIGH);
    delayMicroseconds(100);
    digitalWrite(PIN_XCK, LOW);
    delayMicroseconds(1);
  }

  // Read out all the pixels
  for (int y = 0; y < PIXEL_HEIGHT; y++) {
    for (int x = 0; x < PIXEL_WIDTH; x++) {
      digitalWrite(PIN_XCK, HIGH);
      delayMicroseconds(1);

      // Loses 2 bits of precision to fit each sample into a byte
      Serial.write(analogRead(PIN_IMAGE) >> 2);

      digitalWrite(PIN_XCK, LOW);
      delayMicroseconds(1);
    }
  }
}

void setup() {
  // Go fast
  Serial.begin(2000000);
}

void loop() {
  cam_config();

  // Exposure is the primary # to adjust to get decent images
  cam_set_exposure(exposure);

  cam_capture();
}
