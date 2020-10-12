////////////////////////////////////////////////////////////////////////////
// (cc) 2020, Johannes Neidhart
// This peace of software can be used to take an ESP32 in combination 
// with a 160 x 128 display and program it to print the current time and 
// weather as well as the forecast for the next few hours.
// The display should be compatible with the TFT_eSPI library.
// An onlineweathermap account is needed to pull the forecast data.


// Pinning:
// TFT  Name    ESP   TFT_eSPI
// 1    GND     GND
// 2    VCC     3V3
// 3    SCK     G18   TFT_SCLK
// 4    SDA     G23   TFT_MOSI
// 5    RES     G14   TFT_RST
// 6    RS      G2    TFT_DC
// 7    CS      G17   TFT_CS
// 8    LED_A   5V0



#define WAIT 1500

#include <WiFi.h>
#include "weatherStation.h"
// setup of the WiFi connection
const char* ssid       = "<your WLAN>";  // Enter here the name of your WLAN
const char* password   = "WLAN Passwort>";   // Enter here the password of the WLAN
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

////////////////////////////////////////////////////////////////////////////
//    !!! Do not forget to edit User_Setup.h to suit your Hardware !!!    //
////////////////////////////////////////////////////////////////////////////
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

// Setup weatherStation globally
weatherStation ws("<your location>", "<your ID>");

// updateDisplay() redraws the display
void updateDisplay() {
  // Display Size: 160 x 128
  // Set textsize multiplier to 1, black background and green text.
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  // First, convert the hour from int to string
  char hour[12];
  itoa(ws.hour_int, hour, 10);
  // Take care, that there is is no '0' in front of single digit times
  if (ws.hour_int < 10) {
    hour[1] = hour[0];
    hour[0] = '0';
  }

  // Same for the Minutes.
  char minit[12];
  itoa(ws.min_int, minit, 10);//timeinfo.tm_min;
  if (ws.min_int < 10) {
    minit[1] = minit[0];
    minit[0] = '0';
  }

  // Write the time in the middle top of the display with font '7'
  float xstart = 80. - tft.textWidth("00:00", 7) / 2; // Screenwidth/2 - textwidth/2
  tft.drawString(hour, xstart, 2, 7);
  tft.drawString(":", xstart + tft.textWidth("00", 7), 2, 7);
  tft.drawString(minit, xstart + tft.textWidth("00:", 7), 2, 7);

  // some variables are defined to simplify the positioning of text
  float y = tft.fontHeight(7) + 4;
  float xbuf = 7.;
  float startbuf = 15.;
  float degbuf = tft.textWidth("oC", 2) + 2;
  xstart = tft.textWidth("00:00", 2);

  // Print the current weather
  tft.drawString(ws.temperature, xbuf, y, 2);
  tft.drawString("o", xstart + xbuf - 5, y - 3, 2);
  tft.drawString("C", xstart + xbuf + xbuf - 6, y, 2);
  tft.drawString(ws.description, 1 * xstart +  1 * xbuf + degbuf, y, 2);

  // print the weather in 3 hours
  y += tft.fontHeight(2) + 2;
  tft.drawString(ws.fc_times.at(0), xbuf, y, 2);
  tft.drawString(ws.fc_temp.at(0), xstart + 2 * xbuf, y, 2);
  tft.drawString("o", 2 * xstart + xbuf + 3, y - 3, 2);
  tft.drawString("C", 2 * xstart + 2 * xbuf + 2, y, 2);
  tft.drawString(ws.fc_desc.at(0), 2 * xstart + 2 * xbuf + degbuf, y, 2);

  // print the weather in 6 hours
  y += tft.fontHeight(2) + 2;
  tft.drawString(ws.fc_times.at(1), xbuf, y, 2);
  tft.drawString(ws.fc_temp.at(1), xstart + 2 * xbuf, y, 2);
  tft.drawString("o", 2 * xstart + xbuf + 3, y - 3, 2);
  tft.drawString("C", 2 * xstart + 2 * xbuf + 2, y, 2);
  tft.drawString(ws.fc_desc.at(1), 2 * xstart + 2 * xbuf + degbuf, y, 2);

  // print the weather in 9 hours
  y += tft.fontHeight(2) + 2;
  tft.drawString(ws.fc_times.at(2), xbuf, y, 2);
  tft.drawString(ws.fc_temp.at(2), xstart + 2 * xbuf, y, 2);
  tft.drawString("o", 2 * xstart + xbuf + 3, y - 3, 2);
  tft.drawString("C", 2 * xstart + 2 * xbuf + 2, y, 2);
  tft.drawString(ws.fc_desc.at(2), 2 * xstart + 2 * xbuf + degbuf, y, 2);
}


void setup() {
   // Initialize the Display with orientation and text size
  tft.init();
  tft.setRotation(1);
  tft.setTextSize(1);

  // Fill the screen black
  tft.fillScreen(TFT_BLACK);

  // Text shall be green
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  delay(WAIT);
  
  // Initialize the WiFi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      tft.drawString("not connected", 0, 0, 2);
    delay(WAIT);
  }
tft.fillScreen(TFT_BLACK);
tft.drawString("connected", 0, 0, 2);
  delay(WAIT);
 
  // Pull the weather- and forecastdata and print them to the screen.
  ws.init();
  ws.pull_weatherdata();
  ws.pull_forecastdata();
  ws.updateTime();
  updateDisplay();
}

void loop() {
  // timebuf is used to store the minute value
  int timebuf = ws.min_int;
  // get new time
  ws.updateTime();
  // New minute? Pull New weather data
  if (timebuf != ws.min_int) {
    ws.pull_weatherdata();
    // New hour? Pull new forecast data
    if ((ws.min_int == 0)) {
      ws.pull_forecastdata();
    }
    // update display accordingly
    updateDisplay();
  }

  delay(WAIT);

}
