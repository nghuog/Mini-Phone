#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>
#include "nghegoi.h"

HardwareSerial A7680C(1);
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SIM_RX_PIN D7  // Nối vào TX của A7680C
#define SIM_TX_PIN D6  // Nối vào RX của A7680C

Adafruit_SSD1306 man_hinh(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
String phan_hoi = "";
String sdt = "";
int so_chon = 0;

const int NUT_LEN      = 1; // Chân D0
const int NUT_XUONG    = 2; // Chân D1
const int NUT_CHON     = 3; // Chân D2 (K3)
const int NUT_QUAY_LAI = 4; // Chân D3 (K4)

int trang_hien_tai = 0; 
int muc_trang_chu   = 1;  
unsigned long thoi_gian_nhan = 0;
bool da_giu_nut = false;

// Dữ liệu ảnh bitmap menu
static const unsigned char PROGMEM image_message_mail_bits[] = {
  0x00,0x00,0x00,0x7f,0xff,0x00,0xc0,0x01,0x80,0xe0,0x03,0x80,
  0xb0,0x06,0x80,0x98,0x0c,0x80,0x8c,0x18,0x80,0x86,0x30,0x80,
  0x83,0x60,0x80,0x85,0xd0,0x80,0x88,0x08,0x80,0x90,0x04,0x80,
  0xa0,0x02,0x80,0xc0,0x01,0x80,0x7f,0xff,0x00,0x00,0x00,0x00
};

static const unsigned char PROGMEM image_phone_book_closed_bits[] = {
  0x7f,0xfc,0xc0,0x06,0xff,0xf2,0x80,0x0a,0x80,0x0a,0x9f,0xea,
  0x90,0x2a,0x9f,0xea,0x80,0x0a,0x80,0x0a,0x8f,0x8a,0x80,0x0a,
  0x80,0x0a,0x8f,0x8a,0x80,0x0c,0x7f,0xf8
};

static const unsigned char PROGMEM image_phone_call_in_progress_bits[] = {
  0x38,0xf0,0x44,0x08,0x84,0xe4,0x84,0x12,0x8c,0xca,0x88,0x2a,
  0x44,0x28,0x42,0x00,0x21,0x00,0x30,0xbc,0x18,0x62,0x0c,0x02,
  0x06,0x02,0x01,0x84,0x00,0x78,0x00,0x00
};

static const unsigned char PROGMEM image_menu_settings_sliders_bits[] = {
  0x38,0x00,0x44,0x00,0xc7,0xfc,0x44,0x00,0x38,0x00,0x00,0x70,
  0x00,0x88,0xff,0x8c,0x00,0x88,0x00,0x70,0x38,0x00,0x44,0x00,
  0xc7,0xfc,0x44,0x00,0x38,0x00,0x00,0x00
};

void dienthoai();
void danhba();
void congcu();
void tinnhan();

void setup() {
  pinMode(NUT_LEN, INPUT_PULLUP);
  pinMode(NUT_XUONG, INPUT_PULLUP);
  pinMode(NUT_CHON, INPUT_PULLUP);
  pinMode(NUT_QUAY_LAI, INPUT_PULLUP);

  if (!man_hinh.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  man_hinh.clearDisplay();
  man_hinh.display();

  A7680C.begin(115200, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
  delay(3000);

menu.iso  sendATCommand("AT", 1000);[cite: 3]
  sendATCommand("AT+CPIN?", 1000);[cite: 3]
  sendATCommand("AT+CLIP=1", 1000);[cite: 3]
}

void loop() {
  // 1. Bắt cuộc gọi đến
  if (A7680C.available()) {
    String dong_tin = A7680C.readStringUntil('\n');
    dong_tin.trim();

    if (dong_tin.indexOf("+CLIP:") >= 0) {
      String so_goi_den = "Khong ro";
      
      int vi_tri_dau = dong_tin.indexOf("\"");
      int vi_tri_cuoi = dong_tin.indexOf("\"", vi_tri_dau + 1);

      if (vi_tri_dau >= 0 && vi_tri_cuoi > vi_tri_dau) {
        so_goi_den = dong_tin.substring(vi_tri_dau + 1, vi_tri_cuoi);
      }

      cuocgoiden(so_goi_den);[cite: 3]
    }
  }

  // 2. Menu chính
  if (trang_hien_tai == 0) {
    if (digitalRead(NUT_XUONG) == LOW) {
      delay(150);
      muc_trang_chu++;
      if (muc_trang_chu > 4) muc_trang_chu = 1;
    }
    if (digitalRead(NUT_LEN) == LOW) {
      delay(150);
      muc_trang_chu--;
      if (muc_trang_chu < 1) muc_trang_chu = 4;
    }
    if (digitalRead(NUT_CHON) == LOW) {
      delay(150);
      trang_hien_tai = muc_trang_chu;
    }

    man_hinh.clearDisplay();
    man_hinh.setTextColor(SSD1306_WHITE);
    man_hinh.setTextSize(1);

    if (muc_trang_chu == 1)      dienthoai();
    else if (muc_trang_chu == 2) danhba();
    else if (muc_trang_chu == 3) congcu();
    else if (muc_trang_chu == 4) tinnhan();

    man_hinh.display();
  }

  // 3. Màn hình bấm số gọi đi
  else if (trang_hien_tai == 1) {
    if (digitalRead(NUT_LEN) == LOW) {
      delay(150);
      so_chon++;
      if (so_chon > 9) so_chon = 0;
    }

    if (digitalRead(NUT_XUONG) == LOW) {
      delay(150);
      so_chon--;
      if (so_chon < 0) so_chon = 9;
    }

    if (digitalRead(NUT_CHON) == LOW) {
      if (thoi_gian_nhan == 0) {
        thoi_gian_nhan = millis();
        da_giu_nut = false;
      }

      if ((millis() - thoi_gian_nhan >= 1000) && !da_giu_nut) {
        if (sdt.length() > 0) {
          danggoi(sdt);[cite: 3]
          da_giu_nut = true;
          thoi_gian_nhan = 0;
        }
      }
    } else {
      if (thoi_gian_nhan > 0) {
        if (!da_giu_nut) {
          sdt = sdt + String(so_chon);
        }
        thoi_gian_nhan = 0;
      }
    }

    if (digitalRead(NUT_QUAY_LAI) == LOW) {
      delay(150);
      if (sdt.length() > 0) {
        sdt.remove(sdt.length() - 1);
      } else {
        trang_hien_tai = 0;
      }
    }

    man_hinh.clearDisplay();
    man_hinh.setTextColor(SSD1306_WHITE);
    man_hinh.setTextSize(1);
    man_hinh.setCursor(0, 0);
    man_hinh.print("SDT: ");
    man_hinh.print(sdt);

    man_hinh.setTextSize(2);
    man_hinh.setCursor(55, 25);
    man_hinh.print(so_chon);

    man_hinh.setTextSize(1);
    man_hinh.setCursor(0, 52);
    man_hinh.print("K3:Them(Giu) K4:Xoa/Ve");

    man_hinh.display();
  }

  // 4. Các menu con khác
  else {
    if (digitalRead(NUT_QUAY_LAI) == LOW) {
      delay(150);
      trang_hien_tai = 0;
    }

    man_hinh.clearDisplay();
    man_hinh.setTextColor(SSD1306_WHITE);
    man_hinh.setTextSize(1);
    man_hinh.setCursor(20, 25);
    man_hinh.print("Trang con: ");
    man_hinh.print(trang_hien_tai);
    man_hinh.setCursor(20, 45);
    man_hinh.print("Nhan K4 de ve");
    man_hinh.display();
  }
}

void dienthoai() {
  man_hinh.drawBitmap(57, 24, image_phone_call_in_progress_bits, 15, 16, SSD1306_WHITE);
  man_hinh.setTextWrap(false);
  man_hinh.setCursor(41, 43);
  man_hinh.print("goi dien");
  man_hinh.drawBitmap(0, 24, image_message_mail_bits, 17, 16, SSD1306_WHITE);
  man_hinh.drawBitmap(113, 24, image_phone_book_closed_bits, 15, 16, SSD1306_WHITE);
}

void danhba() {
  man_hinh.drawBitmap(0, 24, image_phone_call_in_progress_bits, 15, 16, SSD1306_WHITE);
  man_hinh.setTextWrap(false);
  man_hinh.setCursor(44, 43);
  man_hinh.print("danh ba");
  man_hinh.drawBitmap(114, 24, image_menu_settings_sliders_bits, 14, 16, SSD1306_WHITE);
  man_hinh.drawBitmap(57, 24, image_phone_book_closed_bits, 15, 16, SSD1306_WHITE);
}

void congcu() {
  man_hinh.drawBitmap(0, 24, image_phone_book_closed_bits, 15, 16, SSD1306_WHITE);
  man_hinh.drawBitmap(57, 24, image_menu_settings_sliders_bits, 14, 16, SSD1306_WHITE);
  man_hinh.drawBitmap(111, 24, image_message_mail_bits, 17, 16, SSD1306_WHITE);
  man_hinh.setTextWrap(false);
  man_hinh.setCursor(44, 43);
  man_hinh.print("cong cu");
}

void tinnhan() {
  man_hinh.drawBitmap(0, 24, image_menu_settings_sliders_bits, 14, 16, SSD1306_WHITE);
  man_hinh.drawBitmap(56, 24, image_message_mail_bits, 17, 16, SSD1306_WHITE);
  man_hinh.drawBitmap(113, 24, image_phone_call_in_progress_bits, 15, 16, SSD1306_WHITE);
  man_hinh.setTextWrap(false);
  man_hinh.setCursor(41, 43);
  man_hinh.print("tin nhan");
}