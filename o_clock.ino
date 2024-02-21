
#include <DS3231.h>
#include <FastLED.h>
#include <Wire.h>

#define outputA 6
#define outputB 7

int change_minute = 4;  // down push button to adjust time (pin 2)
int change_hour = 5;
int h_led;
int m_led;
int s_led;
bool blink = true;
unsigned long hours;
unsigned long minutes;
unsigned long seconds;
int aState;
int aLastState;

#define NUM_LEDS 60
#define LED_PIN 2
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define FRAMES_PER_SECOND 50

CRGB leds[NUM_LEDS];
DS3231 rtc(SDA, SCL);

int counter = 0;
int mode = 0;

void setup() {
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(change_hour, OUTPUT);  // Initialize Outputs & Inputs
  pinMode(change_minute, OUTPUT);
  digitalWrite(change_hour, HIGH);
  digitalWrite(change_minute, HIGH);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
  rtc.begin();

  Serial.begin(9600);
  aLastState = digitalRead(outputA);
}

void loop() {

  Time t = rtc.getTime();
  hours = t.hour;
  minutes = t.min;
  seconds = t.sec;

  if (digitalRead(change_hour) == LOW) {
    Serial.println(hours);
    delay(100);
    if (hours != 11) {
      hours = hours + 1;
    } else {
      hours = 0;
    }
    FastLED.clear();
    rtc.setTime(hours, minutes, seconds);
  }
  if (digitalRead(change_minute) == LOW) {
    delay(100);
    if (minutes != 59) {
      minutes = minutes + 1;
    } else {
      minutes = 0;
    }
    FastLED.clear();
    rtc.setTime(hours, minutes, seconds);
  }

  h_led = hours % 12 * 5;
  m_led = minutes;
  s_led = seconds;

  setLeds();


  aState = digitalRead(outputA);

  if (aState != aLastState) {
    if (digitalRead(outputB) != aState) {
      counter--;
    } else {
      counter++;
    }
    if (counter % 2 == 0)
      mode += 1;

    if(mode > 2) {
      mode = 0;
    }
    Serial.print("Position: ");
    Serial.println(counter);
  }
  aLastState = aState;

  switch (mode) {
    case 0: simpleMode(); break;
    case 1: hystericalMode(); break;
    case 2: hourMode(); break;
  }
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void simpleMode() {
  if (s_led != 31) {
    FastLED.clear();
    leds[s_led - 1] = CHSV(0, 0, 0);
  }
  leds[s_led] = CHSV(HUE_RED, 255, 255);
 
  if(h_led == m_led)
    leds[h_led] = CHSV(HUE_GREEN, 255, 255);
  else{
    leds[m_led] = CHSV(HUE_YELLOW, 255, 255);
    leds[h_led] = CHSV((HUE_PURPLE) % 255, 255, 255);
  }

  if (s_led == 31) {
    FastLED.clear();
    leds[s_led] = CHSV(HUE_RED, 255, 255);
    leds[m_led] = CHSV(HUE_YELLOW, 255, 255);
    leds[h_led] = CHSV((HUE_PURPLE) % 255, 255, 255);
  }
}

void hystericalMode() {
  FastLED.clear();
  for (int i = 4; i < 60; i++) {
    for (int j = i - 4; j <= i; j++)
      leds[(s_led + j) % 60] = CHSV(HUE_BLUE, 255, 255);
    leds[h_led] = CHSV((HUE_PURPLE) % 255, 255, 255);
    leds[m_led] = CHSV(HUE_YELLOW, 255, 255);
    leds[s_led] = CHSV(HUE_RED, 255, 255);
    for (int j = 0; j < 60; j++)
      if (random(10) == 1 && j != h_led && j != m_led)
        leds[j] = leds[j].fadeToBlackBy(100);
    FastLED.show();
    delay(1);
  }
  for (int i = 0; i < 30; i++) {
    leds[h_led] = CHSV((HUE_PURPLE) % 255, 255, 255);
    leds[m_led] = CHSV(HUE_YELLOW, 255, 255);
    leds[s_led] = CHSV(HUE_BLUE, 255, 255);
    for (int j = 0; j < 60; j++)
      if (j != h_led && j != m_led && j != s_led)
        leds[j] = leds[j].fadeToBlackBy(40);
    FastLED.show();
    delay(3);
  }

  for (int i = 0; i < 60; i++){
    if (i != h_led && i != m_led && i != s_led)
      leds[i] = 0;
  }
  leds[s_led] = CHSV(HUE_RED, 255, 255);
  FastLED.show();
}

void hourMode(){
  simpleMode();
  for(int i = 0; i < 60; i++){
    if((i+1) % 5 == 0 && i != h_led)
      if(m_led == i)
        leds[i] = CHSV((HUE_PURPLE) % 255, 255, 255);
      else
        leds[i] = CHSV(HUE_PINK, 255, 255);
    if(i == h_led && blink)
      leds[i] = CHSV(HUE_PINK, 255, 255);
    if(i == h_led && !blink)
      leds[i] = CHSV(0, 0, 0);
  }

  blink = !blink;
  FastLED.show();
  delay(100);
}
void setLeds() {
  if (h_led < 31)
    h_led = h_led + 29;
  else
    h_led = h_led - 31;

  if (m_led < 31)
    m_led = m_led + 29;
  else
    m_led = m_led - 31;

  if (s_led < 31)
    s_led = s_led + 29;
  else
    s_led = s_led - 31;
}

// void setLeds() {
//   if (h_led < 30)
//     h_led = h_led + 30;
//   else
//     h_led = h_led - 30;

//   if (m_led < 30)
//     m_led = m_led + 30;
//   else
//     m_led = m_led - 30;

//   if (s_led < 30)
//     s_led = s_led + 30;
//   else
//     s_led = s_led - 30;
// }
