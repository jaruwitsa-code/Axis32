=====================================================================================
ระบบเฝ้าติดตามการล้มและขอความช่วยเหลือฉุกเฉิน
(Fall Detection and Emergency Alert System)
=====================================================================================
Platform: Arduino ESP32 | License: GPLv3 | Hardware design: EasyEDA | MQTT: Node-RED
=====================================================================================

ผู้จัดทำ
ทีม Axis32
ภาควิชาวิศวกรรมคอมพิวเตอร์ คณะวิศวกรรมศาสตร์ มหาวิทยาลัยเกษตรศาสตร์
รายชื่อสมาชิก:
1. กรวิชญ์ อัศวเพิ่มพูลผล รหัสนิสิต 6810503331
2. จารุวิทย์ แสงประดับ รหัสนิสิต 6810503439
3. นัทธพงศ์ ลำมะนิตย์ รหัสนิสิต 6810503633
4. อริยธัช ธีรพิทยานนท์ รหัสนิสิต 6810504036

ภาพรวมโครงงาน
เป็นระบบสำหรับเฝ้าติดตามการล้มสำหรับผู้สูงอายุแบบเรียลไทม์ โดยใช้เซนเซอร์ MPU6050 
ตรวจจับการล้ม และแจ้งเตือนผ่านหลายช่องทาง ได้แก่:
- Buzzer
- RED and GEEN LEDs
- หน้าจอ OLED
- หน้าเว็บ Dashboard (Node-RED)
- แจ้งเตือนผ่าน LINE Notify

ระบบการทำงาน
ระบบประกอบด้วยบอร์ดหลัก 2 บอร์ดทำงานร่วมกันผ่าน MQTT:

┌─────────────────┐    MQTT      ┌─────────────────┐
│     บอร์ดที่ 1           ───────────►      บอร์ดที่ 2      	  │
│  (Sensor Board)      │   (WiFi)     │  (Display Board)	  │
└─────────────────┘              └─────────────────┘
       │                                  │
       ▼                                  ▼
  MPU6050, Buzzer                  OLED Display, LED, Buzzer
  LINE Notify, Node-RED



โครงสร้างไฟล์
fall-detection-system-Axis32/
│
├── newOLEDlicensed.ino                 # โค้ดสำหรับบอร์ดที่ 2 (แสดงผล OLED และการแจ้งเตือน)
├── mpu6050linenotifylicensed.ino       # โค้ดสำหรับบอร์ดที่ 1 (เซนเซอร์และการแจ้งเตือน)
├── LICENSE                             # สัญญาอนุญาต GNU GPLv3
├── README.txt                          # เอกสารนี้
│
├── schematics/
│   ├── Board 1 schematics.pdf          # ผังวงจรบอร์ดที่ 1 (ออกแบบผ่านโปรแกรม EasyEDA)
│   └── Board 2 schematics.pdf          # ผังวงจรบอร์ดที่ 2 (ออกแบบผ่านโปรแกรม EasyEDA)


รายละเอียดไฟล์ซอร์สโค้ด
1. mpu6050linenotifylicensed.ino (บอร์ดที่ 1)
-----------------------------------------------------------
หน้าที่หลัก:
- อ่านการกดปุ่มเพื่อส่งเสียงขอความช่วยเหลือ
- อ่านค่าจากเซนเซอร์ MPU6050 (accelerometer + gyroscope)
- ประมวลผลเพื่อตรวจจับการล้ม (fall detection algorithm)
- ส่งสถานะผ่าน MQTT ไปยังบอร์ดที่ 2 และ Node-RED
- ส่งการแจ้งเตือนผ่าน LINE Notify เมื่อเกิดการล้ม
- แจ้งเตือน Buzzers

Hardware ที่ใช้:
- ESP32S3
- MPU6050 (เชื่อมต่อผ่าน I2C)
- Buzzer
- Switch


2. newOLEDlicensed.ino (บอร์ดที่ 2)
-----------------------------------------------------------
หน้าที่หลัก:
- รับข้อความจาก MQTT ที่ส่งมาจากบอร์ดที่ 1
- แสดงสถานะและข้อความเตือนบนหน้าจอ OLED
- แสดงเวลาที่เกิดเหตุการณ์
- แจ้งเตือนเสียง Buzzer
- แจ้งเตือนแสง LED

Hardware ที่ใช้:
- ESP32 (หรือบอร์ดที่รองรับ WiFi)
- หน้าจอ OLED (SSD1306, I2C)
- Buzzer
- LED


แบบผังวงจร (Schematics)
ไฟล์ PDF ทั้งสองแบบออกแบบด้วย EasyEDA สามารถเปิดดูหรือแก้ไขต่อได้ 
ด้วยโปรแกรม EasyEDA หรือโปรแกรมดู PDF ทั่วไป

- Board 1 schematics.pdf : ผังวงจรบอร์ดหลัก (ESP32 + MPU6050 + Buzzer + Switch)
- Board 2 schematics.pdf : ผังวงจรบอร์ดแสดงผล (ESP32 + OLED + Buzzer + LED)


การติดตั้งและใช้งาน

ความต้องการเบื้องต้น:
- Arduino IDE (พร้อมติดตั้งบอร์ด ESP32)
- บัญชี LINE Notify (สำหรับรับแจ้งเตือน)
- MQTT Broker (เช่น MQTTx)
- Node-RED (สำหรับ Dashboard)

ขั้นตอนการติดตั้ง:

1. ติดตั้งไลบรารีที่จำเป็นใน Arduino IDE
   - MPU6050 by Jeff Rowberg
   - PubSubClient (สำหรับ MQTT)
   - Adafruit SSD1306 (สำหรับ OLED)
   - WiFiClient

2. ตั้งค่า WiFi และ MQTT
   - แก้ไข SSID, Password ในไฟล์ .ino ทั้งสอง
   - ตั้งค่า MQTT Broker และ Topic ให้ตรงกัน
   - ตัวอย่าง MQTT Topic: "fall_detection/status"

3. ตั้งค่า LINE Notify
   - ขอ Line Token จาก https://notify-bot.line.me
   - ใส่ Token ในไฟล์ mpu6050linenotifylicensed.ino

4. อัปโหลดโค้ด
   - อัปโหลด mpu6050linenotifylicensed.ino ไปยังบอร์ดที่ 1
   - อัปโหลด newOLEDlicensed.ino ไปยังบอร์ดที่ 2

5. ตั้งค่า Node-RED (ถ้าต้องการ)
   - รับ MQTT และสร้าง Dashboard สำหรับแสดงสถานะ


สัญญาอนุญาต
-----------------------------------------------------------
โปรเจคนี้อยู่ภายใต้สัญญาอนุญาต GNU General Public License v3.0

Project: Fall Detection and Emergency Alert System
Copyright 2024 Axis32 Team - Computer Engineering, Kasetsart University

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

ดูรายละเอียดเพิ่มเติมได้ที่ไฟล์ LICENSE
-----------------------------------------------------------

ถ้าคุณพบว่าโปรเจคนี้มีประโยชน์ สามารถแชร์หรือนำไปพัฒนาต่อได้ 
ตามข้อกำหนดของสัญญาอนุญาต GPLv3