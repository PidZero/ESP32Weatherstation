// (c) Johannes Neidhart, 2020
// Arduino IDE / ESP32 version
// Needs a global (WiFi)-connection


#include "time.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include <vector>


#ifndef WEATHERSTATION_H
#define WEATHERSTATION_H

class weatherStation
{
  public:
    weatherStation(String city, String owmId);
    ~weatherStation();
    void init();
    void pull_weatherdata();
    void pull_forecastdata();

    void updateTime();
    String year;
    String month;
    String day;
    String time;

    String temperature;
    String description;

    int min_int{ -1};
    int hour_int{ -1};

    std::vector <String> fc_desc;
    std::vector <String> fc_temp;
    std::vector <String> fc_times;

    //https://arduinojson.org/v6/assistant/

  private:
    String weatherUrl;
    String forecastUrl;
    // struct for the clock
    struct tm timeInfo;
    // setup of the clock
    const char* ntpServer = "pool.ntp.org";
    const long  gmtOffset_sec = 3600;
    const int   daylightOffset_sec = 3600;
};


//Constructor
inline
weatherStation::weatherStation(String city, String owmId) {
  // Resize vectors
  fc_desc.resize(3);
  fc_temp.resize(3);
  fc_times.resize(3);
  weatherUrl = "https://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + owmId + "&units=metric";
  forecastUrl = "https://api.openweathermap.org/data/2.5/forecast?q=" + city + "&cnt=5&appid=" + owmId + "&units=metric";
}


//Destructor
inline
weatherStation::~weatherStation() {
}


// Initialization of the clock needs an internet connection
// which might be setup after an instance of this class is defined.
// This means, that init procedure does not always work inside the constructor
inline
void weatherStation::init() {
  //Initialize the clock and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  while (!getLocalTime(&timeInfo)) {
    delay(1500);
  }
}


// pull_weatherdata() will retrieve the latest weather data and save it to the
// class members temperature and description
inline
void weatherStation::pull_weatherdata() {
  HTTPClient http;
  http.useHTTP10(true);
  http.begin(weatherUrl);
  // Connect an save the return code
  int httpCode = http.GET();
  // If successful store the data and work the JSON
  if (httpCode > 0) {
    // For the JSON this code uses parts of the example from
    //https://arduinojson.org/v6/assistant/
    int ibuf;
    // Allocate memory
    DynamicJsonDocument doc(3000);
    // Streaming the data has the advantag, that the data is not saved and then copied
    // but written directly to the JsonDocument.
    DeserializationError err = deserializeJson(doc, http.getStream());

    // Passing parts of the Json to new variables simplifies the extraction

    JsonObject weather_0 = doc["weather"][0];
    int weather_0_id = weather_0["id"]; // 804
    const char* weather_0_main = weather_0["description"]; // "Clouds"

    JsonObject main = doc["main"];
    float main_temp = main["temp"]; // 29.91
    main_temp = float(int(10 * main_temp)) / 10.; // 29.91
    temperature = String(main_temp).substring(0);
    ibuf = temperature.length();

    temperature.remove(ibuf - 1);
    description = String(weather_0_main);

    // close connection
    http.end();
  }

}


//Pull the forecastdata and write it to forecastdata
inline
void weatherStation::pull_forecastdata() {
  HTTPClient http;
  http.useHTTP10(true);
  http.begin(forecastUrl);
  // Connect an save the return code
  int httpCode = http.GET();
  // If successful store the data and work the JSON

  if (httpCode > 0) {
    // For the JSON this code uses parts of the example from
    //https://arduinojson.org/v6/assistant/
    const char* chbuf;
    const char* descbuf;
    float flbuf;
    int ibuf;
    String list_dt_txt;
    // Allocate memory
    DynamicJsonDocument doc(3000);
    // Streaming the data has the advantag, that the data is not saved and then copied
    // but written directly to the JsonDocument.
    DeserializationError err = deserializeJson(doc, http.getStream());

    // Passing parts of the Json to new variables simplifies the extraction
    // The forecast stores 3h forecast data. Extracting for three times,
    // the first starting in 3h means to first grab the element 1 of dlist.
    JsonArray dlist = doc["list"];
    JsonObject list = dlist[1];
    JsonObject list_main = list["main"];
    JsonObject list_weather_0 = list["weather"][0];
    chbuf = list["dt_txt"]; // "2020-08-15 12:00:00"
    list_dt_txt = String(chbuf); // "2020-08-15 12:00:00"

    chbuf = list["dt_txt"]; // "2020-08-15 12:00:00"
    descbuf = list_weather_0["main"]; // "Rain"
    fc_times.at(0) = list_dt_txt.substring(11, 16);
    flbuf = list_main["temp"]; // 25.7
    fc_temp.at(0) = String(flbuf);
    ibuf = fc_temp.at(0).length();
    fc_temp.at(0).remove(ibuf - 1);
    fc_desc.at(0) = String(descbuf);

    list = dlist[2];
    list_main = list["main"];
    list_weather_0 = list["weather"][0];
    descbuf = list_weather_0["main"]; // "Rain"
    chbuf = list["dt_txt"]; // "2020-08-15 12:00:00"
    list_dt_txt = String(chbuf); // "2020-08-15 12:00:00"
    fc_times.at(1) = list_dt_txt.substring(11, 16);
    flbuf = list_main["temp"]; // 25.7
    fc_temp.at(1) = String(flbuf);
    ibuf = fc_temp.at(1).length();
    fc_temp.at(1).remove(ibuf - 1);
    fc_desc.at(1) = String(descbuf);

    list = dlist[3];
    list_main = list["main"];
    list_weather_0 = list["weather"][0];
    descbuf = list_weather_0["main"]; // "Rain"
    chbuf = list["dt_txt"]; // "2020-08-15 12:00:00"
    list_dt_txt = String(chbuf); // "2020-08-15 12:00:00"
    fc_times.at(2) = list_dt_txt.substring(11, 16);
    flbuf = list_main["temp"]; // 25.7
    fc_temp.at(2) = String(flbuf);
    ibuf = fc_temp.at(2).length();
    fc_temp.at(2).remove(ibuf - 1);
    fc_desc.at(2) = String(descbuf);
  }
  // close connection
  http.end();
}


inline
void  weatherStation::updateTime() {
  // getLocalTime() calls time() function, which returns the Unix time,
  // which is transformed to local Time in a following step.
  getLocalTime(&timeInfo);
  min_int = timeInfo.tm_min;
  hour_int = timeInfo.tm_hour;
}


#endif
