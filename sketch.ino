// Import library yang diperlukan
#include "ESP8266WiFi.h"
#include "ESPAsyncWebServer.h"

// Mendefinisikan Relay sebagai Normally Open (NO)
#define RELAY_NO    true

// Jumlah relay
#define NUM_RELAYS  1

// GPIO untuk relay
int relayGPIOs[NUM_RELAYS] = {5};

// Ganti dengan jaringan wifi
const char* ssid = "SSID_WIFI_ANDA";
const char* password = "PASSWORD";

const char* PARAM_INPUT_1 = "relay";  
const char* PARAM_INPUT_2 = "state";

// Buat objek AsyncWebServer pada port 80
AsyncWebServer server(80);

// Website
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      text-align: center;
    }
    h2 {
      font-size: 3.0rem;
    }
    p {
      font-size: 3.0rem;
    }
    body {
      max-width: 600px;
      margin: 0px auto;
      padding-bottom: 25px;
    }
    .switch {
      position: relative;
      display: inline-block;
      width: 120px;
      height: 68px;
    } 
    .switch input {
      display: none;
    }
    .slider {
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      border-radius: 34px;
    }
    .slider:before {
      position: absolute;
      content: "";
      height: 52px;
      width: 52px;
      left: 8px;
      bottom: 8px;
      background-color: #fff;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 68px;
    }
    input:checked + .slider {
      background-color: #2196F3;
    }
    input:checked + .slider:before {
      -webkit-transform: translateX(52px);
      -ms-transform: translateX(52px);
      transform: translateX(52px);
    }
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>
function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked) {
    xhr.open("GET", "/update?relay=" + element.id + "&state=1", true);
  } else {
    xhr.open("GET", "/update?relay=" + element.id + "&state=0", true);
  }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

// Ganti placeholder dengan bagian tombol di halaman web Anda
String processor(const String& var){
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for(int i=1; i<=NUM_RELAYS; i++){
      String relayStateValue = relayState(i);
      buttons+= "<h4>Relay #" + String(i) + " - GPIO " + relayGPIOs[i-1] + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" "+ relayStateValue +"><span class=\"slider\"></span></label>";
    }
    return buttons;
  }
  return String();
}

String relayState(int numRelay){
  if(RELAY_NO){
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "";
    }
    else {
      return "checked";
    }
  }
  else {
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "checked";
    }
    else {
      return "";
    }
  }
  return "";
}

void setup(){
  Serial.begin(115200);

  // Set semua relay menjadi mati ketika program dimulai - jika diatur ke Normally Open (NO), relay akan mati ketika Anda mengatur relay menjadi HIGH
  for(int i=1; i<=NUM_RELAYS; i++){
    pinMode(relayGPIOs[i-1], OUTPUT);
    if(RELAY_NO){
      digitalWrite(relayGPIOs[i-1], HIGH);
    }
    else{
      digitalWrite(relayGPIOs[i-1], LOW);
    }
  }
  
  // Terhubung ke Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi..");
  }

  // Cetak IP Address Lokal ESP8266
  Serial.println(WiFi.localIP());

  // Rute untuk halaman web root /
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Kirim permintaan GET ke <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET nilai input1 pada <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      if(RELAY_NO){
        Serial.print("NO ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], !inputMessage2.toInt());
      }
      else{
        Serial.print("NC ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], inputMessage2.toInt());
      }
    }
    else {
      inputMessage = "Tidak ada pesan yang dikirim";
      inputParam = "none";
    }
    Serial.println(inputMessage + inputMessage2);
    request->send(200, "text/plain", "OK");
  });
  // Mulai server
  server.begin();
}
  
void loop() {

}
