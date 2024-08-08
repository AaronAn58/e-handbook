#ifndef EPDIF_H
#define EPDIF_H

// Pin definition
#define RST_PIN 8
#define DC_PIN 9
#define CS_PIN 10
#define BUSY_PIN 7

class EpdIf
{
public:
    EpdIf();  // 构造函数
    ~EpdIf(); // 折构函数

    static int IfInit(void);
    static void DigitalWrite(int pin, int value);
    static int DigitalRead(int pin);
    static void DelayMs(unsigned int delaytime);
    static void SpiTransfer(unsigned char data);
};

#endif