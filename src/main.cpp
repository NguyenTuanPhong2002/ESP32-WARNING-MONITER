#include <Arduino.h>

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Servo myservo;
int pos = 0;
String id;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Replace with your network credentials
const char *ssid = "NNE";
const char *password = "01112002";

#define DHTPIN 27 // Digital pin connected to the DHT sensor

const int MQ2 = 34; // set analog mq2 pin in

const int Sound = 35; // set analog sound pin in

const int Tremors = 25; // set analog tremors pin in

const int Horn = 13; // set pin out horn

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
// #define DHTTYPE    DHT22     // DHT 22 (AM2302)
// #define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDHTTemperature()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  // float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  //set the value of the warning temperture
  if (t > 30)
  {
    digitalWrite(Horn, 0);
    delay(1000);
  }
  else
  {
    digitalWrite(Horn, 1);
    delay(1000);
  }
  //on off the horn
  if (isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else
  {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h))
  {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else
  {
    Serial.println(h);
    return String(h);
  }
}

String readMQ2()
{
  int Value = 0;
  Value = analogRead(MQ2);
  float percen = 0;
  percen = Value / 40.95;
  Serial.print("MQ2 value = ");
  Serial.println(Value);
  if (Value > 3000)
  {
    myservo.write(90);
  }
  else
  {
    myservo.write(0);
  }
  return String(percen);
}

String readSound()
{
  int Value = 0;
  Value = analogRead(Sound);
  float percen = 0;
  percen = Value / 40.95;
  Serial.print("Sound value = ");
  Serial.println(Value);
  return String(percen);
}

String readTremors()
{
  int Value = 0;
  Value = analogRead(Tremors);
  float percen = 0;
  percen = Value / 40.95;
  //confige the value of the warning
  if (percen > 70)
  {
    digitalWrite(Horn, 0);
    delay(1000);
  }
  else
  {
    digitalWrite(Horn, 1);
    delay(1000);
  }
  Serial.print("Tremors value = ");
  Serial.println(Value);
  return String(percen);
}

String Warning()
{
  int mq2_value = 0;
  int sound_value = 0;
  int tremors_value = 0;

  mq2_value = analogRead(MQ2);
  sound_value = analogRead(Sound);
  tremors_value = analogRead(Tremors);

  float percen = 0;
  percen = (mq2_value) / 40.95;
  Serial.print("Warning value = ");
  Serial.println(percen);
  if (percen > 60)
  {
    digitalWrite(Horn, 0);
    delay(1000);
  }
  else
  {
    digitalWrite(Horn, 1);
    delay(1000);
  }
  return String(percen);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css" />

  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>THE WARNING MONITOR 2022</h2>

  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Nhiet do </span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>

  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Do am </span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>

  <p>
    <i class="fas fa-window-restore"></i>
    <span class="dht-labels">Muc do khi gas </span>
    <span id="mq2">%MQ2%</span>
    <sup class="units">&percnt;</sup>
  </p>


  <p>
    <i class="fas fa-exclamation-circle"></i>
    <span class="dht-labels">Muc do rung chan </span>
    <span id="tremors">%TREMORS%</span>
    <sup class="units">&percnt;</sup>
  </p>

  <p>
    <i class="fas fa-exclamation-triangle"></i>
    <span class="dht-labels">Muc do canh bao </span>
    <span id="warning">%WARNING%</span>
    <sup class="units">&percnt;</sup>
  </p>

</body>
<script>

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("mq2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/mq2", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("sound").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/sound", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tremors").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/tremors", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("warning").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/warning", true);
  xhttp.send();
}, 1000 ) ;


</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String &var)
{
  // Serial.println(var);
  if (var == "TEMPERATURE")
  {
    return readDHTTemperature();
  }
  else if (var == "HUMIDITY")
  {
    return readDHTHumidity();
  }
  else if (var == "MQ2")
  {
    return readMQ2();
  }
  else if (var == "SOUND")
  {
    return readSound();
  }
  else if (var == "TREMORS")
  {
    return readTremors();
  }
  else if (var == "WARNING")
  {
    return Warning();
  }
  return String();
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(9600);

  myservo.attach(26); // SET PIN SERVO

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);

  pinMode(Horn, OUTPUT);

  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  id = String(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readDHTTemperature().c_str()); });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readDHTHumidity().c_str()); });
  server.on("/mq2", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readMQ2().c_str()); });
  server.on("/sound", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readSound().c_str()); });
  server.on("/tremors", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readSound().c_str()); });
  server.on("/warning", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readSound().c_str()); });
  // Start server
  server.begin();
}

void loop()
{
  for (int i = 0; i < 5; i++)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Nhiet do");
    display.setCursor(80, 0);
    display.print(readDHTTemperature());
    display.setCursor(0, 20);
    display.print("Do am");
    display.setCursor(80, 20);
    display.print(readDHTHumidity());
    display.setCursor(0, 40);
    display.print("Khi gas = ");
    display.setCursor(80, 40);
    display.print(readMQ2());
    display.display();
    delay(1000);
  }
  for (int i = 0; i < 5; i++)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Muc rung chan = ");
    display.setCursor(100, 20);
    display.print(readTremors());
    display.setCursor(0, 40);
    display.print("Canh bao = ");
    display.setCursor(80, 40);
    display.print(Warning());
    display.display();
    delay(1000);
  }
}