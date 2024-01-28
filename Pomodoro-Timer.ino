#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN D1
#define BUTTON_RED_PIN D2
#define BUTTON_BLACK_PIN D3
#define STRIPSIZE 12

const char *ssid = "WLAN-Name";
const char *password = "WLAN-Passwort";

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPSIZE, LED_PIN, NEO_GRB + NEO_KHZ800);

const long utcOffsetInSeconds = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_RED_PIN, INPUT_PULLUP);
  pinMode(BUTTON_BLACK_PIN, INPUT_PULLUP);
  strip.begin();
  strip.setBrightness(3);
  strip.show();
  timeClient.begin();
}

bool isConnected = false;
int phases = 4;
int phaseCounter = 0;

void loop() {
  while (!isConnected) {
    if (!digitalRead(BUTTON_RED_PIN) && !digitalRead(BUTTON_BLACK_PIN)) {
      isConnected = wiFiConnect();
    }
  }
  int r = 255;
  int g = 0;
  int b = 0;
  while (phaseCounter < phases && isConnected) {
    startLearningTime(r, g, b);
    delay(10);
    g += 85;
    Serial.println(phaseCounter);
    phaseCounter++;
  }
  isConnected = !wiFiDisconnect();
  delay(10);
}

void startBreakTime(int r, int g, int b) {
  int breakTime = 5;
  int count = 0;
  colorFill(155, 0, 255);
  delay(10);
  while (count < STRIPSIZE) {
    count = stripTimer(count, ((breakTime * 60) / 12));
    if (count < 0) {
      return;
    }
    colorFill(count, r, g, b);
  }
  switchPhase(r, g, b);
}

void startLearningTime(int r, int g, int b) {
  int count = 0;
  colorFill(r, g, b);
  delay(10);
  while (count < STRIPSIZE) {
    count = stripTimer(count, 120);
    if (count < 0) {
      return;
    }
    if (count < STRIPSIZE) {
      Serial.println("a"+count);
      colorFill(count, 0, 255, 0);
    }
  }
  switchPhase(0, 255, 0);
  if (phaseCounter < 4) {
    startBreakTime(r, g, b);
  }
}
long lastcheck = 0;
void switchPhase(int r, int g, int b) {
  while (true) {
    if (millis() - lastcheck > 500) {
      colorBlinking(r, g, b, 1);
      lastcheck = millis();
    }
    if (!digitalRead(BUTTON_RED_PIN) == HIGH) {
      return;
    }
  }
}


int stripTimer(int count, int seconds) {
  timeClient.update();
  if (timeClient.isTimeSet()) {
    int startSeconds = timeClient.getEpochTime();
    int secondsOffSet = 0;
    int originalCount = count;
    delay(1000);

    while (originalCount == count) {
      timeClient.update();
      secondsOffSet = timeClient.getEpochTime() - startSeconds;

      if (secondsOffSet > 0 && secondsOffSet % seconds == 0) {
        count++;
        Serial.println(count);
        return count;
      }

      if (!digitalRead(BUTTON_RED_PIN) && !digitalRead(BUTTON_BLACK_PIN)) {
        isConnected = !wiFiDisconnect();
        return -1;
      }
      delay(10);
    }
    delay(1000);
  }
  return -1;
}

bool wiFiConnect() {
  WiFi.begin(ssid, password);
  delay(5000);
  if (WiFi.status() == WL_CONNECTED) {
    colorBlinking(0, 255, 0, 3);
    return true;
  }
  return false;
}

bool wiFiDisconnect() {
  WiFi.disconnect(true);
  delay(5000);
  if (WiFi.status() == WL_DISCONNECTED) {
    colorBlinking(255, 0, 0, 3);
    return true;
  }
  return false;
}

void colorBlinking(int r, int g, int b, int count) {
  for (int i = 0; i < count; i++) {
    colorFill(r, g, b);
    delay(50);
    colorFill(0, 0, 0);
    delay(50);
  }
  colorFill(0, 0, 0);
}

void colorFill(int r, int g, int b) {
  colorFill(strip.numPixels(), r, g, b);
}

void colorFill(int count, int r, int g, int b) {
  for (int i = 0; i <= count - 1; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
    strip.show();
  }
}
