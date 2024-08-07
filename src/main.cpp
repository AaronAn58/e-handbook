#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <EPD.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <SD_MMC.h>

#define TEST_FILE_SIZE (4 * 1024 * 1024)
#define SCK 18
#define MISO 15
#define MOSI 13
#define CS 14

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

void setup()
{

  Serial.begin(115200);
  spi.begin(SCK, MISO, MOSI, CS);
  if (!SD.begin(CS, spi))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  Serial.print("SD Type: ");
  switch (cardType)
  {
  case CARD_NONE:
    Serial.println("No SD card attached");
    break;
  case CARD_MMC:
    Serial.println("MMC/SDSC");
    break;
  case CARD_SD:
    Serial.println("SD/SDHC/SDXC");
    break;
  case CARD_SDHC:
    Serial.println("SDHC/SDXC");
  default:
    Serial.println("Unknown");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  // testIO(SD);
  // createDir(SD, "/test123456");
  listDir(SD, "/", 0);
}

// void setup()
// {
//   Serial.begin(115200);
//   struct tm timeinfo;
//   int currentHour = timeinfo.tm_hour;
//   int currentMin = timeinfo.tm_min;
//   int currentSec = timeinfo.tm_sec;

//   Serial.print("Current Time: " + (String)currentHour + ":" + (String)currentMin + ":" + (String)currentSec);
//   startWiFi();
//   WiFiClient wifi;
//   testHttp(wifi, "GET");
// }

void loop()
{
  // put your main code here, to run repeatedly:
}
