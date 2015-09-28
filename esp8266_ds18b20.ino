 /*
  Created by Igor Jarc
 See http://iot-playground.com for details
 Please use community fourum on website do not contact author directly
 
 Code based on https://github.com/DennisSc/easyIoT-ESPduino/blob/master/sketches/ds18b20.ino
 
 External libraries:
 - https://github.com/adamvr/arduino-base64
 - https://github.com/milesburton/Arduino-Temperature-Control-Library
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */
#include <ESP8266WiFi.h>
#include <Base64.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//AP definitions
#define AP_SSID "yyyyyyyyyyy"
#define AP_PASSWORD "xxxxxxxxxxx"

// EasyIoT server definitions
#define USERNAME    "admin"
#define PASSWORD    "test"
#define IP_ADDRESS  "192.168.150.192"
#define PORT        8000
#define NODE        "N13S0"

#define REPORT_INTERVAL 900 // in sec


#define ONE_WIRE_BUS 2  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
#define SENSORS 2

String places[] = { "werkstatt", "lager" };

#define USER_PWD_LEN 40
char unameenc[USER_PWD_LEN];


void setup() {
  Serial.begin(115200);
  
  wifiConnect();
    
  char uname[USER_PWD_LEN];
  String str = String(USERNAME)+":"+String(PASSWORD);  
  str.toCharArray(uname, USER_PWD_LEN); 
  memset(unameenc,0,sizeof(unameenc));
  base64_encode(unameenc, uname, strlen(uname));
}

void loop() {
  float temp;
  int i = 0;
  for( i = 0; i<SENSORS; i++){
    do {
      DS18B20.requestTemperatures(); 
      temp = DS18B20.getTempCByIndex(i);
      Serial.print("Temperature ");
      Serial.print(i);
      Serial.print(":");
      Serial.println(temp);
    } while (temp == 85.0 || temp == (-127.0));
    sendTeperature(temp, places[i]);
  }
  
  int cnt = REPORT_INTERVAL;
  
  while(cnt--)
    delay(1000);
}

void wifiConnect()
{
    Serial.print("Connecting to AP");
    WiFi.begin(AP_SSID, AP_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");  
}

void sendTeperature(float temp, String place)
{  
   WiFiClient client;
   
   while(!client.connect(IP_ADDRESS, PORT)) {
    Serial.println("connection failed");
    wifiConnect(); 
  }

  String stemp = "value="+String(temp);
  int stemp_size = stemp.length(); 
  String url = "";
  url += "/sensor/"+place+"/setdata"; // generate EasIoT server node URL

  Serial.print("POST data to URL: ");
  Serial.println(url);
  
  Serial.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(IP_ADDRESS) +":"+ String(PORT)+ "\r\n" + 
               //"Connection: close\r\n" + 
               //"Authorization: Basic " + unameenc + " \r\n" + 
               "Content-Type: application/x-www-form-urlencoded \r\n" +
               "Content-Length: "+String(stemp_size)+"\r\n" + 
               "\r\n"+
               stemp);
               
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(IP_ADDRESS) +":"+ String(PORT)+ "\r\n" + 
               //"Connection: close\r\n" + 
               //"Authorization: Basic " + unameenc + " \r\n" + 
               "Content-Type: application/x-www-form-urlencoded \r\n" +
               "Content-Length: "+String(stemp_size)+"\r\n" + 
               "\r\n"+
               stemp);

  delay(100);
    while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("Connection closed");
}

