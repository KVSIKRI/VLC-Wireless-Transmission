#include <WiFi.h>
#include <esp_now.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <math.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "esp_wifi.h"

#define GPS_RX 16
#define GPS_TX 17
#define BUZZER_PIN 4
#define LED_PIN 2
#define WIFI_CHANNEL 1  // Common channel

TinyGPSPlus gps;
HardwareSerial SerialGPS(2);
AsyncWebServer server(80);

typedef struct struct_message {
  bool crashDetected;
  bool tiltDetected;
  float latitude;
  float longitude;
} struct_message;

struct_message incomingData;

double txLat = 0, txLng = 0;
double rxLat = 0, rxLng = 0;
double distanceMeters = 0.0;

// ---- Status Flags ----
volatile bool crashActive = false;       // latest received crash flag
volatile bool tiltActive  = false;

// ---- Crash Latching (5s hold) ----
volatile bool crashLatched = false;
volatile unsigned long lastCrashTime = 0;
const unsigned long CRASH_HOLD_MS = 5000;  // 5 seconds

// Distance calculation (Haversine)
double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
  if (!lat1 || !lon1 || !lat2 || !lon2) return 0.0;
  const double R = 6371000.0;
  double dLat = (lat2 - lat1) * M_PI / 180.0;
  double dLon = (lon2 - lon1) * M_PI / 180.0;
  double a = sin(dLat/2)*sin(dLat/2) +
             cos(lat1*M_PI/180.0)*cos(lat2*M_PI/180.0) *
             sin(dLon/2)*sin(dLon/2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return R * c;
}

// ESP-NOW receive callback
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));
  txLat = incomingData.latitude;
  txLng = incomingData.longitude;
  crashActive = incomingData.crashDetected;
  tiltActive  = incomingData.tiltDetected;

  // 🔴 Latch crash for at least 5 seconds
  if (crashActive) {
    crashLatched = true;
    lastCrashTime = millis();
  }

  // Buzzer/LED handled in loop() based on latched state
}

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);

  // Dual mode: AP for web, STA for ESP-NOW
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("V2V_Receiver", "12345678", WIFI_CHANNEL);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  Serial.print("Receiver AP IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Receiver MAC: ");
  Serial.println(WiFi.softAPmacAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  // ---------- Web Server ----------
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    String page = R"rawliteral(
    <html>
    <head>
      <meta charset='utf-8'>
      <meta name='viewport' content='width=device-width,initial-scale=1'>
      <title>V2V Dashboard</title>
      <style>
        *{
          box-sizing:border-box;
          margin:0;
          padding:0;
        }
        body{
          background:#121212;
          color:white;
          font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Arial,sans-serif;
          display:flex;
          justify-content:center;
          align-items:center;
          min-height:100vh;
        }
        .wrapper{
          text-align:center;
          width:100%;
          max-width:1000px;
          padding:16px;
        }
        h1{
          margin-bottom:20px;
        }
        .cards{
          display:flex;
          flex-wrap:wrap;
          justify-content:center;
          align-items:center;
          gap:16px;
          margin-bottom:24px;
        }
        .card{
          margin:0;
          padding:20px;
          border-radius:12px;
          background:#1e1e1e;
          width:260px;
          box-shadow:0 4px 14px rgba(0,0,0,0.4);
        }
        .card h2{
          margin-bottom:8px;
        }
        .red{color:#ff5252}
        .yellow{color:#ffd600}
        .green{color:#00e676}
        .coords{
          font-size:1.1em;
          color:#ccc;
          word-break:break-word;
        }
        p.footer{
          margin-top:10px;
          color:#999;
          font-size:0.9em;
        }
      </style>
      <script>
        async function update(){
          try{
            const res = await fetch('/data');
            const d = await res.json();
            document.getElementById('status').innerHTML = d.status;
            document.getElementById('dist').innerHTML   = d.distance.toFixed(2) + " m";
            document.getElementById('tx').innerHTML     = d.txlat.toFixed(6) + ", " + d.txlng.toFixed(6);
            document.getElementById('rx').innerHTML     = d.rxlat.toFixed(6) + ", " + d.rxlng.toFixed(6);
          }catch(e){
            console.log(e);
          }
        }
        setInterval(update,1000);
        window.addEventListener('load',update);
      </script>
    </head>
    <body>
      <div class="wrapper">
        <h1>🚗 V2V Receiver Dashboard</h1>
        <div class="cards">
          <div class='card'>
            <h2>Status</h2>
            <div id='status' class='green'>Normal</div>
          </div>
          <div class='card'>
            <h2>Distance</h2>
            <div id='dist'>-- m</div>
          </div>
          <div class='card'>
            <h2>Transmitter</h2>
            <div id='tx' class='coords'>--</div>
          </div>
          <div class='card'>
            <h2>Receiver</h2>
            <div id='rx' class='coords'>--</div>
          </div>
        </div>
        <p class='footer'>ESP-NOW + Web Server (Ch 1)</p>
      </div>
    </body>
    </html>
    )rawliteral";
    req->send(200, "text/html", page);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *req) {
    // Use latched crash status so Crash shows at least 5s on UI
    bool crashDisplay = crashLatched;
    bool tiltDisplay  = (!crashDisplay && tiltActive);  // don't show tilt if crash is latched

    String st;
    if (crashDisplay)      st = "<span class='red'>🚨 Crash</span>";
    else if (tiltDisplay)  st = "<span class='yellow'>⚠ Tilt</span>";
    else                   st = "<span class='green'>✔ Normal</span>";

    String json = "{\"status\":\""+st+"\",\"distance\":"+String(distanceMeters,2)+
                  ",\"txlat\":"+String(txLat,6)+",\"txlng\":"+String(txLng,6)+
                  ",\"rxlat\":"+String(rxLat,6)+",\"rxlng\":"+String(rxLng,6)+"}";
    req->send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Web Server Started ✅");
}

void loop() {
  // GPS update
  while (SerialGPS.available()) {
    gps.encode(SerialGPS.read());
    if (gps.location.isUpdated() && gps.location.isValid()) {
      rxLat = gps.location.lat();
      rxLng = gps.location.lng();
    }
  }

  if (rxLat && rxLng && txLat && txLng)
    distanceMeters = calculateDistance(rxLat, rxLng, txLat, txLng);

  // ---- Handle crash latching timeout (5 seconds) ----
  if (crashLatched && (millis() - lastCrashTime >= CRASH_HOLD_MS)) {
    crashLatched = false;  // 5 seconds over, allow clearing
  }

  // ---- Drive LED + Buzzer based on latched states ----
  bool crashDisplay = crashLatched;
  bool tiltDisplay  = (!crashDisplay && tiltActive);  // tilt only if no crash

  if (crashDisplay) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000);     // Crash tone
  } else if (tiltDisplay) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1500);     // Tilt tone
  } else {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }
}
