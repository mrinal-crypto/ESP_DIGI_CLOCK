#include <WiFi.h>
#include <ESP32Ping.h>
#include <WiFiManager.h>
#include "time.h"
#include <U8g2lib.h>
#include <string.h>
#include <FastLED.h>

#define BUZ 12
#define DATA_PIN 21
#define ADJUST_BRIGHTNESS 14
#define NUM_LEDS 4
#define CHIPSET WS2812
#define BRIGHTNESS 50
#define COLOR_ORDER GRB
#define WIFI_CONNECT_STATUS_LED 3
#define UPLOAD_DOWNLOAD_STATUS_LED 0
#define INTERNET_STATUS_LED 1
#define TIME_STATUS_LED 2

CRGB leds[NUM_LEDS];

void tostring();
void wifiSignalQuality();
void connectWiFi();
void wifiConnectStatusLed();
void printLocalTime();
void welcomeMsg();
void clearLCD();

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 18, 23, 5, 22); //for full buffer mode

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;
unsigned long previousMillis = 0;
const long buzzerDuration = 200;
uint8_t wifiRSSI = 0;
String ssid = "";

int signalQuality[] = {99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
                       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 97, 97, 96, 96, 95, 95, 94, 93, 93, 92,
                       91, 90, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 76, 75, 74, 73, 71, 70, 69, 67, 66, 64,
                       63, 61, 60, 58, 56, 55, 53, 51, 50, 48, 46, 44, 42, 40, 38, 36, 34, 32, 30, 28, 26, 24, 22, 20,
                       17, 15, 13, 10, 8, 6, 3, 1, 1, 1, 1, 1, 1, 1, 1
                      };

void setup() {
  Serial.begin(115200);
  pinMode(BUZ, OUTPUT);
  u8g2.begin();
  welcomeMsg();
  delay(3000);
  connectWiFi(0, 11);
  configTime(0, 55);
  u8g2.clearBuffer();
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  delay(500);
}

//////////////////////////////////////////////////////////

void tostring(char str[], int num)
{
  int i, rem, len = 0, n;

  n = num;
  while (n != 0)
  {
    len++;
    n /= 10;
  }
  for (i = 0; i < len; i++)
  {
    rem = num % 10;
    num = num / 10;
    str[len - (i + 1)] = rem + '0';
  }
  str[len] = '\0';
}

/////////////////////////////////////////////////////

void connectWiFi(uint8_t cwx, uint8_t cwy) {
  clearLCD(0, 0, 128, 64);
  WiFiManager wm;
  u8g2.setFont(u8g2_font_t0_11_tr);
  u8g2.drawStr(cwx, cwy, "WiFi Conection - ");
  u8g2.sendBuffer();
  WiFi.disconnect();
  delay(50);
  bool success = false;
  while (!success) {
    wm.setConfigPortalTimeout(60);
    success = wm.autoConnect("ESP.digiClock");
    if (!success) {

      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_t0_11_tr);
      u8g2.drawStr(cwx, cwy, "WiFi Conection - ");

      clearLCD(cwx + 97, cwy - 11, 31, 11);
      u8g2.drawStr(cwx + 97, cwy, " err!");
      u8g2.drawStr(cwx, cwy + 11, "AP - ESP.digiClock");
      u8g2.drawStr(cwx, cwy + 22, "IP - 192.168.4.1");
      u8g2.sendBuffer();
    }
  }
  u8g2.setFont(u8g2_font_t0_11_tr);
  clearLCD(cwx + 97, cwy - 11, 31, 11);
  u8g2.drawStr(cwx + 97, cwy, " ok!");
  u8g2.sendBuffer();

  delay(2000);

  ssid = WiFi.SSID();

  if (strlen(ssid.c_str()) > 7) {
    String shortSSID = ssid.substring(0, 7);
    String wifiName = "SSID - " + shortSSID + " ...";
    clearLCD(cwx, cwy, 128, 11);
    u8g2.setFont(u8g2_font_t0_11_tr);
    u8g2.drawStr(cwx, cwy + 11, wifiName.c_str());
    u8g2.sendBuffer();
  } else {
    String wifiName = "SSID - " + ssid;
    clearLCD(cwx, cwy, 128, 11);

    u8g2.setFont(u8g2_font_t0_11_tr);
    u8g2.drawStr(cwx, cwy + 11, wifiName.c_str());
    u8g2.sendBuffer();
  }

  delay(2000);


  String rawIP = WiFi.localIP().toString(); //toString () used for convert char to string
  String IPAdd = "IP - " + rawIP;
  clearLCD(cwx, cwy + 11, 128, 11);
  u8g2.setFont(u8g2_font_t0_11_tr);
  u8g2.drawStr(cwx, cwy + 22, IPAdd.c_str()); //c_str() function used for convert string to const char *
  u8g2.sendBuffer();

  delay(2000);

  wifiRSSI = WiFi.RSSI() * (-1);
  char str[10];
  tostring(str, signalQuality[wifiRSSI]);
  String wifiSignal = "SIGNAL - " + String(str) + " %";
  clearLCD(cwx, cwy + 22, 128, 11);
  u8g2.setFont(u8g2_font_t0_11_tr);
  u8g2.drawStr(cwx, cwy + 33, wifiSignal.c_str());
  u8g2.sendBuffer();

  delay(2000);

}
///////////////////////////////////////////////////////////////////
void configTime(uint8_t ctx, uint8_t cty) {
  clearLCD(ctx, cty + 11, 128, 11);
  u8g2.setFont(u8g2_font_t0_11_tr);
  u8g2.drawStr(ctx, cty, "TIME SERVER - ");
  u8g2.sendBuffer();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  if (!getLocalTime(&timeinfo))
  {
    clearLCD(ctx + 85, cty - 11, 43, 11);
    u8g2.setFont(u8g2_font_t0_11_tr);
    u8g2.drawStr(ctx + 85, cty, "error!");
    u8g2.sendBuffer();
  }
  if (getLocalTime(&timeinfo)) {
    clearLCD(ctx + 85, cty - 11, 43, 11);
    u8g2.setFont(u8g2_font_t0_11_tr);
    u8g2.drawStr(ctx + 85, cty, "ok!");
    u8g2.sendBuffer();
  }
  delay(2000);
}
////////////////////////////////////////////////////////////////////

void wifiConnectStatusLed(uint8_t wifiConnectStatus) {
  if (wifiConnectStatus == 1) {
    leds[WIFI_CONNECT_STATUS_LED] = CRGB(255, 64, 0);
    FastLED.show();
  }
}

///////////////////////////////////////////////////////////////////

void printLocalTime(uint8_t ltx, uint8_t lty) {

  struct tm timeinfo;
  getLocalTime(&timeinfo);

  if (!getLocalTime(&timeinfo))
  {
    leds[TIME_STATUS_LED] = CRGB(0, 0, 0);
    FastLED.show();

    clearLCD(ltx, lty - 18, 128, 18);
    u8g2.setFont(u8g2_font_unifont_tr);
    u8g2.drawStr(ltx, lty, "Time Failed!");
    u8g2.sendBuffer();
  }

  if (getLocalTime(&timeinfo)) {

    leds[TIME_STATUS_LED] = CRGB(255, 0, 255);

    char timeStringBuff[10];
    char dateStringBuff[25];
    char secStringBuff[10];
    char ampmStringBuff[10];
    char wDayStringBuff[10];
    char mDayStringBuff[10];
    char mNameStringBuff[10];
    char monthStringBuff[10];
    char yearStringBuff[10];
    char dayOfYearStringBuff[10];
    char weakOfYearStringBuff[10];

    strftime(timeStringBuff, sizeof(timeStringBuff), "%I:%M", &timeinfo);
    strftime(secStringBuff, sizeof(secStringBuff), "%S", &timeinfo);
    strftime(ampmStringBuff, sizeof(ampmStringBuff), "%p", &timeinfo);
    strftime(wDayStringBuff, sizeof(wDayStringBuff), "%A", &timeinfo);
    strftime(yearStringBuff, sizeof(yearStringBuff), "%y", &timeinfo);
    strftime(dateStringBuff, sizeof(dateStringBuff), "%d.%m.%y %b", &timeinfo);
    strftime(dayOfYearStringBuff, sizeof(dayOfYearStringBuff), "%j d", &timeinfo);
    strftime(weakOfYearStringBuff, sizeof(weakOfYearStringBuff), "%W w", &timeinfo);

    /*
      %a Abbreviated weekday name.
      %A  Full weekday name.
      %b  Abbreviated month name.
      %B  Full month name.
      %c  Date/Time in the format of the locale.
      %C  Century number [00-99], the year divided by 100 and truncated to an integer.
      %d  Day of the month [01-31].
      %D  Date Format, same as %m/%d/%y.
      %e  Same as %d, except single digit is preceded by a space [1-31].
      %g  2 digit year portion of ISO week date [00,99].
      %F  ISO Date Format, same as %Y-%m-%d.
      %G  4 digit year portion of ISO week date. Can be negative.
      %h  Same as %b.
      %H  Hour in 24-hour format [00-23].
      %I  Hour in 12-hour format [01-12].
      %j  Day of the year [001-366].
      %m  Month [01-12].
      %M  Minute [00-59].
      %n  Newline character.
      %p  AM or PM string.
      %r  Time in AM/PM format of the locale. If not available in the locale time format, defaults to the POSIX time AM/PM format: %I:%M:%S %p.
      %R  24-hour time format without seconds, same as %H:%M.
      %S  Second [00-61]. The range for seconds allows for a leap second and a double leap second.
      %t  Tab character.
      %T  24-hour time format with seconds, same as %H:%M:%S.
      %u  Weekday [1,7]. Monday is 1 and Sunday is 7.
      %U  Week number of the year [00-53]. Sunday is the first day of the week.
      %V  ISO week number of the year [01-53]. Monday is the first day of the week. If the week containing January 1st has four or more days in the new year then it is considered week 1. Otherwise, it is the last week of the previous year, and the next year is week 1 of the new year.
      %w  Weekday [0,6], Sunday is 0.
      %W  Week number of the year [00-53]. Monday is the first day of the week.
      %x  Date in the format of the locale.
      %X  Time in the format of the locale.
      %y  2 digit year [00,99].
      %Y  4-digit year. Can be negative.
      %z  UTC offset. Output is a string with format +HHMM or -HHMM, where + indicates east of GMT, - indicates west of GMT, HH indicates the number of hours from GMT, and MM indicates the number of minutes from GMT.
      %Z  Time zone name.
      %%  % character.
    */

    /* display time */
    clearLCD(ltx, lty - 26, 79, 26);
    u8g2.setFont(u8g2_font_timB24_tn);
    u8g2.drawStr(ltx, lty, timeStringBuff);
    u8g2.sendBuffer();

    /* display second */
    clearLCD(ltx + 80, lty - 26, 18, 15);
    u8g2.setFont(u8g2_font_helvR12_tn);
    u8g2.drawStr(ltx + 80, lty - 11, secStringBuff);
    u8g2.sendBuffer();

    /* display am pm */
    clearLCD(ltx + 113, lty - 26, 15, 11);
    u8g2.setFont(u8g2_font_t0_11_tr);
    u8g2.drawStr(ltx + 113, lty - 15, ampmStringBuff);
    u8g2.sendBuffer();

    /* display date */
    clearLCD(ltx, lty, 128, 11);
    u8g2.setFont(u8g2_font_t0_11_tr);
    u8g2.drawStr(ltx, lty + 11, "Date :");
    u8g2.setFont(u8g2_font_t0_11b_tr);
    u8g2.drawStr(ltx + 45, lty + 11, dateStringBuff);
    u8g2.sendBuffer();

    /* display week day */
    clearLCD(ltx, lty + 11, 128, 13);
    u8g2.setFont(u8g2_font_t0_11_tr);
    u8g2.drawStr(ltx, lty + 22, "Week :");
    u8g2.drawStr(ltx + 45, lty + 22, wDayStringBuff);
    u8g2.sendBuffer();

    /* display year days and week no */
    clearLCD(ltx, lty + 24, 128, 11);
    u8g2.setFont(u8g2_font_t0_11_tr);
    u8g2.drawStr(ltx, lty + 35, "Total:");
    u8g2.drawStr(ltx + 45, lty + 35, dayOfYearStringBuff);
    u8g2.drawStr(ltx + 90, lty + 35, weakOfYearStringBuff);
    u8g2.sendBuffer();
  }
}

////////////////////////////////////////////////////////////

void welcomeMsg() {

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB18_tr);
  u8g2.drawStr(0, 22, "ESP32");
  u8g2.setFont(u8g2_font_ncenR12_tr);
  u8g2.drawStr(0, 40, "DIGI - CLOCK");
  u8g2.sendBuffer();
  u8g2.setFont(u8g2_font_t0_11_tr);
  u8g2.drawStr(2, 60, "developed by M.Maity");
  u8g2.sendBuffer();
  u8g2.clearBuffer();

}

//////////////////////////////////////////////

void clearLCD(const long x, uint8_t y, uint8_t wid, uint8_t hig) {
  /*  box wid is right x, box height is below y
      where font wid is right x, font height is upper y
  */
  u8g2.setDrawColor(0);
  u8g2.drawBox(x, y, wid, hig);
  u8g2.setDrawColor(1);
}


//////////////////////////////////////////////////////////////////
void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnectStatusLed(1);
    printLocalTime(0, 26);
  }
  else {
    wifiConnectStatusLed(2);
    connectWiFi(0, 11);
    configTime(0, 55);
    clearLCD(0, 0, 128, 64);
  }
}
