#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 2      // what pin we're connected to
#define DHTTYPE DHT22 // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino
WiFiClient wifi;
WiFiServer server(80);
PubSubClient mqtt(wifi);

const char *ssid = "mobile@lsong.org";    // Your ssid
const char *password = "song940@163.com"; // Your Password

const char *mqtt_server = "lsong.me";
const char *mqtt_user = "your_username";
const char *mqtt_password = "your_password";

float humi;
float temp;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  // WiFi.mode(WIFI_AP);
  // WiFi.softAP(ssid, password);
  // Serial.println(WiFi.softAPIP());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(MQTTcallback);

  while (!mqtt.connected())
  {
    Serial.println("Connecting to MQTT...");
    if (mqtt.connect("esp8266-dht"))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.println(mqtt.state()); //If you get state 5: mismatch in configuration
      delay(2000);
    }
  }

  dht.begin();
  server.begin();
  Serial.println("Server started");
  mqtt.publish("esp/test", "Hello from ESP8266");
  mqtt.subscribe("esp/test");
}

double kelvin(double celsius)
{
  return celsius + 273.15;
}

double fahrenheit(double celsius)
{
  return ((double)(9 / 5) * celsius) + 32;
}

void loop()
{
  mqtt.loop();
  //Read data and store it to variables hum and temp
  humi = dht.readHumidity();
  temp = dht.readTemperature();
  //Print temp and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(humi);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");

  WiFiClient client = server.available();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close"); // the connection will be closed after completion of the response
  client.println("Refresh: 5");        // refresh the page automatically every 5 sec
  client.println();
  client.println("<!DOCTYPE html>");
  client.println("<html xmlns='http://www.w3.org/1999/xhtml'>");
  client.println("<head>\n<meta charset='UTF-8'>");
  client.println("<title>ESP8266 Temperature & Humidity DHT11 Sensor</title>");
  client.println("</head>\n<body>");
  client.println("<H2>ESP8266 & DHT11 Sensor</H2>");
  client.println("<H3>Humidity / Temperature</H3>");
  client.println("<pre>");
  client.print("Humidity    (%)   : ");
  client.println((float)humi, 2);
  client.print("Temperature (°C)  : ");
  client.println((float)temp, 2);
  client.print("Temperature (°F)  : ");
  client.println(fahrenheit(temp), 2);
  client.print("Temperature (°K)  : ");
  client.println(kelvin(temp), 2);
  client.println("</pre>");
  client.print("</body></html>");
  delay(2000); //Delay 2 sec.
  mqtt.publish("esp/dht/humi", String(humi).c_str());
  mqtt.publish("esp/dht/temp", String(temp).c_str());
}

void MQTTcallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");

  String message;
  for (int i = 0; i < length; i++)
  {
    message = message + (char)payload[i]; //Conver *byte to String
  }

  Serial.print(message);

  if (message == "#on")
  {
    digitalWrite(LED_BUILTIN, LOW);
  } //LED_BUILTIN on
  if (message == "#off")
  {
    digitalWrite(LED_BUILTIN, HIGH);
  } //LED_BUILTIN off
  Serial.println();
}