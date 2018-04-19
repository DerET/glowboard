#include "FastLED.h"

#define CON_PIN 5
#define CON_BAUDRATE 19200

#define LEDS_NUM 62
#define LEDS_ROW (LEDS_NUM / 2)
#define LEDS_PIN 6

#define MODE_GRADIENT 1
#define MODE_PULSE 3
#define MODE_RAINBOW 6
#define MODE_RING 7
#define MODE_SIDES 8
#define MODE_SPOT 4
#define MODE_STATIC 0
#define MODE_WAVE 5
#define MODE_WHEELS 2

// globals
CRGB leds[LEDS_NUM];

uint8_t mode = 127;
uint8_t speed = 1;
uint32_t color[] = { 0, 0, 0, 0 };

// setup
void setup() {
  FastLED.addLeds<WS2812B, LEDS_PIN, GRB>(leds, LEDS_NUM);
  pinMode(CON_PIN, INPUT);
}

// loop
void loop() {
  if (digitalRead(CON_PIN) == LOW) {
    switch (mode) {
      case MODE_GRADIENT:
        showGradient();
        break;
      case MODE_PULSE:
        showPulse();
        break;
      case MODE_RAINBOW:
        showRainbow();
        break;
      case MODE_RING:
        showRing();
        break;
      case MODE_SIDES:
        showSides();
        break;
      case MODE_SPOT:
        showSpot();
        break;
      case MODE_STATIC:
        showStatic();
        break;
      case MODE_WAVE:
        showWave();
        break;
      case MODE_WHEELS:
        showWheels();
        break;
    }

    FastLED.show();
  }
  else {
    // read values
    Serial.begin(CON_BAUDRATE);
    delay(150);
    
    mode = Serial.read();
    delay(50);

    speed = Serial.read();
    delay(50);

    for (uint8_t i = 0; i < 4; i++) {
      color[i] = 0;
      for (uint8_t k = 0; k < 24; k += 8) {
        color[i] |= ((uint32_t) Serial.read() << k);
        delay(50);
      }
    }

    Serial.end();

    // reset lights and run setup function
    for (uint8_t i = 0; i < LEDS_NUM; i++)
      leds[i] = CRGB::Black;

    switch (mode) {
      case MODE_GRADIENT:
        setupGradient();
        break;
      case MODE_PULSE:
        setupPulse();
        break;
      case MODE_RAINBOW:
        setupRainbow();
        break;
      case MODE_RING:
        setupRing();
        break;
      case MODE_SIDES:
        setupSides();
        break;
      case MODE_SPOT:
        setupSpot();
        break;
      case MODE_STATIC:
        setupStatic();
        break;
      case MODE_WAVE:
        setupWave();
        break;
      case MODE_WHEELS:
        setupWheels();
        break;
    }
  }
}

// functions to address led positions
uint8_t lef(uint8_t position) {
  return LEDS_ROW + (position % LEDS_ROW);
}

uint8_t lefr(uint8_t position) {
  return LEDS_NUM - 1 - (position % LEDS_ROW);
}

uint8_t rig(uint8_t position) {
  return LEDS_ROW - 1 - (position % LEDS_ROW);
}

uint8_t rigr(uint8_t position) {
  return position % LEDS_ROW;
}

// gradient
void setupGradient() {
  uint8_t c0[] = { color[0] >> 16, color[0] >> 8, color[0] };
  uint8_t c1[] = { color[1] >> 16, color[1] >> 8, color[1] };
  float d[3];

  for (uint8_t i = 0; i < 3; i++)
    d[i] = ((float) c1[i] - (float) c0[i]) / (LEDS_ROW - 3);

  for (uint8_t i = 0; i < LEDS_ROW - 2; i++) {
    CRGB c(c0[0] + i * d[0], c0[1] + i * d[1], c0[2] + i * d[2]);
    
    leds[lef(i + 1)] = c;
    leds[rig(i + 1)] = c;
  }

  leds[lef(0)] = color[0];
  leds[rig(0)] = color[0];
  leds[lefr(0)] = color[1];
  leds[rigr(0)] = color[1];
}

void showGradient() {
}

// pulse
uint8_t pulse_strength;
uint8_t pulse_r;
uint8_t pulse_g;
uint8_t pulse_b;

void setupPulse() {
  pulse_strength = 0;
  pulse_r = color[0] >> 16;
  pulse_g = color[0] >> 8;
  pulse_b = color[0];
}

void showPulse() {
  delay((10 - speed) * 2);

  uint8_t w = cubicwave8(pulse_strength);
  CRGB c(scale8(pulse_r, w), scale8(pulse_g, w), scale8(pulse_b, w));

  for (uint8_t i = 0; i < LEDS_NUM; i++)
    leds[i] = c;

  pulse_strength += 1;
}

// rainbow
#define rainbow_per 255 / LEDS_ROW
uint8_t rainbow_hue;

void setupRainbow() {
  rainbow_hue = 0;
}

void showRainbow() {
  delay((10 - speed) * 2);

  for (uint8_t i = 0; i < LEDS_ROW; i++) {
    uint8_t hue = rainbow_hue + i * rainbow_per;

    leds[lefr(i)].setHue(hue);
    leds[rigr(i)].setHue(hue);
  }

  rainbow_hue += 1;
}

// ring
uint8_t ring_last;

void setupRing() {
  ring_last = 0;
}

void showRing() {
  delay((14 - speed) * 2);

  leds[ring_last] = CRGB::Black;
  ring_last = (ring_last + 1) % LEDS_NUM;

  for (uint8_t i = 0; i < 8; i++)
    leds[(ring_last + i) % LEDS_NUM] = color[0];
}

// sides
void setupSides() {
  for (uint8_t i = 0; i < LEDS_ROW; i++) {
    leds[lef(i)] = color[0];
    leds[rig(i)] = color[1];
  }
}

void showSides() {
}

// spot
uint8_t spot_last;

void setupSpot() {
  spot_last = 0;
}

void showSpot() {
  delay((18 - speed) * 2);

  leds[lef(spot_last)] = CRGB::Black;
  leds[rig(spot_last)] = CRGB::Black;
  spot_last = (spot_last + 1) % LEDS_ROW;

  for (uint8_t i = 0; i < 3; i++) {
    leds[lef(spot_last + i)] = color[0];
    leds[rig(spot_last + i)] = color[0];
  }
}

// static
void setupStatic() {
  for (uint8_t i = 0; i < LEDS_NUM; i++)
    leds[i] = color[0];
}

void showStatic() {
}

// wave
uint8_t wave_last;
uint8_t wave_r;
uint8_t wave_g;
uint8_t wave_b;

void setupWave() {
  wave_last = 0;
  wave_r = (color[0] >> 16);
  wave_g = (color[0] >> 8);
  wave_b = color[0];
}

void showWave() {
  delay((18 - speed) * 2);

  leds[lef(wave_last)] = CRGB::Black;
  leds[rig(wave_last)] = CRGB::Black;
  wave_last = (wave_last + 1) % LEDS_ROW;

  for (uint8_t i = 0; i < 12; i++) {
    uint8_t w = cubicwave8(23 * i);
    CRGB c(scale8(wave_r, w), scale8(wave_g, w), scale8(wave_b, w));
    
    leds[lef(wave_last + i)] = c;
    leds[rig(wave_last + i)] = c;
  }
}

// wheels
void setupWheels() {
  for (uint8_t i = 0; i < 3; i++) {
    leds[lef(i)] = color[0];
    leds[rig(i)] = color[1];
    leds[lefr(i)] = color[2];
    leds[rigr(i)] = color[3];
  }
}

void showWheels() {
}

