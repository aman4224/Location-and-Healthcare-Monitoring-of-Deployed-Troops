#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include<SoftwareSerial.h>
#include <TinyGPS.h>
TinyGPS gps;
SoftwareSerial ss(D3, D0);
float flat, flon;
unsigned long age;
#define ASEN_PIN A0//tilt
#define ONE_WIRE_BUS D5 //temperature 
#define DSEN2_PIN D2//pulse
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
#define LED1_PIN D7
String buf;
String dsen1;
String dsen2;
int dsenc1;
int dsenc2;
int t = 0;

// Update these with values suitable for your network.
const char* ssid = "Oneplus6";
const char* password = "amanverma4224";
//const char* mqtt_server = "test.mosquitto.org";
const char* mqtt_server = "iot.eclipse.org";
//const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  delay(100);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "prap";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      //once connected to MQTT broker, subscribe command if any
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} //end reconnect()

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  ss.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  pinMode(LED1_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  pinMode(ASEN_PIN, INPUT);
  pinMode(DSEN2_PIN, INPUT);
}

void loop() {
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      //Serial.write(c); // uncomment this line if you want to see the GPS data flowing      delay(1000);
      if (gps.encode(c))
      {
        newData = true;
      }
    }
  }

  if (newData)
  {
    gps.f_get_position(&flat, &flon, &age);
  }
  //buf = F("lat:");
  buf = String(flat, 6);
  buf += F(",");
  //buf += F("\nlon:");
  buf += String(flon, 6);
  sensors.requestTemperatures();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    dsen1 = sensors.getTempCByIndex(0);
    dsenc1 = sensors.getTempCByIndex(0);
    dsen2 = digitalRead(DSEN2_PIN);
    dsenc2 = digitalRead(DSEN2_PIN);
    char message1[4];
    char message2[4];
    dsen1.toCharArray(message1, 4);
    dsen2.toCharArray(message2, 4);
    //    client.publish("sen1", message);
    //    client.publish("sen2", message1);
    //    client.publish("sen3", message2);
    //    client.publish("sen4", message3);
    Serial.print("sen2=");
    Serial.println(dsen1);
    Serial.print("sen3=");
    Serial.println(dsen2);
    if (dsenc1 > 40)
    {
      Serial.println("HIGH");
      digitalWrite(LED1_PIN, HIGH);
      client.publish("prap2", "TEMPERATURE HIGH/FEVER");
      delay(500);
    }
    if (dsenc1 < 40)
    {
      Serial.println("NORMAL");
      digitalWrite(LED1_PIN, LOW);
      client.publish("prap2", "TEMPERATURE NORMAL");
      delay(500);
    }
    if (dsenc2 == 1)
    {
      t = t + 1;
      Serial.println(t);
    }
    if (t > 15)
    {
      Serial.println("LOW");
      digitalWrite(LED1_PIN, HIGH);
      client.publish("prap3", "PULSE LOW");
      delay(500);
    }
    if (dsenc2 == 0)
    {
      t = 0;
      Serial.println("NORMAL");
      digitalWrite(LED1_PIN, LOW);
      client.publish("prap3", "PULSE NORMAL");
      delay(500);
    }
    char charBuf[50];
    buf.toCharArray(charBuf, 50);
    client.publish("aac2", charBuf);
    Serial.print("LOCATION= ");
    Serial.println(charBuf);
    buf = "";
  }
}
