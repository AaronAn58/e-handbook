#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <EPD.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <SD_MMC.h>
#include <DEV_Config.h>
#include <EPD_2in9_V2.h>

#define TEST_FILE_SIZE (4 * 1024 * 1024)
#define SD_SCK 18
#define SD_MISO 15
#define SD_MOSI 13
#define SD_CS 14

const char *ssid = "TP-LINK_7E2B12";
const char *password = "20DCE67E2B12";
IPAddress staticIP(192, 168, 3, 159);
IPAddress gateway(192, 168, 3, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(233, 5, 5, 5);
IPAddress dns2(192, 168, 18, 1);

SPIClass spi = SPIClass(VSPI);

uint8_t startWiFi()
{
  Serial.println("\r\nConnecting to: " + String(ssid));
  if (WiFi.config(staticIP, gateway, subnet, dns, dns2) == false)
  {
    Serial.println("Configuration failed.");
  }
  Serial.println("wifi config success.");
  WiFi.begin(ssid, password);
  Serial.println("WiFi connect status: " + (String)WiFi.waitForConnectResult());
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(500);
    WiFi.begin(ssid, password);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    int wifi_signal = WiFi.RSSI();
    Serial.println("WiFi connected at: " + WiFi.localIP().toString());
  }
  else
    Serial.println("WiFi connection *** FAILED ***");
  return WiFi.status();
}

void StopWiFi()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

// boolean SetupTime() {
//   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov"); //(gmtOffset_sec, daylightOffset_sec, ntpServer)
//   setenv("TZ", Timezone, 1);  //setenv()adds the "TZ" variable to the environment with a value TimeZone, only used if set to 1, 0 means no change
//   tzset(); // Set the TZ environment variable
//   delay(100);
//   bool TimeStatus = UpdateLocalTime();
//   return TimeStatus;
// }

void testHttp(WiFiClient &client, const String &requestType)
{
  client.stop();
  HTTPClient http;
  Serial.print("start http request: \n");
  // http.begin(client, "https://www.baidu.com/");
  sleep(10);
  // http.begin("https://www.baidu.com/");
  http.begin("https://www.aliyun.com/");
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK)
  {
    Serial.println(http.getString());
  }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }
  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char *path)
{
  Serial.printf("Creating Dir: %s\n", path);
  boolean ret = fs.mkdir(path);
  Serial.println("result:" + String(ret));
  if (ret)
  { 
    Serial.println("Dir created");
  }
  else
  {
    Serial.println("mkdir failed");
  }
}
void testWriteFile(fs::FS &fs, const char *path, uint8_t *buf, int len)
{
  unsigned long start_time = millis();
  Serial.printf("Test write %s\n", path);

  fs::File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  int loop = TEST_FILE_SIZE / len;
  while (loop--)
  {
    boolean res = file.write(buf, len);
    Serial.println("Write result: " + String(res));
    if (!res)
    {
      Serial.println("Write failed");
      break;
    }
  }

  file.flush();
  file.close();

  unsigned long time_used = millis() - start_time;
  Serial.printf("Write file used: %d ms, %f KB/s\n", time_used, (float)TEST_FILE_SIZE / time_used);
}

void testReadFile(fs::FS &fs, const char *path, uint8_t *buf, int len)
{
  unsigned long start_time = millis();
  Serial.printf("Test read %s\n", path);
  fs::File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  int loop = TEST_FILE_SIZE / len;
  while (loop--)
  {
    if (!file.read(buf, len))
    {
      Serial.println("Read failed");
      return;
    }
  };
  file.close();
  unsigned long time_used = millis() - start_time;
  Serial.printf("Read file used: %d ms, %f KB/s\n", time_used, (float)TEST_FILE_SIZE / time_used);
}

void testIO(fs::FS &fs)
{
  uint8_t *buf = (uint8_t *)malloc(64 * 1024);
  if (buf == nullptr)
  {
    Serial.println("Failed to allocate memory for buffer");
    return;
  }
  memset(buf, 'A', 1024);  // 用字符 'A' 填充缓冲区

  testWriteFile(fs, "/test_1k.txt", buf, 1024);
  free(buf);
  // testWriteFile(fs, "/test_2k.txt", buf, 2 * 1024);

  // testReadFile(fs, "/test_1k.txt", buf, 1024);
  // testReadFile(fs, "/test_1k.txt", buf, 2 * 1024);
}

// void setup()
// {
//   spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
//   DEV_Module_Init();
//   if (!SD.begin(SD_CS, spi))
//   {
//     Serial.println("Card Mount Failed");
//     return;
//   }
//   uint8_t cardType = SD.cardType();
//   Serial.print("SD Type: ");
//   switch (cardType)
//   {
//   case CARD_NONE:
//     Serial.println("No SD card attached");
//     break;
//   case CARD_MMC:
//     Serial.println("MMC/SDSC");
//     break;
//   case CARD_SD:
//     Serial.println("SD/SDHC/SDXC");
//     break;
//   case CARD_SDHC:
//     Serial.println("SDHC/SDXC");
//   default:
//     Serial.println("Unknown");
//   }
//   uint64_t cardSize = SD.cardSize() / (1024 * 1024);
//   Serial.printf("SD Card Size: %lluMB\n", cardSize);
//   // testIO(SD);
//   // createDir(SD, "/test123456");
//   listDir(SD, "/", 0);
// }

// void setup()
// {
//   Serial.begin(115200);
//   struct tm timeinfo;
//   int currentHour = timeinfo.tm_hour;
//   int currentMin = timeinfo.tm_min;
//   int currentSec = timeinfo.tm_sec;

void setup() {
  DEV_Module_Init();
  EPD_2IN9_V2_Init();
  EPD_2IN9_V2_Clear();
  DEV_Delay_ms(500);
}

//   Serial.print("Current Time: " + (String)currentHour + ":" + (String)currentMin + ":" + (String)currentSec);
//   startWiFi();
//   WiFiClient wifi;
//   testHttp(wifi, "GET");
// }

void loop()
{
  // put your main code here, to run repeatedly:
}


void setup()
{
	printf("EPD_2IN9_V2_test Demo\r\n");
	DEV_Module_Init();

    printf("e-Paper Init and Clear...\r\n");
    EPD_2IN9_V2_Init();
    EPD_2IN9_V2_Clear();
    DEV_Delay_ms(500);

    //Create a new image cache
    UBYTE *BlackImage;
    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    UWORD Imagesize = ((EPD_2IN9_V2_WIDTH % 8 == 0)? (EPD_2IN9_V2_WIDTH / 8 ): (EPD_2IN9_V2_WIDTH / 8 + 1)) * EPD_2IN9_V2_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        while(1);
    }
    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2IN9_V2_WIDTH, EPD_2IN9_V2_HEIGHT, 270, WHITE);

#if 1   //show image for array  
    Paint_NewImage(BlackImage, EPD_2IN9_V2_WIDTH, EPD_2IN9_V2_HEIGHT, 270, WHITE);  
    printf("show image for array\r\n");
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_DrawBitMap(gImage_2in9);

    EPD_2IN9_V2_Display(BlackImage);
    DEV_Delay_ms(2000);
#endif

#if 1   // Drawing on the image
    printf("Drawing\r\n");
    //1.Select Image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    // 2.Drawing on the image
    printf("Drawing:BlackImage\r\n");
    Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);

    Paint_DrawLine(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawRectangle(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawCircle(45, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(105, 95, 20, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawLine(85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

    Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);

    Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);
    Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);

    Paint_DrawString_CN(130, 0,"你好abc", &Font12CN, BLACK, WHITE);
    Paint_DrawString_CN(130, 20, "微雪电子", &Font24CN, WHITE, BLACK);

    EPD_2IN9_V2_Display_Base(BlackImage);
    DEV_Delay_ms(2000);
#endif

#if 1   //Partial refresh, example shows time        
    printf("Partial refresh\r\n");
    PAINT_TIME sPaint_time;
    sPaint_time.Hour = 12;
    sPaint_time.Min = 34;
    sPaint_time.Sec = 56;
    UBYTE num = 20;
    for (;;) {
        sPaint_time.Sec = sPaint_time.Sec + 1;
        if (sPaint_time.Sec == 60) {
            sPaint_time.Min = sPaint_time.Min + 1;
            sPaint_time.Sec = 0;
            if (sPaint_time.Min == 60) {
                sPaint_time.Hour =  sPaint_time.Hour + 1;
                sPaint_time.Min = 0;
                if (sPaint_time.Hour == 24) {
                    sPaint_time.Hour = 0;
                    sPaint_time.Min = 0;
                    sPaint_time.Sec = 0;
                }
            }
        }
        Paint_ClearWindows(150, 80, 150 + Font20.Width * 7, 80 + Font20.Height, WHITE);
        Paint_DrawTime(150, 80, &sPaint_time, &Font20, WHITE, BLACK);

        num = num - 1;
        if(num == 0) {
            break;
        }
        EPD_2IN9_V2_Display_Partial(BlackImage);
        DEV_Delay_ms(500);//Analog clock 1s
    }

#endif

#if 1 // show image for array
    free(BlackImage);
    printf("show Gray------------------------\r\n");
    Imagesize = ((EPD_2IN9_V2_WIDTH % 4 == 0)? (EPD_2IN9_V2_WIDTH / 4 ): (EPD_2IN9_V2_WIDTH / 4 + 1)) * EPD_2IN9_V2_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        while(1);
    }
    EPD_2IN9_V2_Gray4_Init();
    printf("4 grayscale display\r\n");
    Paint_NewImage(BlackImage, EPD_2IN9_V2_WIDTH, EPD_2IN9_V2_HEIGHT, 90, WHITE);
    Paint_SetScale(4);
    Paint_Clear(0xff);
    
    Paint_DrawPoint(10, 80, GRAY4, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, GRAY4, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, GRAY4, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawLine(20, 70, 70, 120, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(20, 70, 70, 120, GRAY4, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, GRAY4, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawCircle(45, 95, 20, GRAY4, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(105, 95, 20, GRAY2, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawLine(85, 95, 125, 95, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawString_EN(10, 0, "waveshare", &Font16, GRAY4, GRAY1);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, GRAY3, GRAY1);
    Paint_DrawNum(10, 33, 123456789, &Font12, GRAY4, GRAY2);
    Paint_DrawNum(10, 50, 987654321, &Font16, GRAY1, GRAY4);
    Paint_DrawString_CN(150, 0,"你好abc", &Font12CN, GRAY4, GRAY1);
    Paint_DrawString_CN(150, 20,"你好abc", &Font12CN, GRAY3, GRAY2);
    Paint_DrawString_CN(150, 40,"你好abc", &Font12CN, GRAY2, GRAY3);
    Paint_DrawString_CN(150, 60,"你好abc", &Font12CN, GRAY1, GRAY4);
    Paint_DrawString_CN(150, 80, "微雪电子", &Font24CN, GRAY1, GRAY4);
    EPD_2IN9_V2_4GrayDisplay(BlackImage);
    DEV_Delay_ms(3000);

    Paint_NewImage(BlackImage, EPD_2IN9_V2_WIDTH, EPD_2IN9_V2_HEIGHT, 0, WHITE);
    Paint_SetScale(4);
    Paint_Clear(WHITE);
    Paint_DrawBitMap(gImage_2in9_Gray4);
    EPD_2IN9_V2_4GrayDisplay(BlackImage);
    DEV_Delay_ms(3000);

#endif

    printf("Clear...\r\n");
    EPD_2IN9_V2_Init();
    EPD_2IN9_V2_Clear();

    printf("Goto Sleep...\r\n");
    EPD_2IN9_V2_Sleep();
    free(BlackImage);
    BlackImage = NULL;
}