#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "mobile@lsong.org"
#define wifi_pass "song940@163.com"

#define mqtt_server "lsong.me"
#define mqtt_user "your_username"
#define mqtt_password "your_password"

WiFiClient wifi;
PubSubClient mqtt(wifi);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_pass);
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
    if (mqtt.connect("ESP8266"))
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

  mqtt.publish("esp/test", "Hello from ESP8266");
  mqtt.subscribe("esp/test");
}

void loop()
{
  mqtt.loop();
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
  Serial.println("-----------------------");
}