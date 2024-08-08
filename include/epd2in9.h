#ifndef EPD2IN9_H
#define EPD2IN9_H

#include "epdif.h"

// Display resolution
#define EPD_WIDTH       128
#define EPD_HEIGHT      296

class Epd : EpdIf {
public:
    unsigned long width;
    unsigned long height;

    Epd();
    ~Epd();
    int Init(void);
    void SendCommand(unsigned char command);
    void SendData(unsigned char data);
    void WaitUntilIdle(void);
    void Reset(void);

private:
    unsigned int reset_pin;
    unsigned int dc_pin;
    unsigned int cs_pin;
    unsigned int busy_pin;
};

#endif
