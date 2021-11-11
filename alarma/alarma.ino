#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IotWebConf.h>
#include <IotWebConfUsing.h>
#include <ArduinoJson.h>
#include <dscKeybusInterface.h>


unsigned int localPort = 8888;
String deviceID = "DSC010000000002"; 
char char_deviceID[18]="DSC010000000002";
const char thingName[] = "coiaca-DSC010000000002";  
const char wifiInitialApPassword[] = "12345678"; 
String defRemoteConfigMqttPwdValue ="rZ25xbVN";  //dsc03 "3XrTOdcc";

#define STRING_LEN 64
#define NUMBER_LEN 8
#define CONFIG_VERSION "DSC_v1.0.0"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
#define CONFIG_PIN 12
#define STATUS_PIN 0   

// Configures the Keybus interface with the specified pins - dscWritePin is optional, leaving it out disables the virtual keypad.
#define dscClockPin 5  // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
#define dscReadPin 4   // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
#define dscWritePin 15  // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
dscKeybusInterface dsc(dscClockPin, dscReadPin, dscWritePin);

// Servicios
DNSServer dnsServer;
WebServer server(80);
WiFiClient net;
WiFiClientSecure net2;
PubSubClient mqttClient;


// -- Callback method declarations.
void wifiConnected();
void configSaved();
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);
void mqttMessageReceived(char* topic, byte* payload, unsigned int length);
void readParamValue(const char* paramName, char* target, unsigned int len);
const int NTP_PACKET_SIZE = 48; 
byte packetBuffer[NTP_PACKET_SIZE]; 

// variables mias
unsigned long lastPublish;
int timerMillis;
int timerEventDone[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
bool DSCBegined = false;
String lastSentStatus;
int activePartition;

boolean needMqttConnect = false;
boolean needReset = false;
unsigned long lastReport = 0;
unsigned long lastMqttConnectionAttempt = 0;

char mqttServerValue[STRING_LEN];
char mqttPortValue[NUMBER_LEN];
char mqttClientIDValue[STRING_LEN];
char mqttUserNameValue[STRING_LEN];
char mqttUserPasswordValue[STRING_LEN];
char accessCodeValue[NUMBER_LEN]; 
char mqttStatusTopicValue[STRING_LEN];
char mqttBirthMessageValue[STRING_LEN];
char mqttLwtMessageValue[STRING_LEN];
char mqttNoDSCValue[STRING_LEN];
char mqttPartitionTopicValue[STRING_LEN]; // Sends armed and alarm status per partition: dsc/Get/Partition1 ... dsc/Get/Partition8
char mqttActivePartitionTopicValue[STRING_LEN];
char mqttZoneTopicValue[STRING_LEN]; // Sends zone status per zone: dsc/Get/Zone1 ... dsc/Get/Zone64
char mqttFireTopicValue[STRING_LEN]; // Sends fire status per partition: dsc/Get/Fire1 ... dsc/Get/Fire8
char mqttTroubleTopicValue[STRING_LEN]; // Sends trouble status
char mqttCommandTopicValue[STRING_LEN]; // Receives messages to write to the panel
char mqttKeepAliveTopicValue[STRING_LEN]; // Topico para enviar el keepAlive cada time interval
char updateIntervalValue[NUMBER_LEN];
char tiMerStatusValue[NUMBER_LEN];
char publishTimerStringValue[NUMBER_LEN];
char ntpServerValue[STRING_LEN];
char timeZoneValue[NUMBER_LEN];
char ntpUpdateIntervalValue[NUMBER_LEN];
char timeDSTValue[NUMBER_LEN];
char mqttRetainValue[NUMBER_LEN];
char enableRConfigValue[NUMBER_LEN];
char remoteConfigTopicValue[STRING_LEN];
char remoteConfigResultTopicValue[STRING_LEN];
char enableMonitoringValue[NUMBER_LEN];
char monitoringTopicValue[STRING_LEN];
char enableMqttDebugValue[NUMBER_LEN];
char MqttDebugTopicValue[STRING_LEN];
char isSecureConectionValue[NUMBER_LEN];



IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

const char CUSTOMHTML_SCRIPT_INNER[] PROGMEM = "document.addEventListener(\"DOMContentLoaded\",function(e){let t=document.querySelectorAll('input[type=\"password\"]');for(let e of t){let t=document.createElement(\"INPUT\");t.type=\"button\",t.value=\"??\",t.style.width=\"auto\",e.style.width=\"83%\",e.parentNode.insertBefore(t,e.nextSibling),t.onclick=function(){\"password\"===e.type?(e.type=\"text\",t.value=\"??\"):(e.type=\"password\",t.value=\"??\")}}var s=document.querySelectorAll('input[type=\"custom_select\"]');if(s.length)for(var o=0;o<s.length;++o){var n=s[o].value;for(var l in optionsList=s[o].getAttribute(\"data-options\").split(\"|\"),selectTag=['<select name=\"',s[o].name,'\" id=\"',s[o].id,'\">'],\"\"===n&&selectTag.push('<option value=\"\"></option>'),optionsList)selectTag.push('<option value=\"'),selectTag.push(l),selectTag.push('\"'),l==n&&selectTag.push(\" selected\"),selectTag.push(\">\"),selectTag.push(optionsList[l]),selectTag.push(\"</option>\");selectTag.push(\"</select>\");var a=s[o].parentNode,p=document.createElement(\"span\");p.innerHTML=selectTag.join(\"\"),a.removeChild(s[o]),a.appendChild(p)}});";
const char CUSTOMHTML_BODY_INNER[] PROGMEM = "<div><img src='data:image/png;base64,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'/></div>\n";

static char isSecureConectionVal[][STRING_LEN] = { "0", "1"};
static char isSecureConectionNam[][STRING_LEN] = { "No", "Yes"};
static char mqttRetVal[][STRING_LEN] = { "0", "1"};
static char mqttRetNam[][STRING_LEN] = { "0", "1"};
IotWebConfParameterGroup group1 =  IotWebConfParameterGroup("group1", "MQTT Config");
IotWebConfTextParameter mqttServerParam = IotWebConfTextParameter("MQTT Server", "mqttServer", mqttServerValue, STRING_LEN, "mqtt.coiaca.com", nullptr, "mqtt.coiaca.com");
IotWebConfNumberParameter mqttPortParam = IotWebConfNumberParameter("MQTT server port (unsecure)", "MQTTPort", mqttPortValue, NUMBER_LEN, "1883", "1..9999", "min='1' max='9999' step='1'");
IotWebConfSelectParameter isSecureConectionParam = IotWebConfSelectParameter("Is Secure Port", "Is Secure Port", isSecureConectionValue, STRING_LEN, (char*)isSecureConectionVal, (char*)isSecureConectionNam, sizeof(isSecureConectionNam) / STRING_LEN, STRING_LEN);
IotWebConfTextParameter mqttUserNameParam = IotWebConfTextParameter("MQTT user", "mqttUser", mqttUserNameValue, STRING_LEN, "mqttusr", nullptr, "mqttusr");
IotWebConfPasswordParameter mqttUserPasswordParam = IotWebConfPasswordParameter("MQTT password", "mqttPass", mqttUserPasswordValue, STRING_LEN, "mqttpwd", nullptr, "mqttpwd");
IotWebConfTextParameter mqttClientIDParam = IotWebConfTextParameter("MQTT Client ID", "mqttClientID", mqttClientIDValue, STRING_LEN,"CoiacaDSC010000000002", nullptr, "CoiacaDSC010000000002");
IotWebConfSelectParameter mqttRetainParam = IotWebConfSelectParameter("MQTT Retain", "mqttRetain", mqttRetainValue, STRING_LEN, (char*)mqttRetVal, (char*)mqttRetNam,sizeof(mqttRetVal) / STRING_LEN, STRING_LEN);

IotWebConfParameterGroup group2 =  IotWebConfParameterGroup("group2", "Alarm Config");
IotWebConfTextParameter accessCodeParam = IotWebConfTextParameter("Access Code", "accessCode", accessCodeValue, NUMBER_LEN,"1234", "1..9999", "min='0' max='9999' step='1'");

IotWebConfParameterGroup group3 =  IotWebConfParameterGroup("group3", "Mqtt Advanced");
IotWebConfTextParameter mqttStatusTopicParam = IotWebConfTextParameter("Status Topic", "mqttStatusTopic", mqttStatusTopicValue, STRING_LEN, "DSC010000000002/Status", nullptr,"DSC010000000002/Status");
IotWebConfTextParameter mqttBirthMessageParam = IotWebConfTextParameter("Birth Message", "mqttBirthMessage", mqttBirthMessageValue, STRING_LEN, "online", nullptr, "online");
IotWebConfTextParameter mqttLwtMessageParam = IotWebConfTextParameter("LWT Message", "mqttLwtMessage", mqttLwtMessageValue, STRING_LEN, "offline", nullptr, "offline");
IotWebConfTextParameter mqttNoDSCParam = IotWebConfTextParameter("Disconnected Message", "mqttNoDSC", mqttNoDSCValue, STRING_LEN, "Alarm Disconnected", nullptr, "Alarm Disconnected");
IotWebConfTextParameter mqttPartitionTopicParam = IotWebConfTextParameter("Partition Topic Prefix", "mqttPartitionTopic", mqttPartitionTopicValue, STRING_LEN, "DSC010000000002/Partition", nullptr, "DSC010000000002/Partition");
IotWebConfTextParameter mqttActivePartitionTopicParam = IotWebConfTextParameter("Active Partition Topic", "mqttActivePartitionTopic", mqttActivePartitionTopicValue, STRING_LEN, "DSC010000000002/activePartition", nullptr, "DSC010000000002/activePartition");
IotWebConfTextParameter mqttZoneTopicParam = IotWebConfTextParameter("Zone Topic Prefix", "mqttZoneTopic", mqttZoneTopicValue, STRING_LEN, "DSC010000000002/Zone", nullptr, "DSC010000000002/Zone");
IotWebConfTextParameter mqttFireTopicParam = IotWebConfTextParameter("Fire Topic Prefix", "mqttFireTopic", mqttFireTopicValue, STRING_LEN, "DSC010000000002/Fire", nullptr, "DSC010000000002/Fire");
IotWebConfTextParameter mqttTroubleTopicParam = IotWebConfTextParameter("Trouble Topic", "mqttTroubleTopic", mqttTroubleTopicValue, STRING_LEN, "DSC010000000002/Trouble", nullptr, "DSC010000000002/Trouble");
IotWebConfTextParameter mqttCommandTopicParam = IotWebConfTextParameter("Commands Topic", "mqttCommandTopic", mqttCommandTopicValue, STRING_LEN, "DSC010000000002/cmd", nullptr, "DSC010000000002/cmd");
IotWebConfTextParameter mqttKeepAliveTopicParam = IotWebConfTextParameter("Keep Alive Topic", "mqttKeepAliveTopic", mqttKeepAliveTopicValue, STRING_LEN, "DSC010000000002/keepAlive", nullptr, "DSC010000000002/keepAlive");
IotWebConfNumberParameter updateIntervalParam = IotWebConfNumberParameter("Keep Alive interval (seconds)", "updateInterval", updateIntervalValue, NUMBER_LEN, "30", "1..100", "min='1' max='100' step='1'");

static char enableMonitoringPVal[][STRING_LEN] = { "0", "1","2","3"};
static char enableMonitoringPNam[][STRING_LEN] = {"Disabled","Basic","Armed","All"};
static char enableMqttDebugPVal[][STRING_LEN] = { "0", "1"};
static char enableMqttDebugPNam[][STRING_LEN] = { "No", "Yes"};
IotWebConfParameterGroup group4 =  IotWebConfParameterGroup("group4", "Monitoring Configuration");
IotWebConfSelectParameter enableMonitoringParam = IotWebConfSelectParameter("Enable Monitoring", "enableMonitoring", enableMonitoringValue, STRING_LEN, (char*)enableMonitoringPVal, (char*)enableMonitoringPNam, sizeof(enableMonitoringPVal) / STRING_LEN, STRING_LEN);
IotWebConfTextParameter monitoringTopicParam = IotWebConfTextParameter("Monitoring Topic Prefix", "monitoringTopic", monitoringTopicValue, STRING_LEN, "MNTR/DSC010000000002", nullptr, "MNTR/DSC010000000002");
IotWebConfSelectParameter enableMqttDebugParam = IotWebConfSelectParameter("Enable MQTT Debug", "enableMqttDebug", enableMqttDebugValue, STRING_LEN,(char*)enableMqttDebugPVal, (char*)enableMqttDebugPNam, sizeof(enableMqttDebugPVal) / STRING_LEN, STRING_LEN);
IotWebConfTextParameter MqttDebugTopicParam = IotWebConfTextParameter("MQTT Debug Topic", "MqttDebugTopic", MqttDebugTopicValue, STRING_LEN, "RMgmt/DSC010000000002/debug", nullptr, "RMgmt/DSC010000000002/debug");


void setup() {
  /*
    dsc.hideKeypadDigits = false;      // Controls if keypad digits are hidden for publicly posted logs (default: false)
    dsc.processRedundantData = false;  // Controls if repeated periodic commands are processed and displayed (default: true)
    dsc.processModuleData = true;      // Controls if keypad and module data is processed and displayed (default: false)
    dsc.displayTrailingBits = false;   // Controls if bits read as the clock is reset are displayed, appears to be spurious data (default: false)
  */

  //ESP.wdtDisable();
  pinMode(13, FUNCTION_3);
  pinMode(14, FUNCTION_3);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
 
  Serial.begin(115200);
  Serial.println("Starting up...");

  group1.addItem(&mqttServerParam);
  group1.addItem(&mqttPortParam);
  group1.addItem(&isSecureConectionParam);
  group1.addItem(&mqttUserNameParam);
  group1.addItem(&mqttUserPasswordParam);
  group1.addItem(&mqttClientIDParam);
  group1.addItem(&mqttRetainParam);

  group2.addItem(&accessCodeParam);

  group3.addItem(&mqttStatusTopicParam);
  group3.addItem(&mqttBirthMessageParam);
  group3.addItem(&mqttLwtMessageParam);
  group3.addItem(&mqttNoDSCParam);
  group3.addItem(&mqttPartitionTopicParam);
  group3.addItem(&mqttActivePartitionTopicParam);
  group3.addItem(&mqttZoneTopicParam);
  group3.addItem(&mqttFireTopicParam);
  group3.addItem(&mqttTroubleTopicParam);
  group3.addItem(&mqttCommandTopicParam);
  group3.addItem(&mqttKeepAliveTopicParam);
  group3.addItem(&updateIntervalParam);
  
  group4.addItem(&enableMonitoringParam);
  group4.addItem(&monitoringTopicParam);
  group4.addItem(&enableMqttDebugParam);
  group4.addItem(&MqttDebugTopicParam);

  iotWebConf.setStatusPin (STATUS_PIN); 
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.addParameterGroup(&group1);
  iotWebConf.addParameterGroup(&group2);
  iotWebConf.addParameterGroup(&group3);
  iotWebConf.addParameterGroup(&group4);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.setWifiConnectionCallback(&wifiConnected);
  iotWebConf.setConfigSavedCallback(&configSaved);


  // -- Initializing the configuration.
  boolean validConfig = iotWebConf.init();
  Serial.println("Valid config del la WEB: " + (String) validConfig);
  /**if (!validConfig)
  {
    mqttServerValue[0] = '\0';
    mqttUserNameValue[0] = '\0';
    mqttUserPasswordValue[0] = '\0';
  }
**/
  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  // Conexion mqtt para la funcionalidad principal
  if (atoi(isSecureConectionValue) != 1) {
      Serial.println("Conexion inicial en el setup mqtt");
      Serial.println("mqttServerValue: " + (String) mqttServerValue);
      Serial.println("mqttPortValue: " + (String) mqttPortValue);
      mqttClient.setServer(mqttServerValue, atoi(mqttPortValue));
      mqttClient.setClient(net);
      mqttClient.setCallback(mqttMessageReceived);
  }

  // Conexion mqtt para la Configuracion remota
  if (atoi(isSecureConectionValue) == 1)  {
    Serial.println("******************* ES CONEXION SEGURA"); 
    mqttClient.setServer(mqttServerValue, atoi(mqttPortValue));
    mqttClient.setClient(net2);
    mqttClient.setCallback(mqttMessageReceived);
  }
}



void loop() {
  iotWebConf.doLoop();
  mqttClient.loop();
  dsc.loop();

  // Conexion funcionalidad principal 
  if (needMqttConnect){
    if (connectMqtt()){
      needMqttConnect = false;
    }
  }
  else if ((iotWebConf.getState() == iotwebconf::OnLine) && (!mqttClient.connected())){
    // --------------- SerialDebug: ---------
    Serial.println("============================== MQTT reconnect");
    // --------------- mqttDebug: --------- 
    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext=deviceID + " - MQTT reconnect";
      mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(),(bool) atoi(mqttRetainValue));
    }
    connectMqtt();
  }

  // Restart
  if (needReset){
    // --------------- SerialDebug: --------- 
    Serial.println("Rebooting after 1 second.");
    // --------------- SerialDebug: --------- 
    // --------------- mqttDebug: --------- 
    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext=deviceID + " - Rebooting after 1 second..." ;
      mqttClient.publish(MqttDebugTopicValue,msgtext.c_str() , (bool) atoi(mqttRetainValue));}
    // --------------- mqttDebug: --------- 
    
    iotWebConf.delay(1000);
    ESP.restart();
  }

  if (iotWebConf.getState() == iotwebconf::OnLine) {
    if ( atoi(updateIntervalValue) > 0 && millis() - lastPublish > (atoi(updateIntervalValue) * 1000) ) { 
      Serial.println(" - Keep Alive interval elapsed. Automatically publishing...");
      if (atoi(enableMqttDebugValue) == 1) {
        String msgtext=deviceID + " - Keep Alive interval elapsed. Automatically publishing...";
        mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
      }
      publicaEstados();
      lastPublish = millis();
    }
  }
  else{
    if (DSCBegined){
      dsc.stop();
      DSCBegined = false;
      Serial.println(F("DSC Keybus Interface stoped."));
      // ESTO NO PORQU NO ESTA CONECTADO: if (atoi(enableMqttDebugValue) == 1) {mqttClient.publish(MqttDebugTopicValue, deviceID + " DSC Keybus Interface stoped.", (bool) atoi(mqttRetainValue));}
    }
  }

  if (DSCBegined){
    doDSC();
    if (activePartition != dsc.writePartition){
      activePartition = dsc.writePartition;
      mqttClient.publish(mqttActivePartitionTopicValue, String(activePartition).c_str(), true);
    }      
  }
}


/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  if (iotWebConf.handleCaptivePortal())
  {
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>Coiaca: IoT Device Configuration</title></head><body><center>";
  s += FPSTR(CUSTOMHTML_BODY_INNER);
  s += "<br /><br />Device ID: <b>";
  s += deviceID;
  s += "</b><br /><br />";
  s += "<a href='config'>Config</a><br />";
  s += "</center></body></html>\n";

  server.send(200, "text/html; charset=UTF-8", s);
}

void wifiConnected(){
  Serial.println("============================ WIFI conectado");
  needMqttConnect = true;
  dsc.begin();
  DSCBegined = true;
  Serial.println("DSC Keybus Interface begined.");
  activePartition = dsc.writePartition;
  mqttClient.publish(mqttActivePartitionTopicValue,String(activePartition).c_str(), true); 
}

void configSaved(){
  Serial.println("Configuration updated.");
  if (atoi(enableMqttDebugValue) == 1) {
    String msgtext=deviceID + " - Configuration updated.";
    mqttClient.publish(MqttDebugTopicValue, msgtext.c_str(), (bool) atoi(mqttRetainValue));
  }
  needReset = true;
}

bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper){
  Serial.println("Validating form.");
  bool valid = true;
  /*
    int l = webRequestWrapper->arg(stringParam.getId()).length();
    if (l < 3)
    {
      stringParam.errorMessage = "Please provide at least 3 characters for this test!";
      valid = false;
    }
  */
  return valid;
}