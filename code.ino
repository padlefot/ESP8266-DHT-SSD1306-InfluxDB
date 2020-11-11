/*********
  padlefot. November 2020.
  Thanks to https://randomnerdtutorials.com/esp32-esp8266-dht-temperature-and-humidity-oled-display/ for sample code and circuit drawing.
*********/
//Include some required libs
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <InfluxDb.h>                          // https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino

//Define some variables
#define SCREEN_WIDTH 128                       // OLED display width, in pixels
#define SCREEN_HEIGHT 64                       // OLED display height, in pixels
#define WIFI_SSID "Some"                       // Enter SSID of your WIFI Access Point
#define WIFI_PASS "WiFi"                       // Enter Password of your WIFI Access Point
                                               // Uncomment the type of sensor in use:
#define DHTTYPE      DHT11                     // DHT 11
//#define DHTTYPE    DHT22                     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21                     // DHT 21 (AM2301)
#define DHTPIN 2                               // Digital pin connected to the DHT sensor
                                               // On the SSD1306 display - Pin settings for esp8266/NodeMCU: D2 is SDA, D1 is SCK.
//define InfluxDB stuff
#define ESP_DEVICENAME "dht-11-test123"         // Name to display for device in Influx Database
#define INFLUXDB_HOST "192.168.2.8"             // Enter IP of device running Influx Database
#define INFLUXDB_PORT "8086"                    // Enter the port # of InfluxDB (This does nothing atm)
#define INFLUXDB_DATABASE "sensors"             // Enter the Influx Database 
#define INFLUXDB_USER "test"                    // Enter the Influx Database Username
#define INFLUXDB_PASS "test"                    // Enter the Influx Database Password

  // Declaration for a SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Declaration for DHT sensors
DHT dht(DHTPIN, DHTTYPE);

  //Start setup
void setup() {
  Serial.begin(115200);
  
  //Start dht sensor, WiFi connection and SD1306 Display
  dht.begin();
  ESP8266WiFiMulti WiFiMulti;

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi AP: ");
  Serial.print(WIFI_SSID);
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Setup/Boot Complete.");
}

void loop() {
  delay(5000);

  //read temperature and humidity
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  // clear display
  display.clearDisplay();

  // display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Humidity: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(h);
  display.print(" %");

  display.display();
  Serial.println("--OLED Display Refreshed");


  //declare influxDB
  Influxdb influx(INFLUXDB_HOST);
  influx.setDbAuth (INFLUXDB_DATABASE, INFLUXDB_USER, INFLUXDB_PASS);
  // influx.setDb (INFLUXDB_DATABASE); // This can be used instead of the line above if auth is disabled on the database


  // Create a measurement object for temperature
  InfluxData measurement ("temperature");
  //measurement.addTag("device", d2);
  measurement.addTag("sensor", ESP_DEVICENAME);
  measurement.addValue("value", t);

  // Write temperature measurement into db
  influx.write(measurement);

  // Create a measurement object for humidity
  InfluxData measurement2 ("humidity");
  //measurement2.addTag("device", d2);
  measurement2.addTag("sensor", ESP_DEVICENAME);
  measurement2.addValue("value", h);

  // Write humidity measurement into db
  influx.write(measurement2);

  // Print influx details to serial for visibility
    Serial.print("--Writing to InfluxDB = ");
    Serial.print(INFLUXDB_DATABASE);
    Serial.print(" @ ");
    Serial.println(INFLUXDB_HOST);
    Serial.println ("..");
  // Print measurements to serial for visibility

  Serial.print("temperature = ");
  Serial.print(t);
  Serial.println("C  ");
  Serial.print("humidity = ");
  Serial.print(h);
  Serial.println("%   ");
  Serial.println("..");
  delay(2000);
}
