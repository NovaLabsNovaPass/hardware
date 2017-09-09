#include <U8g2lib.h>

#define DISP_MAXL 5
#define DISP_MAXCH 32

class Disp {
  public:
    void Setup(void);
    void Writeln(uint8_t line, String str);
    void Writeln(uint8_t line, char *cp);
    void Writeln(uint8_t line, const char *cp);
    void WriteStr(uint8_t line, String s);
    char lines[DISP_MAXL][DISP_MAXCH];
    void Show(void);
    void Clear(void);
};



