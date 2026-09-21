#ifndef NGHEGOI_H
#define NGHEGOI_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>

// Tham chiếu các biến phần cứng từ file chính
extern const int NUT_LEN;
extern const int NUT_XUONG;
extern const int NUT_CHON;
extern const int NUT_QUAY_LAI;

extern HardwareSerial A7680C;
extern Adafruit_SSD1306 man_hinh;

// Danh sách các hàm gọi / nhận cuộc gọi
void sendATCommand(String cmd, int timeout);
void cuocgoiden(String so);
void danggoi(String so);
void veGiaoDien(String so, String trang_thai);

#endif // NGHEGOI_H