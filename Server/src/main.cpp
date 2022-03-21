#include <Arduino.h>
#include <WiFi.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <esp_timer.h>

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void upload_html_page();
void wifi_setup();
void upload_buttons_page();
void upload_adc_page();

AsyncWebServer server(80);

int state = 0;
int state19 = 0, state22 = 0, state23 = 0;

int gpio35_val = 0,gpio34_val = 0,gpio33_val = 0,gpio32_val = 0;

const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

char* html_page;
size_t size_of_page;

char* adc_page;
size_t size_of_adc_page;

char* buttons_page;
size_t size_of_buttons_page;




void setup() {
  Serial.begin(115200);
  SPIFFS.begin();

  pinMode(22,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(19,OUTPUT);
  //adc
  pinMode(32,INPUT);
  pinMode(33,INPUT);
  pinMode(34,INPUT);
  pinMode(35,INPUT);
  //pwm
  ledcSetup(1,15000,8);
  ledcAttachPin(21,1);


  SPIFFS.begin();
  

  upload_html_page();
  upload_buttons_page();
  upload_adc_page();

  wifi_setup();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", html_page);
    
  });

  server.on("/19", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(19,!state19);
    state19 = !state19;
    request->send_P(200, "text/html", html_page);
    Serial.println("19!");
  });

  server.on("/22", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(22,!state22);
    state22 = !state22;
    Serial.println("22!");
    Serial.println(request->args());
    request->send_P(200, "text/html", buttons_page);
  });

  server.on("/23", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(23,!state23);
    state23 = !state23;
    Serial.println("23!");
    request->send_P(200, "text/html", buttons_page);
  });

  server.on("/buttons", HTTP_GET, [](AsyncWebServerRequest *request){

    request->send_P(200, "text/html", buttons_page);
  });

  server.on("/web_server", HTTP_GET, [](AsyncWebServerRequest *request){

    request->send_P(200, "text/html", html_page);
    
  });

   server.on("/adc", HTTP_GET, [](AsyncWebServerRequest *request){

    request->send_P(200, "text/html", adc_page);
    
  });

  server.on("/update_adc", HTTP_GET, [](AsyncWebServerRequest *request){

    gpio34_val = analogRead(34);
    gpio35_val = analogRead(35);
    gpio33_val = analogRead(33);
    gpio32_val = analogRead(32);

    String gpio35_str(gpio35_val);
    String gpio34_str(gpio34_val);
    String gpio33_str(gpio33_val);
    String gpio32_str(gpio32_val);
    String output(gpio35_str+'.'+gpio34_str+'.'+gpio33_str+'.'+gpio32_str);

    Serial.println(gpio35_str);
    Serial.println(gpio34_str);
    Serial.println(gpio33_str);
    Serial.println(gpio32_str);
    gpio35_str+=gpio34_str+=gpio33_str+=gpio32_str;
    Serial.println(output);
    request->send(200,"text/plane", output);
    
  });

  server.on("/form", HTTP_GET, [](AsyncWebServerRequest *request){
    
    AsyncWebParameter* ptr = request->getParam("range_input");
    const String & str = ptr->value();
   // request->send_P(200, "text/html", buttons_page);
    Serial.println(str.toInt());
    Serial.println(str.toInt()*255/100);
    ledcWrite(1, str.toInt()*255/100);
  });

 /* server.on("/buttons", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", buttons_page);
  });*/
  //request->send(SPIFFS, "/Button-Cyan.png", "image/png");

  /*server.on("/buttons", HTTP_GET, [](AsyncWebServerRequest *request){

    request->send_P(200, "buttons/html", buttons_page);
    digitalWrite(22,!state);
    state = !state;
  });*/


  server.begin();
  // put your setup code here, to run once:
}

void loop() {
  
  // put your main code here, to run repeatedly:
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Соединение установлено");
  uint8_t mac_arr[6];
  // что-то делаем, например, вычисляем mac-адрес
  for (int i = 0; i < 6; i++) {
    mac_arr[i] = info.sta_connected.mac[i];
    Serial.printf("%02X", info.sta_connected.mac[i]);
    if (i < 5)Serial.print(":");
  }
  
  Serial.println("\n------------");
  Serial.println(WiFi.softAPgetStationNum());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {

  Serial.println("Соединение разорвано");

  // что-то делаем, например, снова вычисляем mac-адрес
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info.sta_disconnected.mac[i]);
    if (i < 5)Serial.print(":");
  }

  Serial.println("\n------------");
}

void upload_html_page()
{
  //PAGE!
  File file = SPIFFS.open("/web_server.html");

  size_of_page = file.size() + 1;
  html_page = new char[size_of_page];

  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("File Content:");

  {
    int i = 0;
    while(file.available())
    {
      char a = file.read();
      html_page[i++] = a;
    }
    html_page[i] = '\0';
  }

  Serial.println(html_page);

  file.close();
}

void wifi_setup()
{
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password, 1, 0, 1);

  WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_AP_STACONNECTED);
  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_AP_STADISCONNECTED);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void upload_buttons_page()
{
  File file = SPIFFS.open("/buttons.html");

  size_of_buttons_page = file.size() + 1;
  buttons_page = new char[size_of_buttons_page];

  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("File Content:");
  {
    int i = 0;
    while(file.available())
    {
      char a = file.read();
      buttons_page[i++] = a;
    }
    buttons_page[i] = '\0';
  }

  Serial.println(buttons_page);
    
  file.close();
}

void upload_adc_page()
{
  File file = SPIFFS.open("/adc.html");

  size_of_adc_page = file.size() + 1;
  adc_page = new char[size_of_adc_page];

  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("File Content:");
  {
    int i = 0;
    while(file.available())
    {
      char a = file.read();
      adc_page[i++] = a;
    }
    adc_page[i] = '\0';
  }

  Serial.println(adc_page);
    
  file.close();
}