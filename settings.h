//----------------------------------------Interval Timer
unsigned long RetardoLectura;
//---------------------------------------GPRS SETTINGS


// Your GPRS credentials
// Leave empty, if missing user or pass
#define apn "broadband.tigo.gt"
#define user ""
#define pass ""
 
//---------------------------------------MQTT SETTINGS
const char* broker = "eospower.flatbox.io";

const char* publishTopic = "iot-2/evt/status/fmt/json/eospower";
const char* responseTopic ="iotdm-1/response/860719021114180/eospower";
const char* manageTopic = "iotdevice-1/mgmt/manage/eospower";
const char* updateTopic = "iotdm-1/device/update/eospower";
const char* rebootTopic = "iotdm-1/mgmt/initiate/device/reboot/eospower";


//---------------------------------------Variables de Reloj para espera y envio de paquetes de MQTT
unsigned long UInterval     = 1000UL; //Variable configurable remotamente sobre el interbalo de publicacion

//-------- Variables de ERROR EN ENVIO de paquetes de MQTT ANTES DE REINICIO
#define FAILTRESHOLD 150
