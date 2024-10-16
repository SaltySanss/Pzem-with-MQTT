
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>

WiFiClient   espClient;
PubSubClient client(espClient);                     

char  ssid[]             = "";           //Wifi user
char  password[]         = "";           // Wifi password
const char* mqttserver   = "";      //ip home assistant
const int   mqttPort     = 1883;
const char* mqttclient   = "test";             //กรณีมีบอร์ดหลายตัวจำเป็นต้องตั้งชื่อ client ให้แตกต่างกันเพื่อส่งไป mqtt
const char* mqttUser     = "";
const char* mqttPassword = "";
const char* Topic        = "test";             //ชื่อ topic ที่จะส่งไปยัง mqtt


PZEM004Tv30 pzem(D6,D5); //to (TX,RX) of PZEM for Node MCU
float V, I, P, E, f, PF;

////////////////////////////////////////////////////////

char ntp_server1[20] = "1.asia.pool.ntp.org";
char ntp_server2[20] = "1.th.pool.ntp.org";
char ntp_server3[20] = "time.navy.mi.th";

int  timezone = 7 * 3600;                     //timezone for Thailand is +7
int  dst = 0;

String  Month[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
char    daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"}; 
String  Day,DATE1,DATE2,TIME,myIP,SignalStrength ;
int     Signal;


////////////////////////////////////////////////////////


void setup() {
  Serial.begin(115200);

  Wificonnection();
  myIP = ipToString( WiFi.localIP() ); 
  configTime(timezone, dst, ntp_server1, ntp_server2, ntp_server3);
  Serial.println();     
}

void loop() {
  
  if (WiFi.status() != WL_CONNECTED || !client.connected())
  {
    Wificonnection();
  }
  
  Signal  = WiFi.RSSI();
  SignalStrength = String(Signal) + " dBm";
  Serial.println("Signal Strength : "+ SignalStrength);  
  Serial.println(TimeSync());
  Serial.println(Day);
  Serial.println(DATE1);
  Serial.println(DATE2);
  Serial.println(TIME);
  Serial.println();
  
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

  TomqttMSG("DD"    ,DATE1);
  TomqttMSG("TT"    ,TIME);
  TomqttVal("WiFI"  ,Signal);
  
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

void TomqttMSG(const char* Name, String value)    //Function นี้จะส่งข้อมูลทีละตัวเป็นไฟล์ json ไปยัง mqtt 
{
  StaticJsonDocument<300> tomqtt;
  tomqtt[Name] = value;
  char mqtt[100];
  serializeJsonPretty(tomqtt, mqtt);
  client.publish(Topic, mqtt);                    //(ชื่อ topic ที่จะส่งไปยัง mqtt, ชุดข้อมูล json ที่จะส่ง) โดยใน function นี้จะส่งข้อมูลทีละตัว
  serializeJsonPretty(tomqtt, Serial);
  Serial.println();
}

////////////////////////////////////////////////////////

String zero(byte num)
{
  if ( num<10 )
  { 
    return "0" + String(num);
  }
  else
  { 
    return String(num);
  }
}


////////////////////////////////////////////////////////

String TimeSync()
{
  int     h,m,s,md,mo,y,wd,yd;
  
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  h  = p_tm->tm_hour; 
  m  = p_tm->tm_min; 
  s  = p_tm->tm_sec;  
  md = p_tm->tm_mday; 
  mo = p_tm->tm_mon  + 1;
  y  = p_tm->tm_year + 1900;
  wd = p_tm->tm_wday;
  yd = p_tm->tm_yday;
  
  Day   = String(daysOfTheWeek[wd]); 
  DATE1 = String(md)+ " " + Month[mo-1] + " " + String(y);
  DATE2 = String(md)+ "/" + zero(mo)    + "/" + String(y);
  TIME  = zero(h) + ":" + zero(m) + ":" + zero(s);

  return DATE1 + "  " + TIME;
}

///////////////////////////////////////////////////////////////////////////
