
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>

WiFiClient   espClient;
PubSubClient client(espClient);                     

char  ssid[]             = "Salty";           //Wifi user
char  password[]         = "SaltySanss";           // Wifi password
const char* mqttserver   = "192.168.88.184";
const int   mqttPort     = 1883;
const char* mqttclient   = "test";             //กรณีมีบอร์ดหลายตัวจำเป็นต้องตั้งชื่อ client ให้แตกต่างกันเพื่อส่งไป mqtt
const char* mqttUser     = "saltysanss";
const char* mqttPassword = "san08753";
const char* Topic        = "test";             //ชื่อ topic ที่จะส่งไปยัง mqtt

String myIP;

PZEM004Tv30 pzem(D6,D5); //to (TX,RX) of PZEM for Node MCU
float V, I, P, E, f, PF;

void setup() {
  Serial.begin(115200);

  Wificonnection();
  myIP = ipToString( WiFi.localIP() ); 

  Serial.println();     
}

void loop() {
  
  if (WiFi.status() != WL_CONNECTED || !client.connected())
  {
    Wificonnection();
  }

  V = pzem.voltage();
  I = pzem.current();
  P = pzem.power();
  E = pzem.energy();
  f = pzem.frequency();
  PF = pzem.pf();

  TomqttVal("V"  ,V);
  TomqttVal("I"  ,I);
  TomqttVal("P"  ,P);
  TomqttVal("E"  ,E);
  TomqttVal("f"  ,f);
  TomqttVal("PF"  ,PF);
  
  delay(1000);
}


void Wificonnection()
{
  WiFi.begin(ssid, password);
  int count = 0;

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Wifi Connecting");
    client.setServer(mqttserver, mqttPort);
    while (!client.connected()) 
    {
      Serial.println("connecting mqtt");
      if (client.connect(mqttclient, mqttUser, mqttPassword)) 
      {
        Serial.println("mqttconnect");
      } 
      else 
      {
        Serial.print("failed to connect");
        Serial.println(client.state());
        delay(2000);
      }
    }
    Serial.println("Connected");
  }
}

String ipToString(IPAddress ip)
{
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

void TomqttVal(const char* Name, float value)     //Function นี้จะส่งข้อมูลทีละตัวเป็นไฟล์ json ไปยัง mqtt 
{
  StaticJsonDocument<300> tomqtt;
  tomqtt[Name] = value;
  char mqtt[100];
  serializeJsonPretty(tomqtt, mqtt);
  client.publish(Topic, mqtt);                    //(ชื่อ topic ที่จะส่งไปยัง mqtt, ชุดข้อมูล json ที่จะส่ง) โดยใน function นี้จะส่งข้อมูลทีละตัว
  serializeJsonPretty(tomqtt, Serial);
  Serial.println();
}
