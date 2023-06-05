#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <vector>
#include <math.h>

int data[3];

// Replace with your network credentials
const char* ssid = "Vivo v20";
const char* password = "00000000";

// Replace with your MQTT broker IP address
const char* mqtt_server = "192.168.48.252";

// Create an instance of the WiFiClient class
WiFiClient espClient;

// Create an instance of the PubSubClient class
PubSubClient client(espClient);

#define N 3

float mu[N] = {100, 130, 98};
//float mu[N] = {80, 120, 98};
float sigma[N][N] = {{400, 200, 100}, {200, 600, 300}, {100, 300, 250}};

float L[N][N] = {{0}};

void setup() {
  // Start the serial communication
  Serial.begin(9600);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Connect to MQTT broker
  client.setServer(mqtt_server, 1883);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect("arduinoClient2")) {
      Serial.println("Connected to MQTT broker.");
      for (int i = 0; i < N; i++) {
      for (int j = 0; j < (i+1); j++) {
      float s = 0;
      for (int k = 0; k < j; k++) {
        s += L[i][k] * L[j][k];
      }
      L[i][j] = (i == j) ? sqrt(sigma[i][i] - s) :
                     (1.0 / L[j][j] * (sigma[i][j] - s));
    }
  }
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {

   float x[N] = {0};
  for (int i = 0; i < N; i++) {
    x[i] = random(10000) / 10000.0;
  }

  float y[N] = {0};
  for (int i = 0; i < N; i++) {
    float s = 0;
    for (int j = 0; j < (i+1); j++) {
      s += L[i][j] * x[j];
    }
    y[i] = s + mu[i];
  }
  
  // Publish a message to the topic "sensor_data"
  int arr[3];
  arr[0] = y[0];  // BPM
  arr[1] = y[1]; // BP
  arr[2] = y[2]; // spo2
  String msg = "";
  for(int i=0 ; i<3 ; i++){
    msg += String(arr[i]); 
    if(i<2){
      msg += ",";
    } 
  }

//  String msg = "5,7,9";
  client.publish("ICU/patient2", msg.c_str());
  Serial.print("Message published: ");
  Serial.println(msg);
  delay(10000);
}
