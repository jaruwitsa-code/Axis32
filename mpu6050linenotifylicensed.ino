/*
  ระบบเฝ้าติดตามการล้มและขอความช่วยเหลือฉุกเฉิน
  Copyright (C) 2026  Axis32Team
aaaass
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include "config.h"

Adafruit_MPU6050 mpu;
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_BROKER, 1883, wifiClient);
uint32_t last_publish;

#define sdagyro 38
#define sclgyro 39
#define buzzer 45
#define sw 48

// Logic สำหรับ MH-FMD (Active Low)
#define BUZZER_ON  LOW   
#define BUZZER_OFF HIGH  


#define TOPIC_B1    TOPIC_PREFIX "/board1"
#define TOPIC_B1_BTN  TOPIC_PREFIX "/board1/button"
#define TOPIC_B1_SND TOPIC_PREFIX "/board1/snd"
String CHANNEL_ACCESS_TOKEN = "2enwjEglyoTA7/3J614AkOqTC9d1nNHSeRh47/m9av1talUf+TpNLo8DMpwYy88dDpzPXIPTx1f/Btzsp3apBKsiFP8iq3Aaaq+VqdL/4bISXaJsGDpbXyGiD1SOnjjFi9UZyxyrfWY6FVfGzutvAAdB04t89/1O/w1cDnyilFU=";
String USER_ID = "U3984de2d34c547f1238de4cd6f6b9548"; 

const float FALL_LOW = 0.3;  
const float FALL_HIGH = 5.5; 
bool isFallen = false;

// ตัวแปรสำหรับคุมเวลาการ Print
unsigned long lastPrintTime = 0;
unsigned long lastPrintTimeL = 0;
const int printInterval = 1000; // พิมพ์ทุกๆ 100ms (10 ครั้งต่อวินาที)
const int printIntervalL = 10000;

void connect_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  printf("WiFi MAC address is %s\n", WiFi.macAddress().c_str());
  printf("Connecting to WiFi %s.\n", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    printf(".");
    fflush(stdout);
    delay(500);
  }
  printf("\nWiFi connected.\n");
}

void connect_mqtt() {
  printf("Connecting to MQTT broker at %s.\n", MQTT_BROKER);
  if (!mqtt.connect("", MQTT_USER, MQTT_PASS)) {
    printf("Failed to connect to MQTT broker.\n");
    for (;;) {} // wait here forever
  }
  mqtt.setCallback(mqtt_callback);
  printf("MQTT broker connected.\n");
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
}

void snd_callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "board1/buzzer") {
    if (message == "ON") {
      digitalWrite(BUZZER_PIN, HIGH); // เสียงดัง
    } else {
      digitalWrite(BUZZER_PIN, LOW);  // เสียงเงียบ
    }
  }
}

void setup() {
  Serial.begin(115200); 
  
  pinMode(sw, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, BUZZER_OFF); 
  connect_wifi();
  connect_mqtt();
  last_publish = 0;

  Wire.begin(sdagyro, sclgyro);
  if (!mpu.begin(0x68)) {
    Serial.println("MPU6050 Connection Failed!");
    while (1) delay(10);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  Serial.println("================================");
  Serial.println("   FALL DETECTION SYSTEM READY  ");
  Serial.println("================================");
}



void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // คำนวณค่า Magnitude
  float mag = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2)) / 9.81;

  // 1. ตรวจจับการล้ม
  if (!isFallen && mag < FALL_LOW) {
    unsigned long start = millis();
    while (millis() - start < 500) {
      mpu.getEvent(&a, &g, &temp);
      float mag_check = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2)) / 9.81;
      if (mag_check > FALL_HIGH) {
        isFallen = true;
        Serial.println("\n[!] >>> FALL DETECTED! <<< [!]");
        break;
      }
    }
  }

  // 2. ควบคุม Buzzer และปุ่มกด
  bool buttonPressed = (digitalRead(sw) == HIGH);

  
  if (isFallen) {
    digitalWrite(buzzer, BUZZER_ON);
    if (buttonPressed) {
      isFallen = false;
      sendLineMessage("🟢🟢🟢 BUTTON PRESSED 🟢🟢🟢");
      digitalWrite(buzzer, BUZZER_OFF);
      Serial.println("\n[R] >>> SYSTEM RESET <<<");
      delay(500); 
    }
  } else {
    digitalWrite(buzzer, buttonPressed ? BUZZER_ON : BUZZER_OFF);
  }

  if (millis() - lastPrintTimeL >= printIntervalL) {
    lastPrintTimeL = millis();
    if (isFallen) {
      sendLineMessage("🚑🚑🚑 FALL DETECTED 🚑🚑🚑");
      }
  }

  // 3. ส่วนการส่งค่า Serial Print แบบวนซ้ำ (ไม่กระทบ Loop)
  if (millis() - lastPrintTime >= printInterval) {
    lastPrintTime = millis();
    
    Serial.print("Accel: ");
    Serial.print(mag, 2); // แสดงทศนิยม 2 ตำแหน่ง
    Serial.print(" g | ");
    
    Serial.print("Status: ");
    Serial.print(isFallen ? "ALARM " : "NORMAL");
    Serial.print(" | ");
    
    Serial.print("Button: ");
    Serial.println(buttonPressed ? "DOWN" : "UP");
    mqtt.publish(TOPIC_B1_BTN, buttonPressed ? "1" : "0");

    if (isFallen) {
      mqtt.publish(TOPIC_B1, "1");
    } else {
      mqtt.publish(TOPIC_B1, "0");
    }
  }
}
void sendLineMessage(String msg) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.line.me/v2/bot/message/push");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + CHANNEL_ACCESS_TOKEN);

    String payload = "{";
    payload += "\"to\":\"" + USER_ID + "\",";
    payload += "\"messages\":[{\"type\":\"text\",\"text\":\"" + msg + "\"}]";
    payload += "}";

    int code = http.POST(payload);
    Serial.print("HTTP Response code: ");
    Serial.println(code);

    if (code > 0) {
      String res = http.getString();
      Serial.println(res);
    } else {
      Serial.println("ส่งไม่สำเร็จ!");
    }

    http.end();
  } else {
    Serial.println("WiFi disconnected");
  }
}
