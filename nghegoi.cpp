#include "nghegoi.h"

// Ảnh bitmap (PROGMEM) khuôn mặt khi đàm thoại
static const unsigned char PROGMEM image_EviSmile1_bits[] = {
  0x30,0x03,0x00,0x60,0x01,0x80,0xe0,0x01,0xc0,0xf3,0xf3,0xc0,
  0xff,0xff,0xc0,0xff,0xff,0xc0,0x7f,0xff,0x80,0x7f,0xff,0x80,
  0x7f,0xff,0x80,0xef,0xfd,0xc0,0xe7,0xf9,0xc0,0xe3,0xf1,0xc0,
  0xe1,0xe1,0xc0,0xf1,0xe3,0xc0,0xff,0xff,0xc0,0x7f,0xff,0x80,
  0x7b,0xf7,0x80,0x3d,0x2f,0x00,0x1e,0x1e,0x00,0x0f,0xfc,0x00,
  0x03,0xf0,0x00
};

// Gửi lệnh AT (Lệnh điều khiển module SIM)
void sendATCommand(String cmd, int timeout) {
  A7680C.println(cmd);
  long int time = millis();
  while ((time + timeout) > millis()) {
    while (A7680C.available()) {
      char c = A7680C.read();
      Serial.write(c);
    }
  }
}

// Vẽ giao diện khi đàm thoại
void veGiaoDien(String so, String trang_thai) {
  man_hinh.clearDisplay();
  man_hinh.drawBitmap(7, 7, image_EviSmile1_bits, 18, 21, 1);
  man_hinh.setTextColor(1);
  man_hinh.setTextWrap(false);
  man_hinh.setTextSize(1);
  man_hinh.setCursor(30, 10);
  man_hinh.print(so);
  man_hinh.setCursor(35, 29);
  man_hinh.print(trang_thai);
  man_hinh.display();
}

// Xử lý cuộc gọi đến (Incoming Call)
void cuocgoiden(String so) {
  man_hinh.clearDisplay();
  man_hinh.setTextColor(SSD1306_WHITE);
  man_hinh.setTextSize(1);
  man_hinh.setCursor(0, 5);
  man_hinh.print("Goi den tu:");
  man_hinh.setCursor(0, 22);
  man_hinh.setTextSize(1);
  man_hinh.print(so);
  man_hinh.setCursor(0, 42);
  man_hinh.print("K3: NGHE   K4: TU CHOI");
  man_hinh.display();

  String phan_hoi_den = "";

  while (true) {
    while (A7680C.available()) {
      char a = A7680C.read();
      phan_hoi_den += a;
      if (phan_hoi_den.indexOf("NO CARRIER") >= 0) {
        return;
      }
    }

    // Nhấn K3 để nghe máy
    if (digitalRead(NUT_CHON) == LOW) {
      delay(200);
      A7680C.println("ATA");
      veGiaoDien(so, "Dang dam thoai");
      
      while (true) {
        while (A7680C.available()) {
          char a = A7680C.read();
          phan_hoi_den += a;
          if (phan_hoi_den.indexOf("NO CARRIER") >= 0) return;
        }

        // Nhấn K4 để dập máy
        if (digitalRead(NUT_QUAY_LAI) == LOW) {
          delay(200);
          A7680C.println("AT+CHUP");
          return;
        }
      }
    }

    // Nhấn K4 để từ chối
    if (digitalRead(NUT_QUAY_LAI) == LOW) {
      delay(200);
      A7680C.println("AT+CHUP");
      return;
    }
  }
}

// Xử lý gọi đi (Outgoing Call)
void danggoi(String so) {
  String phan_hoi = "";
  A7680C.println("ATD" + so + ";");

  veGiaoDien(so, "Dang goi...");

  unsigned long timerKiemTra = millis();

  while (true) {
    while (A7680C.available()) {
      char a = A7680C.read();
      phan_hoi += a;

      if (phan_hoi.indexOf("NO CARRIER") >= 0 || 
          phan_hoi.indexOf("BUSY") >= 0 || 
          phan_hoi.indexOf("NO ANSWER") >= 0) {
        phan_hoi = "";
        return;
      }

      if (phan_hoi.indexOf("VOICE CALL: BEGIN") >= 0 || 
          phan_hoi.indexOf("MO CONNECT") >= 0 || 
          phan_hoi.indexOf("+CLCC: 1,0,0") >= 0) {
        veGiaoDien(so, "Da nghe may");
        phan_hoi = "";
      }
    }

    if (millis() - timerKiemTra >= 1000) {
      A7680C.println("AT+CLCC");
      timerKiemTra = millis();
    }

    if (digitalRead(NUT_QUAY_LAI) == LOW) {
      delay(200);
      A7680C.println("AT+CHUP");
      phan_hoi = "";
      break; 
    }
  }
}