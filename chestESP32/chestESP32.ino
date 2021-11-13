//
//SENSOR PECHO
//
//  ______     ______     __   __     ______     ______     ______
///\  ___\   /\  ___\   /\ "-.\ \   /\  ___\   /\  __ \   /\  == \      
//  \ \___  \  \ \  __\   \ \ \-.  \  \ \___  \  \ \ \/\ \  \ \  __<
//   \/\_____\  \ \_____\  \ \_\\"\_\  \/\_____\  \ \_____\  \ \_\ \_\    
//    \/_____/   \/_____/   \/_/ \/_/   \/_____/   \/_____/   \/_/ /_/    V6
//

//
//
// CADENA DE SENSORES:

// (((SENSOR PULSO)))---\
// (((SENSOR PANZA)))--->(((RECEPTOR))--->[[[unity]]]
// (((SENSOR PECHO)))---/
//

/*
    This sketch connects via WiFI + websockets to a ESP32S AP and sends sensor data as UDP packets
    Alexandre Castonguay acastonguay@artengine.ca For Yesica Duarte. Thank you to EWMA library creator Arsen Torbarina.
*/

#include <Plaquette.h>
#include <Thresholder.h>

// The analog input.
AnalogIn in(34); // R1 202


// The serial output.
StreamOut serialOut;

AdaptiveNormalizer norm;

Thresholder peakDetector(0.7, THRESHOLD_RISING, 0.5);

//// WiFi UDP connection ////
#include <WiFi.h>
#include <WiFiUdp.h>
const char* ssid     = "ESPdatos";
const char* password = "respiracion";
IPAddress ip(192, 168, 4, 1);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
WiFiUDP Udp;
IPAddress myIP;                    // the IP address of your shield
//// Fin WiFi UDP connection ////

int datoL2 = 0; // Indicates state of breathing "1" or "2"

unsigned long initRutina;
unsigned long leTemps = 0;
unsigned long elViejoTiempo = 0;
unsigned long intervalleEntreResp = 0;
String calMessage = "CC, ";//CAL CHEST
String calibracion = "hello";

void begin() {

  Serial.begin(9600);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  myIP = WiFi.localIP();

  pinMode (LED_BUILTIN, OUTPUT); // LED del ESP8266

  for ( int i = 0 ; i < 5 ; i++ ) { //el led parpadea 5 veces para saludar!
    digitalWrite(LED_BUILTIN, 1 ) ;
    delay ( 100 ) ;
    digitalWrite (LED_BUILTIN, 0 ) ;
    delay ( 100 ) ;
  }

  digitalWrite (LED_BUILTIN, HIGH);//apago led por las dudas reverse para los ESP8266

  // Smooth over a window of 100ms (ie. one tenth of a second).
  // NOTE: Try changing the smoothing value to see how it affects the outputs.
  in.smooth(0.2);

  norm.time(10);

  initRutina = millis() + 15000; //1min 60MIL
}


void step() {

  in >> norm >> peakDetector;
  Serial.print(6 * norm);
  Serial.println(" ");
  Serial.print(6 * peakDetector);
  Serial.println(" ");

  if (millis() < initRutina) {//solo envia msg calibracion
    String calibracion = String (norm * 6);
    String sendCal = calMessage + calibracion;
    Udp.beginPacket(ip, 8888);
    Udp.print(sendCal);
    Udp.endPacket();
  }

  if (millis() > initRutina) {//envia msg sensado

    if (peakDetector == 1) {
      leTemps = millis();
      intervalleEntreResp = millis() - elViejoTiempo;

      datoL2 == 1; // peak detected
      digitalWrite (LED_BUILTIN, LOW);
      Udp.beginPacket(ip, 8888);
      Udp.print("chest,2");
      Udp.endPacket();

      Serial.println("peak detected, data one sent");
      Serial.print("intervalle entre respiration : ");
      Serial.println(intervalleEntreResp);

      elViejoTiempo = leTemps;

    }
    else if ( (millis() - elViejoTiempo) < intervalleEntreResp / 4) { // si nous sommes à l'intérieur du tiers de l'ijntervalle de respiration
      digitalWrite (LED_BUILTIN, LOW);
      Udp.beginPacket(ip, 8888);
      Udp.print("chest,2");
      Udp.endPacket();
      Serial.println("chest, 2");

    }
    else {
      datoL2 == 2; // nope
      digitalWrite (LED_BUILTIN, HIGH);
      Udp.beginPacket(ip, 8888);
      Udp.print("chest,1");
      Udp.endPacket();
      Serial.println("chest, 1");
    }
  }


} // fin step
