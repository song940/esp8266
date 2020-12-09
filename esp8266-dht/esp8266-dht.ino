#include <ESP8266WiFi.h>
#include <DHT.h>;

#define DHTPIN 2          // what pin we're connected to
#define DHTTYPE DHT22     // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
WiFiServer server(80);

const char *ssid = "esp8266";             // Your ssid
const char *password = "song940@163.com"; // Your Password

float humi;
float temp;

void setup()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  dht.begin();
  server.begin();

  Serial.begin(115200);
  Serial.println(WiFi.softAPIP());
  Serial.println("Server started");
}

double Fahrenheit(double celsius)
{
  return ((double)(9 / 5) * celsius) + 32;
}

double Kelvin(double celsius)
{
  return celsius + 273.15;
}

void loop()
{
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
  client.print("Humidity (%)     : ");
  client.println((float)humi, 2);
  client.print("Temperature (°C)  : ");
  client.println((float)temp, 2);
  client.print("Temperature (°F)  : ");
  client.println(Fahrenheit(temp), 2);
  client.print("Temperature (°K)  : ");
  client.println(Kelvin(temp), 2);
  client.println("</pre>");
  client.print("</body>\n</html>");
  delay(1000); //Delay 2 sec.
}
