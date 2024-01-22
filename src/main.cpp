/*
  An example showing rainbow colours on a 1.8" TFT LCD screen
  and to show a basic example of font use.

  Make sure all the display driver and pin connections are correct by
  editing the User_Setup.h file in the TFT_eSPI library folder.

  Note that yield() or delay(0) must be called in long duration for/while
  loops to stop the ESP8266 watchdog triggering.

  #########################################################################
  ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
  #########################################################################
*/

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "pin_config.h"
#include "TFT_eSPI.h"
#include <WiFi.h>
#include "time.h"
#include "Gregpole.h".


#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
#error  "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"
#endif
#define DFPlayerRxd U1RXD_IN_IDX
#define DFPlayerTxd U1TXD_OUT_IDX

// Display.

#define     DISPLAY_HEIGHT          170                         // T-Display-S3 display height in pixels.
#define     DISPLAY_WIDTH           320                         // T-Display-S3 display width in pixels.
#define     DISPLAY_BRIGHTNESS_MAX  252                         // T-Display-S3 display brightness maximum.
#define     DISPLAY_BRIGHTNESS_MIN  0                           // T-Display-S3 display brightness minimum.

// Sprites.

#define     SPRITE_BATTERY_FONT     2                           // Battery sprite font size.
#define     SPRITE_BATTERY_HEIGHT   30                          // Battery sprite height in pixels.
#define     SPRITE_BATTERY_WIDTH    100                         // Battery sprite width in pixels.
#define     SPRITE_DATE_FONT        4                           // Date sprite font size.
#define     SPRITE_DATE_HEIGHT      40                          // Date sprite height in pixels.
#define     SPRITE_DATE_WIDTH       200                         // Date sprite width in pixels.
#define     SPRITE_TIME_FONT        6                           // Time sprite font size.
#define     SPRITE_TIME_HEIGHT      40                          // Time sprite height in pixels.
#define     SPRITE_TIME_WIDTH       200                         // Time sprite width in pixels.


// Battery.

uint32_t    uVolt;                                              // Battery voltage.

// Display (T-Display-S3 lcd display).

TFT_eSPI    lcd = TFT_eSPI();                                   // T-Display-S3 lcd.
int         lcdBacklighBrightness = DISPLAY_BRIGHTNESS_MAX;     // T-Display-S3 brightness.

// Sprites.

TFT_eSprite spriteBackground = TFT_eSprite(& lcd);              // Background sprite.
TFT_eSprite spriteBattery = TFT_eSprite(& lcd);                 // Battery sprite.
TFT_eSprite spriteDate = TFT_eSprite(& lcd);                    // Date sprite.
TFT_eSprite spriteTime = TFT_eSprite(& lcd);                    // Time sprite.

// Time.

char        chDayOfMonth[3];                                    // Day of month (0 through 31).
const int   nDaylightOffsetSeconds = 3600;                      // Daylight savings time offset.
char        chDayofWeek[4];                                     // Day of week (Sunday through Saturday).
const long  lGmtOffsetSeconds = -6 * 3600;                      // Time zone offset.
char        chHour[3];                                          // Hour.
char        chMinute[3];                                        // Minute.
char        chMonth[4];                                         // Month.
const char* chNtpServer = "pool.ntp.org";                       // NTP time server.
char        chSecond[3];                                        // Second.
char        chYear[5];                                          // Year.

// Wifi.

String      stringIP;                                           // IP address.
const char* chPassword = "coolhippo758";                  // Your router password.
const char* chSsid     = "NETGEAR20";                      // Your router SSID.


TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

unsigned long targetTime = 0;
byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;
uint32_t runing = 0;

#define LED_YELLOW 43
#define LED_GREEN 44


void setup(void)
{
  Serial.begin(115200);

    // Analog.

  analogReadResolution(12);

  // Battery.

    // Sprite.
    
    spriteBattery.createSprite(SPRITE_BATTERY_WIDTH, SPRITE_BATTERY_HEIGHT);
    spriteBattery.setSwapBytes(true);
    spriteBattery.setTextColor(TFT_BLACK, TFT_WHITE);

    // Enable the modeul to operate from an external LiPo battery.
    
    pinMode(15, OUTPUT);
    digitalWrite(15, 1);

  // Background sprite.
  
  spriteBackground.createSprite(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  spriteBackground.setSwapBytes(true);
  spriteBackground.setTextColor(TFT_WHITE, TFT_BLACK);

  // Date sprite.

  spriteDate.createSprite(SPRITE_DATE_WIDTH, SPRITE_DATE_HEIGHT);
  spriteDate.setSwapBytes(true);
  spriteDate.setTextColor(TFT_WHITE, TFT_BLACK);

  // Time sprite.

  spriteTime.createSprite(SPRITE_TIME_WIDTH, SPRITE_TIME_HEIGHT);
  spriteTime.setSwapBytes(true);
  spriteTime.setTextColor(TFT_WHITE, TFT_BLACK);

  // Wifi.

    // Start wifi.

    lcd.fillScreen(TFT_BLACK);
    lcd.drawString("Awaiting wifi connection...", 0, 0, 2);
    WiFi.begin(chSsid, chPassword);

    // Wait for a connection.
    
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }

    // Connected.
    
    Serial.println("\nWiFi connected.");
    stringIP = WiFi.localIP().toString();
    lcd.fillScreen(TFT_BLACK);
    lcd.drawString("Wifi connected to " + stringIP + ".", 0, 0, 2);
    delay(2000);

      // Time.

  configTime(lGmtOffsetSeconds, nDaylightOffsetSeconds, chNtpServer);




  Serial2.begin(9600, SERIAL_8N1, DFPlayerRxd, DFPlayerTxd);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);

  //digitalWrite(LED_YELLOW, HIGH);
  //Serial.begin(115200);
  //digitalWrite(LED_GREEN, HIGH);



    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);


//    tft.fillScreen(TFT_YELLOW); delay(1000);
//    tft.fillScreen(TFT_GREEN); delay(1000);
//    tft.fillScreen(TFT_BLUE); delay(1000);

    targetTime = millis() + 1000;
}

void loop()
{
  // Local variables.

  static  char                chBuffer[81];
  static  struct              tm timeinfo;
 
  // Check for time and date availability.

  if(getLocalTime(& timeinfo))
  {
    // Time and date available, obtain hours, minutes and seconds.
    
    strftime(chHour, sizeof(chHour), "%H", & timeinfo);
    strftime(chMinute, sizeof(chMinute), "%M", & timeinfo);
    strftime(chSecond, sizeof(chSecond), "%S", & timeinfo);
  
    // Then obtain day of week, day of month, month and year.
    
    strftime(chDayofWeek, sizeof(chDayofWeek), "%A", & timeinfo);
    strftime(chDayOfMonth, sizeof(chDayOfMonth), "%d", & timeinfo);
    strftime(chMonth, sizeof(chMonth), "%B", & timeinfo);
    strftime(chYear, sizeof(chYear), "%Y", & timeinfo);
  }
     
  // Allow the user to adjust backlight brightness.
  
  if((digitalRead(14) == 0) && (lcdBacklighBrightness < DISPLAY_BRIGHTNESS_MAX))
  {
    // Increase.
    
    lcdBacklighBrightness += 4;
    ledcSetup(0, 10000, 8);
    ledcAttachPin(38, 0);
    ledcWrite(0, lcdBacklighBrightness);
  }
  else if((digitalRead(0)) == 0 && (lcdBacklighBrightness > DISPLAY_BRIGHTNESS_MIN))
  {
    // Decrease.
    
    lcdBacklighBrightness -= 4;
    ledcSetup(0, 10000, 8);
    ledcAttachPin(38, 0);
    ledcWrite(0, lcdBacklighBrightness);
  }
  
  // Transfer "Greg_on_a pole_small" to the background sprite in order to "erase" the background sprite.
  
  spriteBackground.pushImage(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, Greg_on_pole_small);

  // Create and draw the time sprite onto the background sprite.

    // Convert time from 24 hour to 12 hour (optional).
    
    int nTemp = atoi(chHour);
    if(nTemp > 12)
      nTemp -= 12;
    sprintf(chHour, "%2d", nTemp);

    // Update the time sprite.

    spriteTime.fillRect(0, 0, SPRITE_TIME_WIDTH, SPRITE_TIME_HEIGHT, TFT_BLACK);
    sprintf(chBuffer, "%s:%s:%s", String(chHour), String(chMinute), String(chSecond));
    spriteTime.drawString(String(chBuffer), (SPRITE_TIME_WIDTH / 2) - (lcd.textWidth(chBuffer, SPRITE_TIME_FONT) / 2), 0, SPRITE_TIME_FONT);
    
    // Transfer the time sprite onto the background sprite with black transparency.
        spriteTime.pushToSprite(& spriteBackground, (DISPLAY_WIDTH / 2) - (SPRITE_TIME_WIDTH / 2), 30, TFT_BLACK);

  // Create and draw the date sprite onto the background sprite.

    // Leading zero supress the day of month.
    
    nTemp = atoi(chDayOfMonth);
    if(nTemp < 10)
      sprintf(chDayOfMonth, "%2d", nTemp);
      
    // Update the date sprite
    
    spriteDate.fillRect(0, 0, SPRITE_DATE_WIDTH, SPRITE_DATE_HEIGHT, TFT_BLACK);
    sprintf(chBuffer, "%s, %s %s, %s", String(chDayofWeek), String(chMonth), String(chDayOfMonth), String(chYear));
    spriteDate.drawString(String(chBuffer), (SPRITE_DATE_WIDTH / 2) - (lcd.textWidth(chBuffer, SPRITE_DATE_FONT) / 2), 0, SPRITE_DATE_FONT);

    // Transfer the date sprite onto the background sprite with black transparency.
    
    spriteDate.pushToSprite(& spriteBackground, (DISPLAY_WIDTH / 2) - (SPRITE_DATE_WIDTH / 2), 140, TFT_BLACK);

  // Update the battery sprite.

    // Read the battery voltage.
    
    uVolt = (analogRead(4) * 2 * 3.3 * 1000) / 4096;
  
    // Upeate the battery sprite.
    
    spriteBattery.fillRect(0, 0, SPRITE_BATTERY_WIDTH, SPRITE_BATTERY_HEIGHT, TFT_WHITE);
    spriteBattery.drawString(String(uVolt / 1000) + "." + String(uVolt % 1000) + " vDC", 0, 0, SPRITE_BATTERY_FONT);
    
    // Transfer the battery sprite onto the background sprite with white transparency.
    
    spriteBattery.pushToSprite(& spriteBackground, 0, 0, TFT_WHITE);

  // Update the backlight brightness bar if either button is pressed.

  if((digitalRead(14) == 0) || (digitalRead(0) == 0))
  {
    // Either button is pressed, display the backlight brightness bar.
    
    spriteBackground.fillRect(DISPLAY_WIDTH - 16, 
                              DISPLAY_HEIGHT - (lcdBacklighBrightness * DISPLAY_HEIGHT / DISPLAY_BRIGHTNESS_MAX),
                              16,
                              lcdBacklighBrightness * DISPLAY_HEIGHT / DISPLAY_BRIGHTNESS_MAX,
                              TFT_WHITE);
  }
  
  // With the background sprite now completed, transfer the background sprite to the LCD.
  
  spriteBackground.pushSprite(0, 0);

  // Delay 100 milliseconds.

  delay(100);


/*
    if (millis() > runing) {
        Serial.print("Current running ");
        Serial.print(millis());
        Serial.println(" millis");
        runing = millis() + 1000;
    }
    if (targetTime < millis()) {
        targetTime = millis() + 10000;
          DO SOMETHING HERE EVERY SECOND

        }
*/

 //       tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Do not plot the background colour
//        tft.drawCentreString("12.34", 80, 54, 6); // Draw text centre at position 80, 24 using font 6

  
/*
for (int i=0; i<100; i++){
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  delay (1000);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  delay (1000);

}
*/
}





