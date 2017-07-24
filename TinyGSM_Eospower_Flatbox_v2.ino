/**************************************************************
 *Verifivador de funcionamiento de Luces de semaforo
 **************************************************************/

// Modem SIM800:
#define TINY_GSM_MODEM_SIM800
//Librerias de funcionamiento:

#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>                                              //https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7
#include "settings.h"

// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

char imeicode[16];
//----------------------------------------------------------------------Variables de verificacion de fallas de capa de conexion con servicio
int failed, sent, published;
//----------------------------------------------------------------------Declaracion de Variables Globales (procuar que sean las minimas requeridas.
String ISO8601;                                                     //Variable para almacenar la marca del timepo (timestamp) de acuerdo al formtao ISO8601
//----------------------------------------------------------------------Variables Para casignacion de pines para los led RGB
int DeviceState = 0;
unsigned long lastNResetMillis;                                       //Variable para llevar conteo del tiempo desde la ultima publicacion 
int hora = 0;
const char* cLat = "14.598805";
const char* cLong = "-90.511142";
String Lat = "14.598805";
String Long = "-90.511142";
//----------------------------------------------------------------------Variables Para casignacion de pines para los led RGB
#define OUT_LED_PIN_RED   10                                                //Asignacion D10 Para el color Rojo del LeD RGB
#define OUT_LED_PIN_GREEN 9                                                 //Asignacion D9 Para el color Rojo del LeD RGB
#define OUT_LED_PIN_BLUE  8                                                 //Asignacion D8 Para el color Rojo del LeD RGB

#define IN_PIN_RED    4                                                        //Asignacion D4 Para el color Rojo del LeD RGB
#define IN_PIN_GREEN  5                                                       //Asignacion D5 Para el color Rojo del LeD RGB
#define IN_PIN_YELLOW 6                                                       //Asignacion D6 Para el color Rojo del LeD RGB

String LightColor = "";
String OldLightColor = "";

int redledStatus = LOW;
int greenledStatus = LOW;
int blueledStatus = LOW;

boolean IN_RED = false;
boolean IN_GREEN = false;
boolean IN_YELLOW = false;

//----------------------------------------------------------------------denifinir el parpadeo de coloers del led RGB ---- Puertos D8 (rojo) d7 (verde) D6 (Azul)
void BlueLight(){
  digitalWrite(OUT_LED_PIN_RED, LOW);
  digitalWrite(OUT_LED_PIN_GREEN, LOW);
  digitalWrite(OUT_LED_PIN_BLUE, HIGH);
}

void RedLight(){
  digitalWrite(OUT_LED_PIN_RED, HIGH);
  digitalWrite(OUT_LED_PIN_GREEN, LOW);
  digitalWrite(OUT_LED_PIN_BLUE, LOW);  
}

void GreenLight(){
  digitalWrite(OUT_LED_PIN_RED, LOW);
  digitalWrite(OUT_LED_PIN_GREEN, HIGH);
  digitalWrite(OUT_LED_PIN_BLUE, LOW);
}

void PurpleLight(){
  digitalWrite(OUT_LED_PIN_RED, HIGH);
  digitalWrite(OUT_LED_PIN_GREEN, LOW);
  digitalWrite(OUT_LED_PIN_BLUE, HIGH);
}

void WhiteLight(){
  digitalWrite(OUT_LED_PIN_RED, HIGH);
  digitalWrite(OUT_LED_PIN_GREEN, HIGH);
  digitalWrite(OUT_LED_PIN_BLUE, HIGH);
}

void lightsOff(){
  digitalWrite(OUT_LED_PIN_RED, LOW);
  digitalWrite(OUT_LED_PIN_GREEN, LOW);
  digitalWrite(OUT_LED_PIN_BLUE, LOW);
}

long lastReconnectAttempt = 0;

//-----------------------------------------------------------------------------Setup function

void setup() {
  pinMode(OUT_LED_PIN_RED, OUTPUT);
  pinMode(OUT_LED_PIN_GREEN, OUTPUT);
  pinMode(OUT_LED_PIN_BLUE, OUTPUT);

  pinMode(IN_PIN_RED, INPUT);
  pinMode(IN_PIN_GREEN, INPUT);
  pinMode(IN_PIN_YELLOW, INPUT);

  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(3000);
  
  // light on indicates init
  PurpleLight();
  
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();

  modem.getIMEI();

  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    while (true);
  }
  Serial.println(" OK");

  Serial.print("Connecting to ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println(" fail");
    while (true);
  }
  Serial.println(" OK");
  
  //Modem on om light blue
  BlueLight();

  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);

  while (!mqtt.connect(imeicode,"flatbox","FBx_admin2012")) {
    Serial.println("connect failed");
    delay(1000);
  }
  WhiteLight();
  Serial.println("connected");
  mqtt.publish("iotdevice-1/mgmt/manage/eospower","Iniciando Prueba de Semaforo");
  mqtt.subscribe("iotdm-1/device/update/eospower");
  lightsOff();  
}

//--------------------------------------------------------------------------------------Reconnect Fucntion
boolean remqttConnect() {
  Serial.print("Connecting to ");
  Serial.print(broker);
  if (!mqtt.connect(imeicode,"flatbox","FBx_admin2012")) {
    Serial.println(" fail");
    return false;
  }
  Serial.println(" OK");
  mqtt.publish(manageTopic, "GsmClientTest started");
  mqtt.subscribe(responseTopic);
  return mqtt.connected();
}
//---------------------------------------------------------------- Declaracion de sensor de luz

boolean GetRedStatus(){
   boolean RedStatus;
   if(IN_PIN_RED == HIGH){
    RedStatus = true;
    LightColor = "RED";
    RedLight();
   }else{
    RedStatus = false;
   }
   return RedStatus;   
}

boolean GetGreenStaus(){
   boolean GreenStaus;
   if(IN_PIN_GREEN == HIGH){
    GreenStaus = true;
    LightColor = "Green";
    GreenLight();
   }else{
    GreenStaus = false;
   }
    return GreenStaus;
}

boolean GetYellowStaus(){
   boolean YellowStaus;
   if(IN_PIN_YELLOW == HIGH){
    YellowStaus = true;
    LightColor = "Yellow";
    WhiteLight();
    }else{
    YellowStaus = false;
   }
   return YellowStaus;   
}

//--------------------------------------------------------------------------------------------funcion de enviode Datos
void publishLightColor(String IDModulo, String LColor,String Latitude,String Longitude, String Tstamp) {
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& d = root.createNestedObject("d");
  JsonObject& Lightdata = d.createNestedObject("Lightdata");
  Lightdata["IMEI"] = IDModulo;
  Lightdata["Semaforo"] = LColor;
  Lightdata["Lat"] = Latitude;
  Lightdata["Long"] = Longitude;
  Lightdata["Tstamp"] = Tstamp;
  char MqttLightdata[500];
  root.printTo(MqttLightdata, sizeof(MqttLightdata));
  Serial.println(F("publishing device publishTopic metadata:")); 
  Serial.println(MqttLightdata);
  sent ++;
  if (mqtt.publish(publishTopic, MqttLightdata)){
    Serial.println(F("enviado data de semaforo: OK"));
    published ++;
    failed = 0; 
  }else {
    Serial.println(F("enviado data de semaforo: FAILED"));
    failed ++;
  }
}
//-------------------------------------------------------------------------------Rset function after 24h
void NormalReset(){
  if (millis()- lastNResetMillis > 60 * 60 * UInterval){
    hora++;
    if (hora > 24){
      String msg = ("24h NReset");  
      String Msg = ( imeicode + msg + "@:" + ISO8601);
      mqtt.publish("iotdevice-1/mgmt/manage/eospower","Mensaje de control Reser24N");
      Serial.println(Msg);
      void disconnect ();
      hora = 0;
      asm volatile ("  jmp 0");
    }
     lastNResetMillis = millis(); //Actulizar la ultima hora de envio
  }
}
//------------------------------------------------------------------------------------------Loop Funciton
void loop() {

  IN_RED = GetRedStatus();
  IN_GREEN = GetGreenStaus();
  IN_YELLOW = GetYellowStaus();
  
  if ( (IN_RED == 0) && (IN_GREEN == 0) && (IN_YELLOW == 0) ){
    LightColor = "OFF";
    lightsOff();
  }
  if (LightColor != OldLightColor){
    OldLightColor = LightColor;
    Serial.print(F("Light is: "));
    Serial.println(LightColor);
    publishLightColor(imeicode, LightColor,cLat,cLong,ISO8601);  // publishRF_Boton(String IDModulo, String EventID, String Tstamp)
  }
  
  NormalReset();
 
  if ( millis() - RetardoLectura > 30*60* UInterval){
  mqtt.publish("iotdevice-1/mgmt/manage/eospower","Mensaje de control");
  RetardoLectura = millis(); //Actulizar la ultima hora de envio
 }

 // VERIFICAMOS CUANTAS VECES NO SE HAN ENVIOADO PAQUETES (ERRORES)
   if (failed >= FAILTRESHOLD){
    failed =0;
    published =0;
    sent=0;    
    asm volatile ("  jmp 0");
  }
  
  
  if (mqtt.connected()) {
    mqtt.loop();
  } else {
    // Reconnect every 10 seconds
    unsigned long t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (remqttConnect()) {
        lastReconnectAttempt = 0;
      }
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print(F("callback invoked for topic: "));                    //Imprimir un mensaje señalando sobre que topico se recibio un mensaje
  Serial.println(topic);                                              //Imprimir el Topico

  if (strcmp (responseTopic, topic) == 0) {                            //verificar si el topico conicide con el Topico responseTopic[] definido en el archivo settings.h local
    //handleResponse(payload);
    mqtt.publish(manageTopic, "light changed");
    //return; // just print of response for now                         //Hacer algo si conicide (o en este caso hacer nada)
  }
  
  if (strcmp (rebootTopic, topic) == 0) {                             //verificar si el topico conicide con el Topico rebootTopic[] definido en el archivo settings.h local
    Serial.println(F("Rebooting..."));                                //imprimir mensaje de Aviso sobre reinicio remoto de unidad.
    asm volatile ("  jmp 0");                                                   //Emitir comando de reinicio para ESP8266
  }
  
  if (strcmp (updateTopic, topic) == 0) {                             //verificar si el topico conicide con el Topico updateTopic[] definido en el archivo settings.h local
    //handleUpdate(payload);                                            //enviar a la funcion handleUpdate el contenido del mensaje para su parseo.
  }
}

