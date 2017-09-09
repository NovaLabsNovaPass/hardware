#include "Disp.h"

extern U8G2_SH1106_128X64_NONAME_1_SW_I2C u8g2;

void Disp::Setup(void) {
  uint8_t ypos;
  u8g2.begin(); // display
}

void Disp::Writeln(uint8_t line_no, String str) {
  char cbuf[32];

  str.toCharArray(cbuf,32);

  if (line_no < DISP_MAXL) {
    strncpy(lines[line_no],cbuf,DISP_MAXCH);
  }
}

void Disp::Writeln(uint8_t line_no, char *cp) {
  if (line_no < DISP_MAXL) {
    strncpy(lines[line_no],cp,DISP_MAXCH);
  }
}

void Disp::Writeln(uint8_t line_no, const char *cp) {
  if (line_no < DISP_MAXL) {
    strncpy(lines[line_no],cp,DISP_MAXCH);
  }
}

void Disp::WriteStr(uint8_t line_no, String s) {
  if (line_no < DISP_MAXL) {
    s.toCharArray(lines[line_no],DISP_MAXCH);
  }
}

void Disp::Show(void) {

  uint8_t i,starty;

  for (i = 0; i < DISP_MAXL; i++) {
      SerialUSB.println(lines[i]);
  }
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_helvB10_tf);
    starty = 12;
    for (i = 0; i < DISP_MAXL; i++, starty += 12) {
      if (lines[i]) {
        if (strlen(lines[i]) > 16)
          u8g2.setFont(u8g2_font_helvB08_tf);
        else
          u8g2.setFont(u8g2_font_helvB10_tf);
        u8g2.drawStr(0,starty,lines[i]);
      }
    }
  } while ( u8g2.nextPage() );

}

void Disp::Clear(void) {
   uint8_t i;

    for (i = 0; i < DISP_MAXL; i++) {
      lines[i][0] = (char)0;
    }

    u8g2.clearDisplay();
}
