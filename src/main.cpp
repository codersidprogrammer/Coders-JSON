#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       
#define SCREEN_ADDRESS 0x3C 

//DISPLAY
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//JSON Library untuk arduino
//Untuk melakukan serialize dan deserialize
DynamicJsonDocument doc(1536);

//SSID
const char *ssid = "Nyari tuhan jangan wifi";
const char *password = "imissyou";

//Server API JSON untuk weather
const char *serverName = "https://ibnux.github.io/BMKG-importer/cuaca/5002333.json";
String weatherReading;


//Milis
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;


//Untuk menampilkan dan memposisikan  tulisan di OLED
void oledDisplay(int size, int x, int y, float value, String unit)
{
  int charLen = 12;
  int xo = x + charLen * 3.2;
  int xunit = x + charLen * 3.6;
  int xval = x;
  display.setTextSize(size);
  display.setTextColor(WHITE);

  if (unit == "%")
  {
    display.setCursor(x, y);
    display.print(value, 0);
    display.print(unit);
  }
  else
  {
    if (value > 99)
    {
      xval = x;
    }
    else
    {
      xval = x + charLen;
    }
    display.setCursor(xval, y);
    display.print(value, 0);
    display.drawCircle(xo, y + 2, 2, WHITE); // print degree symbols (  )
    display.setCursor(xunit, y);
    display.print(unit);
  }
}


//Untuk melakukan request ke API JSON
String httpGETRequest(const char *serverName)
{
  WiFiClientSecure httpsClient;
  HTTPClient http;

  httpsClient.setInsecure();
  httpsClient.connect(serverName, 443);

  http.begin(httpsClient, serverName);

  String payload;
  int response = http.GET();
  if (response == HTTP_CODE_OK)
  {
    payload = http.getString();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(response);
  }

  http.end();
  return payload;
}


//Setup
void setup()
{
  Serial.begin(9600);

  //Memulai konfigurasi WIFI
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());


  // SSD1306_SWITCHCAPVCC = Aktivasi tegangan 3.3V internal display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; 
  }

  // Clear the buffer
  display.clearDisplay();
  delay(2000);
}

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    display.clearDisplay();
    //Check WiFi status Wifi
    if (WiFi.status() == WL_CONNECTED)
    {

      //Menampilkan Text "Fetching..." ke OLED
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("Fetching..");
      display.display();

      //Melakukan Request ke API JSON
      weatherReading = httpGETRequest(serverName);
      Serial.println("---- REQUEST RESULT FROM API ----");
      Serial.println(weatherReading);
      
      //Proses deserialize
      deserializeJson(doc, weatherReading);

      display.clearDisplay();

      //Iterasi untuk menunjukan semua elemen yang ada di JSON
      for (JsonObject elem : doc.as<JsonArray>())
      {
        const char *cuaca = elem["cuaca"];

        display.setTextSize(1.5);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println(cuaca);
        oledDisplay(3, 5, 28, elem["humidity"], "%");
        oledDisplay(2, 70, 16, elem["tempC"], "C");
        oledDisplay(2, 70, 44, elem["tempF"], "F");
        display.display();

        delay(1500);
        display.clearDisplay();
      }
    }
    else
    {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
