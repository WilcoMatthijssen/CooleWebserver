#include <Arduino.h>
#include <LittleFS.h>
#include <SPI.h>
#include <Ethernet.h>

// ===== SERVER =====
EthernetServer server(80);

// ===== STATE =====
int pwmVal[4] = {0, 0, 0, 0};

// ===== PINS (adjust for Opta!) =====
const int pwmPins[4] = {3, 5, 6, 9}; // NOTE: change to real Opta PWM-capable pins

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  Serial.println("Opta starting...");

  // ===== Ethernet =====
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  Ethernet.begin(mac);

  delay(1000);
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());

  server.begin();

  // ===== FS =====
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
  }

  // ===== PWM setup (Opta-safe) =====
  for (int i = 0; i < 4; i++) {
    pinMode(pwmPins[i], OUTPUT);
    analogWrite(pwmPins[i], 0);
  }
}

// ===== LOOP =====
void loop() {
  EthernetClient client = server.available();
  if (!client) return;

  String req = "";
  bool blank = false;

  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      req += c;

      if (c == '\n' && blank) break;
      blank = (c == '\n');
    }
  }

  // =========================
  // ROUTING (manual parsing)
  // =========================

  // ===== INDEX =====
  if (req.indexOf("GET / ") >= 0) {
    File file = LittleFS.open("/index.html", "r");

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();

    while (file.available()) client.write(file.read());
    file.close();
  }

  // ===== CHART JS =====
  else if (req.indexOf("GET /chart.js") >= 0) {
    File file = LittleFS.open("/chart.js", "r");

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/javascript");
    client.println();

    while (file.available()) client.write(file.read());
    file.close();
  }

  // ===== GIF =====
  else if (req.indexOf("GET /logo.gif") >= 0) {
    File file = LittleFS.open("/logo.gif", "r");

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: image/gif");
    client.println();

    while (file.available()) client.write(file.read());
    file.close();
  }

  // ===== GPIO API =====
  else if (req.indexOf("GET /api/gpio") >= 0) {

    int pinIndex = req.indexOf("pin=");
    int stateIndex = req.indexOf("state=");

    int pin = req.substring(pinIndex + 4, pinIndex + 6).toInt();
    int state = req.substring(stateIndex + 6, stateIndex + 7).toInt();

    pinMode(pin, OUTPUT);
    digitalWrite(pin, state);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println();
    client.println("{\"ok\":true}");
  }

  // ===== ADC API =====
  else if (req.indexOf("GET /api/adc") >= 0) {

    int adcValue = random(0, 4095);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println();

    client.print("{\"value\":");
    client.print(adcValue);
    client.println("}");
  }

  // ===== PWM API =====
  else if (req.indexOf("GET /api/pwm") >= 0) {

    int chIndex = req.indexOf("ch=");
    int valIndex = req.indexOf("val=");

    int ch = req.substring(chIndex + 3, chIndex + 4).toInt();
    int val = req.substring(valIndex + 4).toInt();

    val = constrain(val, 0, 255);

    if (ch >= 0 && ch < 4) {
      pwmVal[ch] = val;
      analogWrite(pwmPins[ch], val);
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println();
    client.println("{\"ok\":true}");
  }

  client.stop();
}