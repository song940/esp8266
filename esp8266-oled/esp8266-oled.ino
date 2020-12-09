#include <EEPROM.h>
#include <SSD1306.h>
#include <ESP8266mDNS.h>
#include <ESP8266SSDP.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>

#define BTN_UP    12 //GPIO 12 = D6
#define BTN_DOWN  13 //GPIO 13 = D7
#define BTN_OK    14 //GPIO 14 = D5
#define BTN_FLASH 0  //GPIO 0  = FLASH BUTTON
#define BTN_RESET 4
#define LED_1     2
#define LED_2     16
#define BUZZER    D3

#define WiFi_Logo_width 60
#define WiFi_Logo_height 36

const char WiFi_Logo_bits[] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x00,
0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF,
0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
0xFF, 0x03, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0xFF, 0xFF, 0xFF, 0x07, 0xC0, 0x83, 0x01, 0x80, 0xFF, 0xFF, 0xFF,
0x01, 0x00, 0x07, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0x00,
0xC0, 0xFF, 0xFF, 0x7C, 0x00, 0x60, 0x0C, 0x00, 0xC0, 0x31, 0x46, 0x7C,
0xFC, 0x77, 0x08, 0x00, 0xE0, 0x23, 0xC6, 0x3C, 0xFC, 0x67, 0x18, 0x00,
0xE0, 0x23, 0xE4, 0x3F, 0x1C, 0x00, 0x18, 0x00, 0xE0, 0x23, 0x60, 0x3C,
0x1C, 0x70, 0x18, 0x00, 0xE0, 0x03, 0x60, 0x3C, 0x1C, 0x70, 0x18, 0x00,
0xE0, 0x07, 0x60, 0x3C, 0xFC, 0x73, 0x18, 0x00, 0xE0, 0x87, 0x70, 0x3C,
0xFC, 0x73, 0x18, 0x00, 0xE0, 0x87, 0x70, 0x3C, 0x1C, 0x70, 0x18, 0x00,
0xE0, 0x87, 0x70, 0x3C, 0x1C, 0x70, 0x18, 0x00, 0xE0, 0x8F, 0x71, 0x3C,
0x1C, 0x70, 0x18, 0x00, 0xC0, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x08, 0x00,
0xC0, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x0C, 0x00, 0x80, 0xFF, 0xFF, 0x1F,
0x00, 0x00, 0x06, 0x00, 0x80, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x07, 0x00,
0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0xF8, 0xFF, 0xFF,
0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x00,
0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF,
0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00, 0x00,
0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const char *ssid     = "wifi@lsong.org";
const char *password = "song940@163.com";

const char *menu[] = {
  "Menu 1",
  "Menu 2",
  "Menu 3",
  "Settings",
  NULL
};

int cursor = 0;
int current = 0;
int keyCode = 0;
int progress = 0;
bool canBtnPress = true;

int buffer[128];
int lastMsg = 0;

WiFiClient network;
PubSubClient mqtt(network);
ESP8266WebServer server(80);
SSD1306 display(0x3c, D1, D2);

void setup() {

  pinMode(LED_1    , OUTPUT);
  pinMode(LED_2    , OUTPUT);
  pinMode(BUZZER   , OUTPUT);
  pinMode(BTN_FLASH, INPUT);
  pinMode(BTN_RESET, INPUT_PULLUP);
  pinMode(BTN_DOWN , INPUT_PULLUP);
  pinMode(BTN_OK   , INPUT_PULLUP);
  pinMode(BTN_UP   , INPUT_PULLUP);
 
  display.init();
  display.flipScreenVertically();
  display.setContrast(255);
  display.setFont(ArialMT_Plain_10);
  //
  EEPROM.begin(4096);
  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);
  Serial.begin(115200);
  Serial.println("\nStarting...\n");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED && progress<=100) {
    loading();
    Serial.print ( "." );
    blink();
  }

  mqtt.setServer("lsong.org", 1883);
  mqtt.setCallback(onMessage);

  server.on ( "/", handleRoot );
  server.on ( "/test.svg", drawGraph );
  server.on ( "/display", []() {
    display.clear();
    display.drawStringMaxWidth(0, 0, 128, server.arg("msg"));
    display.display();
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.on("/description.xml", HTTP_GET, [](){
    SSDP.schema(server.client());
  });
  server.onNotFound ( handleNotFound );
  server.begin();

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );
  Serial.println ( "HTTP server started" );

  Serial.printf("Starting SSDP...\n");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("Philips hue clone");
  SSDP.setSerialNumber("001788102201");
  SSDP.setURL("index.html");
  SSDP.setModelName("Philips hue bridge 2012");
  SSDP.setModelNumber("929000226503");
  SSDP.setModelURL("http://www.meethue.com");
  SSDP.setManufacturer("Royal Philips Electronics");
  SSDP.setManufacturerURL("http://www.philips.com");
  SSDP.begin();

  while(progress<=100){
    loading();
  }

  display.clear();
  display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, WiFi_Logo_height + 16, WiFi.localIP().toString());
  display.display();
  delay(3000);
  drawMenu();
  play("1d2f1e");
}

void onMessage(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void play(char* melody) {
  int length = sizeof(melody) / sizeof(char);;
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
  for (int i = 0; i < length / 2; i+=2) {
    int d = (int)melody[i] - 48;
    char t = melody[i+1];
    for(int j=0;j<8;j++){
      if(t == names[j]){
        tone(BUZZER, tones[j], tones[j] * 2);
        delay(d * 300);
      }
    }
  }
  noTone(BUZZER);
}

void blink(){
  int s = digitalRead(LED_2);
  digitalWrite (LED_2, !s);
  delay(100);
  digitalWrite (LED_2, s);
}

void loading(){
  display.clear();
  display.drawProgressBar(0, 32, 120, 10, progress);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, String(progress++) + "%");
  display.display();
}

void loop() {

  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt.publish("lsong", "esp8266 is online");
      // ... and resubscribe
      mqtt.subscribe("esp8266");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

  mqtt.loop();
  server.handleClient();
  if(Serial.available()){
    String input = Serial.readString();
  }

  scanButton();

  if(current == 1){
    for(int i=0;i<127;i++){
      buffer[i] = buffer[i+1];
    }
    buffer[127] = rand() % 64;
    display.clear();
    for(int i=0;i<128;i++) display.drawLine(i, 64, i, 64-buffer[i]);
    display.display();
  }
  
  long now = millis();
  if(current == 2){
    if (now - lastMsg > 2000) {
      lastMsg = now;
      Serial.print("Publish message: ");
      mqtt.publish("lsong", "hello");
    }
  }
}

void drawMenu(){
  int i = 0;
  int fontSize = 12;
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  while(menu[i]){
    display.drawString(10, i * fontSize, menu[i]);
    i++;
  }
  display.drawString(0, cursor * fontSize, "->");
  display.display();
}


void scanButton(){
  int a = digitalRead(BTN_OK);
  int b = digitalRead(BTN_UP);
  int c = digitalRead(BTN_DOWN);
  if(a == LOW || b == LOW || c == LOW){
     if(canBtnPress) {
        canBtnPress = false;
        if(a == LOW) keyCode = 0;
        if(b == LOW) keyCode = 1;
        if(c == LOW) keyCode = 2;
      }
  } else if(!canBtnPress){
    canBtnPress = true;
    Serial.println ( "keyCode: " + keyCode );
    doSwitch(keyCode);
  }
}

void doSwitch(int code){
  current = -1;
  if(code == 1 && cursor > 0){
    cursor--;
    drawMenu();
  }
  if(code == 2 && cursor < 3){
    cursor++;
    drawMenu();
  }
  if(code == 0){
    current = cursor;
    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "cursor: " + String(cursor));
    display.display();
  }
}

void handleRoot() {
  char temp[512];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr  = min / 60;
  snprintf ( temp, 512,
    "<html>\
      <head>\
        <meta http-equiv='refresh' content='10'/>\
        <title>ESP8266 Demo</title>\
        <style>\
          body{ width: 50%; margin: auto; text-align: center; }\
          button, input, textarea{ display: block; width: 100%; }\
        </style>\
      </head>\
      <body>\
        <h1>Hello from ESP8266!</h1>\
        <p>Uptime: %02d:%02d:%02d</p>\
        <img src=\"/test.svg\" />\
        <form action='/display' >\
          <textarea name='msg' ></textarea>\
          <button type='submit' >submit</button>\
        </form>\
      </body>\
    </html>", hr, min % 60, sec % 60
  );
  server.send ( 200, "text/html", temp );
}

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x+= 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";
  server.send ( 200, "image/svg+xml", out);
}

void handleNotFound() {
  blink();
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 404, "text/plain", message );
}
