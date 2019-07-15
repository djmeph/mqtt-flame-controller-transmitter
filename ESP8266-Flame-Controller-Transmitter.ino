#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "wifi_credentials.h"

WiFiClient espClient;
PubSubClient client(espClient);

#define POOF_SLIDER A0
#define POOF_INPUT D1
#define WIFI_LED D7
#define MQTT_LED D8

uint8_t button_state;
int button_state_last = -1;
int debounce = 0;
const int debounce_time = 10;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    char receivedChar = (char)payload[i];
    Serial.print(receivedChar);
  }
  Serial.println();
}

void connectMqtt() {
  digitalWrite(MQTT_LED, LOW);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_clientId)) {
      digitalWrite(MQTT_LED, HIGH);
      Serial.println("connected");
      // client.subscribe("ledStatus");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setupWifi() {
  digitalWrite(WIFI_LED, LOW);
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid); 
  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  digitalWrite(WIFI_LED, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void button() {
  button_state = digitalRead(POOF_INPUT);
  if (button_state != button_state_last && millis() - debounce > debounce_time) {
    button_state_last = button_state;
    debounce = millis();
    if (button_state == LOW) {
      Serial.printf("Poof! %u\n", analogRead(POOF_SLIDER));
      client.publish(mqtt_topic, "Poof!");
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  pinMode(POOF_INPUT, INPUT_PULLUP);
  pinMode(POOF_SLIDER, INPUT);
  pinMode(WIFI_LED, OUTPUT);
  pinMode(MQTT_LED, OUTPUT);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
  }
  if (!client.connected()) {
    Serial.printf("Client connected: %s\n", client.connected() ? "TRUE" : "FALSE");
    connectMqtt();
  }
  client.loop();
  button();
}
