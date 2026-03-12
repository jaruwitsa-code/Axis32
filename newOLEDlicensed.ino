/*
  ระบบเฝ้าติดตามการล้มและขอความช่วยเหลือฉุกเฉิน
  Copyright (C) 2026  Axis32Team

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

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h" // ตรวจสอบว่าในไฟล์นี้มี MQTT_BROKER, WIFI_SSID, WIFI_PASS และ TOPIC_PREFIX

// --- ตั้งค่าจอ OLED I2C ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- ตั้งค่า MQTT และ Network ---
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient); // ใช้ตัวแปรนี้จัดการ MQTT

#define TOPIC_B1    TOPIC_PREFIX "/board1"
#define TOPIC_B1_BTN  TOPIC_PREFIX "/board1/button"

int remoteButtonStatus = 0;

// --- กำหนดขา Pin ---
const int buzzerPin = 11; 
const int sdaOled = 48;
const int sclOled = 47;    
const int ledPin = 42;  
const int greenPin = 40;  

// --- ตัวแปรสถานะ (0 = ปกติ, 1 = ตรวจพบการล้ม) ---
int networkStatus = 0; 

// ฟังก์ชันเชื่อมต่อ WiFi
void connect_wifi() {
  Serial.printf("Connecting to WiFi %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

// ฟังก์ชัน Callback: จะทำงานเมื่อมีข้อความเข้าจาก MQTT Broker
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String message = String((char*)payload);
  String strTopic = String(topic);

  if (strTopic == TOPIC_B1) {
    networkStatus = (message == "1") ? 1 : 0;
  } 
  else if (strTopic == TOPIC_B1_BTN) {
    remoteButtonStatus = (message == "1") ? 1 : 0;
  }
}

// ฟังก์ชันเชื่อมต่อ MQTT Broker
void connect_mqtt() {
  while (!mqtt.connected()) {
    if (mqtt.connect("Board2_Receiver", MQTT_USER, MQTT_PASS)) {
      mqtt.subscribe(TOPIC_B1);      // ติดตามการล้ม
      mqtt.subscribe(TOPIC_B1_BTN);  // ติดตามการกดปุ่ม
      Serial.println("Subscribed to all topics");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // ตั้งค่าขา Output
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledPin, LOW);

  // เริ่มต้นการเชื่อมต่อ
  connect_wifi();
  mqtt.setServer(MQTT_BROKER, 1883);
  mqtt.setCallback(mqtt_callback); // ผูกฟังก์ชัน callback

  // เริ่มต้น I2C และจอ OLED
  Wire.begin(sdaOled, sclOled);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("System Monitoring...");
  display.display();
  delay(2000);
}

void loop() {
  // ตรวจสอบสถานะการเชื่อมต่อ MQTT
  if (!mqtt.connected()) {
    connect_mqtt();
  }
  mqtt.loop(); // สำคัญมาก: ต้องมีเพื่อให้ระบบรับข้อมูล MQTT ทำงานได้

  // --- การแสดงผลและการแจ้งเตือนตามสถานะ networkStatus ---
  display.clearDisplay();
  display.setCursor(0, 10);
  
  if (networkStatus == 1) {
    // สถานะเตือนภัย (จาก Board 1)
    display.setTextSize(2);
    display.println("!! FALL !!");
    display.setTextSize(1);
    display.println("\nCheck Patient Now!");
    
    // แจ้งเตือนด้วยแสงและเสียง
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH); 
    digitalWrite(greenPin ,LOW);
  } 
  else {
    // สถานะปกติ
    display.setTextSize(2);
    display.println(" NORMAL ");
    display.setTextSize(1);
    display.println("\nNo falling detected");
    
    // ปิดการแจ้งเตือน
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    digitalWrite(greenPin ,HIGH);
    
  }

  display.display();
  delay(100); // หน่วงเวลาสั้นๆ เพื่อลดการกระพริบของจอ
}