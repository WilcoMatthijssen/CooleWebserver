#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <SPI.h>
#include <wifi.h>




WebServer server(80);

// PWM config
const int pwmPins[4] = {16, 17, 18, 19};
const int pwmCh[4]   = {0, 1, 2, 3};
int pwmVal[4] = {0,0,0,0};

void setup() {
  Serial.begin(115200);
  Serial.println("started");


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
delay(500);
Serial.print('.');
  } 

  Serial.println(WiFi.localIP());

  // Start filesystem
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  // PWM setup
  for (int i = 0; i < 4; i++) {
    ledcSetup(pwmCh[i], 5000, 8);
    ledcAttachPin(pwmPins[i], pwmCh[i]);
  }


  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });
  server.on("/chart.js", HTTP_GET, []() {
  File file = LittleFS.open("/chart.js", "r");
  server.streamFile(file, "application/javascript");
  file.close();
});

  server.on("/logo.gif", HTTP_GET, []() {
  File file = LittleFS.open("/logo.gif", "r");
  server.streamFile(file, "image/gif");
  file.close();
});
server.on("/api/gpio", HTTP_GET, []() {

  int pin = server.arg("pin").toInt();
  int state = server.arg("state").toInt();

  pinMode(pin, OUTPUT);
  digitalWrite(pin, state);

  server.send(200, "application/json", "{\"ok\":true}");
});

server.on("/api/adc", HTTP_GET, []() {
  int adcValue = random(0, 4095);
  server.send(200, "application/json", "{\"value\":" + String(adcValue) + "}");
});

  server.on("/api/pwm", HTTP_GET, []() {
    int ch = server.arg("ch").toInt();
    int val = server.arg("val").toInt();

    val = constrain(val, 0, 255);

    if (ch >= 0 && ch < 4) {
      pwmVal[ch] = val;
      ledcWrite(pwmCh[ch], val);
    }

    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.begin();
}

void loop() {
  server.handleClient();
}