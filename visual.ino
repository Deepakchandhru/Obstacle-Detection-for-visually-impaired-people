#include <WiFi.h>
#include <HTTPClient.h>

#define TRIG_PIN 26
#define ECHO_PIN 25
#define BUZZER_PIN 32

#define MAX_DISTANCE 100

// Wi-Fi credentials
const char* ssid = "**";      // Replace with your Wi-Fi SSID
const char* password = "**";  // Replace with your Wi-Fi password

String serverUrl = "http:// w.x.y.z:5000/data"; 
long duration, predist;
int distance;

int distances = 0;
String infer;

void setup() {
  Serial.begin(115200);
  delay(1000); 

  Serial.println("Booting...");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 15) {
    delay(1000);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
  } else {
    Serial.println("\nFailed to connect to WiFi.");
  }
}

int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 25000); // Timeout to prevent hanging
  distance = duration * 0.034 / 2;
  return distance;
}

String diff(int pre, int dist) {
  int differ = pre - dist;
  String inference = "";

  if (differ != 0) {
    inference += "Object is moving ";
    if (differ > 0) {
      inference += "towards you at ";
    } else {
      inference += "away from you at ";
      differ *= -1;
    }
    inference += String(differ) + " cm/second ";
  } else {
    inference += "Object is stationary ";
  }
  return inference;
}

void produceSound(int hz) {
  tone(BUZZER_PIN, hz);
  delay(100);
  noTone(BUZZER_PIN);
}

void sendOutputToServer(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"message\": \"" + message + "\"}";
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print("Message sent. Server responded with code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending message: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected. Cannot send message.");
  }
}

void loop() {
  predist = distances;
  distances = getDistance(TRIG_PIN, ECHO_PIN);

  Serial.print("Distance: ");
  Serial.print(distances);
  Serial.println(" cm");

  if (distances <= MAX_DISTANCE) {
    infer = "Object at " + String(distances) + " cm. " +
                    diff(predist, distances);
    Serial.println(infer);
    sendOutputToServer(infer);
    produceSound(200); 
  }

  Serial.println();
  delay(1000);
}