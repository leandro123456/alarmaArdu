/*
 * 
 * Interface para sistemas de alarma DSC Power Series
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
# include <ESP8266HTTPUpdateServer.h>
//#include <ESP8266HTTPUpdateServer.h>
//#include <MQTT.h>
#include <MQTTClient.h>
#include <IotWebConf.h>
#include <IotWebConfUsing.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <dscKeybusInterface.h>
//#include <coiaca.h>

//NTP
#include <TimeLib.h>
#include <WiFiUdp.h>
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

////// CUSTOM DATA /////////////
String deviceID = "DSC010000000002"; // >>REPLACE_FOR_PERSO<<
char char_deviceID[18]="DSC010000000002";
// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "coiaca-DSC010000000002";  // >>REPLACE_FOR_PERSO<<

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "12345678"; // >>REPLACE_FOR_PERSO<<

// Default Remote Management password and Coiaca MQTT broker pwd  
String defRConfigPwdValue = "wDJGtmE6"; // >>REPLACE_FOR_PERSO<<
String defRemoteConfigMqttPwdValue = "3XrTOdcc"; // >>REPLACE_FOR_PERSO<<
char* mqttRCClientIDValue = "DSC010000000002RM"; //Para diferenciar el clientID entre el de la func ppal y el e RC  >>REPLACE_FOR_PERSO<<
////// FIN CUSTOM DATA /////////////

#define STRING_LEN 128
#define NUMBER_LEN 8

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "DSC_v0.7.0"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN 12
#define STATUS_PIN 0   

// Configures the Keybus interface with the specified pins - dscWritePin is optional, leaving it out disables the virtual keypad.
#define dscClockPin 5  // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
#define dscReadPin 4   // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
#define dscWritePin 15  // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
dscKeybusInterface dsc(dscClockPin, dscReadPin, dscWritePin);

// Servicios
DNSServer dnsServer;
ESP8266WebServer server(80);
//ESP8266HTTPUpdateServer httpUpdater;
WiFiClient net;
WiFiClientSecure netRConf;
MQTTClient mqttClient;
MQTTClient mqttClientRConf(256);

// -- Callback method declarations.
void wifiConnected();
void configSaved();
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);
void mqttMessageReceived(String &topic, String &payload);
void mqttRConfMessageReceived(String &topic, String &payload);
void readParamValue(const char* paramName, char* target, unsigned int len);
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// variables mias
unsigned long lastPublish;
int timerMillis;
int timerEventDone[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
bool DSCBegined = false;
String lastSentStatus;
int activePartition;

boolean needMqttConnect = false;
boolean needRConfMqttConnect = false;
boolean needReset = false;
unsigned long lastReport = 0;
unsigned long lastMqttConnectionAttempt = 0;
unsigned long lastRConfMqttConnectionAttempt = 0;
// FIN variables mias

// Custom params values
char deviceIdValue[STRING_LEN];
char mqttServerValue[STRING_LEN];
char mqttPortValue[NUMBER_LEN];
char mqttClientIDValue[STRING_LEN];
char mqttUserNameValue[STRING_LEN];
char mqttUserPasswordValue[STRING_LEN];
char accessCodeValue[NUMBER_LEN]; // An access code is required to disarm/night arm and may be required to arm based on panel configuration.
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
char tiMerStringValue[STRING_LEN];
char publishTimerStringValue[NUMBER_LEN];
char ntpServerValue[STRING_LEN];
char timeZoneValue[NUMBER_LEN];
char ntpUpdateIntervalValue[NUMBER_LEN];
char timeDSTValue[NUMBER_LEN];
char mqttRetainValue[NUMBER_LEN];
char mqttQoSValue[NUMBER_LEN];
char enableRConfigValue[NUMBER_LEN];
char RConfigPwdValue[STRING_LEN];
char remoteConfigMqttServerValue[STRING_LEN];
char remoteConfigMqttPortValue[NUMBER_LEN];
char remoteConfigMqttUserValue[STRING_LEN];
char remoteConfigMqttPwdValue[STRING_LEN];
char remoteConfigTopicValue[STRING_LEN];
char remoteConfigResultTopicValue[STRING_LEN];
char enableMonitoringValue[NUMBER_LEN];
char monitoringTopicValue[STRING_LEN];
char forceSecureAllTrafficValue[NUMBER_LEN];
char remoteConfigRetainValue[NUMBER_LEN];
char remoteConfigQoSValue[NUMBER_LEN];
char enableMqttDebugValue[NUMBER_LEN];
char MqttDebugTopicValue[STRING_LEN];




IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);


// Para customizar HTML
// -- Javascript block will be added to the header.
//const char CUSTOMHTML_SCRIPT_INNER[] PROGMEM = "document.addEventListener(\"DOMContentLoaded\",function(e){let t=document.querySelectorAll('input[type=\"password\"]');for(let e of t){let t=document.createElement(\"INPUT\");t.type=\"button\",t.value=\"??\",t.style.width=\"auto\",e.style.width=\"83%\",e.parentNode.insertBefore(t,e.nextSibling),t.onclick=function(){\"password\"===e.type?(e.type=\"text\",t.value=\"??\"):(e.type=\"password\",t.value=\"??\")}}var s=document.querySelectorAll('input[type=\"custom_select\"]');if(s.length)for(var n=0;n<s.length;++n){var l=s[n].value;for(var o in optionsList=s[n].getAttribute(\"data-options\").split(\"|\"),selectTag=['<select name=\"',s[n].name,'\" id=\"',s[n].id,'\">'],optionsList)selectTag.push('<option value=\"'),selectTag.push(o),selectTag.push('\"'),o==l&&selectTag.push(\" selected\"),selectTag.push(\">\"),selectTag.push(optionsList[o]),selectTag.push(\"</option>\");selectTag.push(\"</select>\");var a=s[n].parentNode,p=document.createElement(\"span\");p.innerHTML=selectTag.join(\"\"),a.removeChild(s[n]),a.appendChild(p)}});";
const char CUSTOMHTML_SCRIPT_INNER[] PROGMEM = "document.addEventListener(\"DOMContentLoaded\",function(e){let t=document.querySelectorAll('input[type=\"password\"]');for(let e of t){let t=document.createElement(\"INPUT\");t.type=\"button\",t.value=\"??\",t.style.width=\"auto\",e.style.width=\"83%\",e.parentNode.insertBefore(t,e.nextSibling),t.onclick=function(){\"password\"===e.type?(e.type=\"text\",t.value=\"??\"):(e.type=\"password\",t.value=\"??\")}}var s=document.querySelectorAll('input[type=\"custom_select\"]');if(s.length)for(var o=0;o<s.length;++o){var n=s[o].value;for(var l in optionsList=s[o].getAttribute(\"data-options\").split(\"|\"),selectTag=['<select name=\"',s[o].name,'\" id=\"',s[o].id,'\">'],\"\"===n&&selectTag.push('<option value=\"\"></option>'),optionsList)selectTag.push('<option value=\"'),selectTag.push(l),selectTag.push('\"'),l==n&&selectTag.push(\" selected\"),selectTag.push(\">\"),selectTag.push(optionsList[l]),selectTag.push(\"</option>\");selectTag.push(\"</select>\");var a=s[o].parentNode,p=document.createElement(\"span\");p.innerHTML=selectTag.join(\"\"),a.removeChild(s[o]),a.appendChild(p)}});";
// -- HTML element will be added inside the body element.
const char CUSTOMHTML_BODY_INNER[] PROGMEM = "<div><img src='data:image/png;base64,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'/></div>\n";

// -- This is an OOP technique to override behaviour of the existing
// IotWebConfHtmlFormatProvider. Here two method are overriden from
// the original class. See IotWebConf.h for all potentially overridable
// methods of IotWebConfHtmlFormatProvider .

// FIN Para customizar HTML

IotWebConfParameterGroup group1 =  IotWebConfParameterGroup("group1", "");
IotWebConfTextParameter deviceIDParam = IotWebConfTextParameter("Device ID12", "deviceId", deviceIdValue, STRING_LEN, char_deviceID, nullptr, char_deviceID);

IotWebConfParameterGroup group2 =  IotWebConfParameterGroup("group2", "MQTT Config");
IotWebConfTextParameter mqttServerParam = IotWebConfTextParameter("MQTT Server", "mqttServer", mqttServerValue, STRING_LEN, "mqtt.coiaca.com", nullptr, "mqtt.coiaca.com");
IotWebConfNumberParameter mqttPortParam = IotWebConfNumberParameter("MQTT server port (unsecure)", "MQTTPort", mqttPortValue, NUMBER_LEN, "1883", "1..9999", "min='1' max='9999' step='1'");
IotWebConfTextParameter mqttUserNameParam = IotWebConfTextParameter("MQTT user", "mqttUser", mqttUserNameValue, STRING_LEN, "mqttusr", nullptr, "mqttusr");
IotWebConfPasswordParameter mqttUserPasswordParam = IotWebConfPasswordParameter("MQTT password", "mqttPass", mqttUserPasswordValue, STRING_LEN, "mqttpwd", nullptr, "mqttpwd");
IotWebConfTextParameter mqttClientIDParam = IotWebConfTextParameter("MQTT Client ID", "mqttClientID", mqttClientIDValue, STRING_LEN,"CoiacaDSC010000000002", nullptr, "CoiacaDSC010000000002");

IotWebConfTextParameter accessCodeParam = IotWebConfTextParameter("Access Code", "accessCode", accessCodeValue, NUMBER_LEN, "password", nullptr, "password");
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

static char TIMERStatusVal[][STRING_LEN] = { "Disabled", "Enabled"};
static char TIMERStatusNam[][STRING_LEN] = { "Disabled", "Enabled"};
static char PubTimerVal[][STRING_LEN] = { "No", "Yes"};
static char PubTimerNam[][STRING_LEN] = { "No", "Yes"};
static char timeDSTVal[][STRING_LEN] = { "No", "Yes"};
static char timeDSTNam[][STRING_LEN] = { "No", "Yes"};
static char mqttRetVal[][STRING_LEN] = { "0", "1"};
static char mqttRetNam[][STRING_LEN] = { "0", "1"};
static char mqttQoSParamVal[][STRING_LEN] = { "0", "1","2"};
static char mqttQoSParamNam[][STRING_LEN] = { "0", "1","2"};
IotWebConfParameterGroup group3 =  IotWebConfParameterGroup("group3", "Mqtt Advanced");
IotWebConfSelectParameter tiMerStatusParam = IotWebConfSelectParameter("Timer", "tiMerStatus", tiMerStatusValue, NUMBER_LEN, (char*)TIMERStatusVal, (char*)TIMERStatusNam, 2, NUMBER_LEN);
IotWebConfTextParameter tiMerStringParam = IotWebConfTextParameter("Timer String", "tiMerString", tiMerStringValue, STRING_LEN, "H20001AH23590A", nullptr, "H20001AH23590A");
IotWebConfSelectParameter publishTimerStringParam = IotWebConfSelectParameter("Publish Timer String", "publishTimerString", publishTimerStringValue, NUMBER_LEN,(char*)PubTimerVal, (char*)PubTimerNam, sizeof(PubTimerVal), NUMBER_LEN);
IotWebConfTextParameter ntpServerParam = IotWebConfTextParameter("NTP server", "ntpServer", ntpServerValue, STRING_LEN, "pool.ntp.org", nullptr, "pool.ntp.org");
IotWebConfNumberParameter timeZoneParam = IotWebConfNumberParameter("Time Zone", "timeZone", timeZoneValue, NUMBER_LEN, "-3", "-11..14", "min='-11' max='14' step='1'");
IotWebConfNumberParameter ntpUpdateIntervalParam = IotWebConfNumberParameter("NTP Update interval (seconds)", "ntpUpdateInterval", ntpUpdateIntervalValue, NUMBER_LEN, "300", "1..500", "min='1' max='500' step='1'");
IotWebConfSelectParameter timeDSTParam = IotWebConfSelectParameter("DST", "timeDST", timeDSTValue, NUMBER_LEN,(char*)timeDSTVal, (char*)timeDSTNam, sizeof(timeDSTVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfSelectParameter mqttRetainParam = IotWebConfSelectParameter("MQTT Retain", "mqttRetain", mqttRetainValue, NUMBER_LEN, (char*)mqttRetVal, (char*)mqttRetNam,sizeof(mqttRetVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfSelectParameter mqttQoSParam = IotWebConfSelectParameter("MQTT QoS", "mqttQoS", mqttQoSValue, NUMBER_LEN, (char*)mqttQoSParamVal, (char*)mqttQoSParamNam, sizeof(mqttQoSParamVal) / NUMBER_LEN, NUMBER_LEN);

static char enableRConfigPVal[][STRING_LEN] = {0,1};
static char enableRConfigPNam[][STRING_LEN] = {"No","Yes"};
static char secureAllTrafficPVal[][STRING_LEN] = {0,1};
static char secureAllTrafficPNam[][STRING_LEN] = { "No", "Yes"};
static char enableMonitoringPVal[][STRING_LEN] = {0,1,2,3};
static char enableMonitoringPNam[][STRING_LEN] = {"Disabled","Triggered","Armed","All"};
static char enableMqttDebugPVal[][STRING_LEN] = {0,1};
static char enableMqttDebugPNam[][STRING_LEN] = { "No", "Yes"};
static char remoteConfigRetainPVal[][STRING_LEN] = {0,1};
static char remoteConfigRetainPNam[][STRING_LEN] = { "0", "1"};
static char remoteConfigQoSPVal[][STRING_LEN] = {0,1,2};
static char remoteConfigQoSPNam[][STRING_LEN] = { "0", "1","2"};
IotWebConfParameterGroup group4 =  IotWebConfParameterGroup("group4", "Timer Config");
IotWebConfSelectParameter enableRConfigParam = IotWebConfSelectParameter("Enable Remote Management", "enableRConfig", enableRConfigValue, NUMBER_LEN, (char*)enableRConfigPVal, (char*)enableRConfigPNam, sizeof(enableRConfigPVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfPasswordParameter RConfigPwdParam = IotWebConfPasswordParameter("Remote Management Password", "RConfigPwd", RConfigPwdValue, STRING_LEN, defRConfigPwdValue.c_str(), nullptr, defRConfigPwdValue.c_str());
IotWebConfTextParameter remoteConfigMqttServerParam = IotWebConfTextParameter("Remote Management MQTT server", "remoteConfigMqttServer", remoteConfigMqttServerValue, STRING_LEN, "mqtt.coiaca.com", nullptr, "mqtt.coiaca.com");
IotWebConfNumberParameter remoteConfigMqttPortParam = IotWebConfNumberParameter("Remote Management MQTT server port (TLS)", "remoteConfigMqttPort", remoteConfigMqttPortValue, NUMBER_LEN, "8883", "1..9999", "min='1' max='9999' step='1'");
IotWebConfTextParameter remoteConfigMqttUserParam = IotWebConfTextParameter("Remote Management MQTT user", "remoteConfigMqttUser", remoteConfigMqttUserValue, STRING_LEN, "DSC010000000002", nullptr, "DSC010000000002");
IotWebConfTextParameter remoteConfigMqttPwdParam = IotWebConfTextParameter("Remote Management MQTT password", "remoteConfigMqttPwd", remoteConfigMqttPwdValue, STRING_LEN, defRemoteConfigMqttPwdValue.c_str(), nullptr, defRemoteConfigMqttPwdValue.c_str()); 
IotWebConfSelectParameter forceSecureAllTrafficParam = IotWebConfSelectParameter("Force all traffic through this secure connection", "forceSecureAllTraffic", forceSecureAllTrafficValue, NUMBER_LEN,(char*)secureAllTrafficPVal, (char*)secureAllTrafficPNam, sizeof(secureAllTrafficPVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfTextParameter remoteConfigTopicParam = IotWebConfTextParameter("Remote Management Command Topic", "remoteConfigTopic", remoteConfigTopicValue, STRING_LEN, "RMgmt/DSC010000000002", nullptr, "RMgmt/DSC010000000002");
IotWebConfTextParameter remoteConfigResultTopicParam = IotWebConfTextParameter("Remote Management Result Topic", "remoteConfigResultTopic", remoteConfigResultTopicValue, STRING_LEN, "RMgmt/DSC010000000002/results", nullptr, "RMgmt/DSC010000000002/results");
IotWebConfSelectParameter enableMonitoringParam = IotWebConfSelectParameter("Enable Monitoring", "enableMonitoring", enableMonitoringValue, NUMBER_LEN, (char*)enableMonitoringPVal, (char*)enableMonitoringPNam, sizeof(enableMonitoringPVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfTextParameter monitoringTopicParam = IotWebConfTextParameter("Monitoring Topic Prefix", "monitoringTopic", monitoringTopicValue, STRING_LEN, "MNTR/DSC010000000002", nullptr, "MNTR/DSC010000000002");
IotWebConfSelectParameter enableMqttDebugParam = IotWebConfSelectParameter("Enable MQTT Debug", "enableMqttDebug", enableMqttDebugValue, NUMBER_LEN,(char*)enableMqttDebugPVal, (char*)enableMqttDebugPNam, sizeof(enableMqttDebugPVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfTextParameter MqttDebugTopicParam = IotWebConfTextParameter("MQTT Debug Topic", "MqttDebugTopic", MqttDebugTopicValue, STRING_LEN, "RMgmt/DSC010000000002/debug", nullptr, "RMgmt/DSC010000000002/debug");
IotWebConfSelectParameter remoteConfigRetainParam = IotWebConfSelectParameter("Remote Management MQTT Retain", "remoteConfigRetain", remoteConfigRetainValue, NUMBER_LEN, (char*)remoteConfigRetainPVal, (char*)remoteConfigRetainPNam, sizeof(remoteConfigRetainPVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfSelectParameter remoteConfigQoSParam = IotWebConfSelectParameter("Remote Management MQTT QoS", "remoteConfigQoS", remoteConfigQoSValue, NUMBER_LEN,  (char*)remoteConfigQoSPVal, (char*)remoteConfigQoSPNam, sizeof(remoteConfigQoSPVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfParameterGroup group6 =  IotWebConfParameterGroup("group6", "Mqtt Remote Management");



void setup() {

/*
// Optional configuration
  dsc.hideKeypadDigits = false;      // Controls if keypad digits are hidden for publicly posted logs (default: false)
  dsc.processRedundantData = false;  // Controls if repeated periodic commands are processed and displayed (default: true)
  dsc.processModuleData = true;      // Controls if keypad and module data is processed and displayed (default: false)
  dsc.displayTrailingBits = false;   // Controls if bits read as the clock is reset are displayed, appears to be spurious data (default: false)
*/
  

//ESP.wdtDisable();
pinMode(13, FUNCTION_3); // esto es para que el pin 13 se comporte como GPIO13 
pinMode(14, FUNCTION_3); // esto es para que el pin 14 se comporte como GPIO14 
pinMode(13, OUTPUT);
pinMode(14, OUTPUT);

 
  Serial.begin(115200);

  Serial.println("Starting up...");


  //group1.addItem(&deviceIDParam);
  //iotWebConf.addParameterGroup(&group1);
  
  group2.addItem(&mqttServerParam);
  group2.addItem(&mqttPortParam);
  group2.addItem(&mqttUserNameParam);
  group2.addItem(&mqttUserPasswordParam);
  group2.addItem(&mqttClientIDParam);

  group3.addItem(&accessCodeParam);
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

  group4.addItem(&tiMerStatusParam);
  group4.addItem(&tiMerStringParam);
  group4.addItem(&publishTimerStringParam);
  group4.addItem(&ntpServerParam);
  group4.addItem(&timeZoneParam);
  group4.addItem(&ntpUpdateIntervalParam);
  group4.addItem(&timeDSTParam);
  group4.addItem(&mqttRetainParam);
  group4.addItem(&mqttQoSParam);
  
  group6.addItem(&enableRConfigParam);
  group6.addItem(&RConfigPwdParam);
  group6.addItem(&remoteConfigMqttServerParam);
  group6.addItem(&remoteConfigMqttPortParam);
  group6.addItem(&remoteConfigMqttUserParam);
  group6.addItem(&remoteConfigMqttPwdParam);
  group6.addItem(&forceSecureAllTrafficParam);
  group6.addItem(&remoteConfigTopicParam);
  group6.addItem(&remoteConfigResultTopicParam);
  group6.addItem(&enableMonitoringParam);
  group6.addItem(&monitoringTopicParam);
  group6.addItem(&enableMqttDebugParam);
  group6.addItem(&MqttDebugTopicParam);
  group6.addItem(&remoteConfigRetainParam);
  group6.addItem(&remoteConfigQoSParam);

  iotWebConf.setStatusPin (STATUS_PIN); 
  iotWebConf.addSystemParameter(&deviceIDParam);
  //iotWebConf.addParameterGroup(&group1);
  iotWebConf.addParameterGroup(&group2);
  iotWebConf.addParameterGroup(&group3);
  iotWebConf.addParameterGroup(&group4);
  iotWebConf.addParameterGroup(&group6);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.setWifiConnectionCallback(&wifiConnected);
//  iotWebConf.setupUpdateServer(&httpUpdater);
  iotWebConf.setConfigSavedCallback(&configSaved);
  

  // FIN Para customizar HTML

  // -- Initializing the configuration.
  boolean validConfig = iotWebConf.init();
  Serial.println("Valid config del la WEB: " + (String) validConfig);
  // meter aca las validaciones y los reemplazos por los valores default
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
  if (atoi(forceSecureAllTrafficValue) != 1 || atoi(enableRConfigValue) != 1) {
      Serial.println("Conexion inicial en el setup mqtt");
      Serial.println("mqttServerValue: " + (String) mqttServerValue);
      Serial.println("mqttPortValue: " + (String) mqttPortValue);
      mqttClient.begin(mqttServerValue, atoi(mqttPortValue), net);
      mqttClient.onMessage(mqttMessageReceived);
  }

  // Conexion mqtt para la Configuracion remota
  if (atoi(enableRConfigValue) == 1 || atoi(forceSecureAllTrafficValue) == 1)  {  
    mqttClientRConf.begin(remoteConfigMqttServerValue, atoi(remoteConfigMqttPortValue), netRConf);
    mqttClientRConf.onMessage(mqttRConfMessageReceived);
  }


  ///NTP
  Udp.begin(localPort);
  //setSyncProvider(getNtpTime);
  setSyncInterval(atoi(ntpUpdateIntervalValue));

  

}



void loop() {

  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();

  if (atoi(forceSecureAllTrafficValue) != 1) {
    mqttClient.loop();
    }
  if (atoi(enableRConfigValue) == 1 || atoi(forceSecureAllTrafficValue) == 1) {
    Serial.println("Remote: enableRConfigValue es:"+ (String)enableRConfigValue);
    Serial.println("Remote: forceSecureAllTrafficValue es:"+ (String)forceSecureAllTrafficValue);
    mqttClientRConf.loop();
    }

 
  // Conexion funcionalidad principal 
    if (atoi(forceSecureAllTrafficValue) != 1) {
      
      if (needMqttConnect){
            if (connectMqtt()){
              needMqttConnect = false;
            }
           // Serial.println("el mqtt esta conectado: "+ (String)mqttClient.connected());
      }
      
      else if ((iotWebConf.getState() == iotwebconf::OnLine) && (!mqttClient.connected())){
            // --------------- SerialDebug: ---------
            Serial.println("============================== MQTT reconnect");
            // --------------- SerialDebug: ---------
            // --------------- mqttDebug: --------- 
            if (atoi(enableMqttDebugValue) == 1) {
              mqttClientRConf.publish(MqttDebugTopicValue,
               (String) deviceIdValue + " - MQTT reconnect",
                (bool) atoi(remoteConfigRetainValue),
                 atoi(remoteConfigQoSValue));}
            // --------------- mqttDebug: --------- 
            connectMqtt();
          }
    }


  // Conexion configuracin remota  
    if (atoi(enableRConfigValue) == 1 || atoi(forceSecureAllTrafficValue) == 1) {
      if (needRConfMqttConnect){
        if (connectMqttRConf()){
          needRConfMqttConnect = false;
        }
      }
    else if ((iotWebConf.getState() == iotwebconf::OnLine) && (!mqttClientRConf.connected())){
      // --------------- SerialDebug: ---------
      Serial.println("MQTT Rconf reconnect");
      // --------------- SerialDebug: ---------         
      connectMqttRConf();
    }
    }

  // Restart
  if (needReset){
    // --------------- SerialDebug: --------- 
    Serial.println("Rebooting after 1 second.");
    // --------------- SerialDebug: --------- 
    // --------------- mqttDebug: --------- 
    if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Rebooting after 1 second..." , (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
    // --------------- mqttDebug: --------- 
    
    iotWebConf.delay(1000);
    ESP.restart();
  }

  //NTP sync
  if (timeStatus() == timeNotSet && (iotWebConf.getState() == iotwebconf::OnLine)) {
    setSyncProvider(getNtpTime);
    
    // --------------- SerialDebug: ---------
    Serial.println((String)"DateTime: " + getReadableTime() + " - Timezone: " + timeZoneValue + " DST: " + timeDSTValue + " - TimeString: " + getTimeString());
    // --------------- SerialDebug: --------- 
    // --------------- mqttDebug: ---------
    if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - DateTime: " + getReadableTime() + " - Timezone: " + timeZoneValue + " DST: " + timeDSTValue + " - TimeString: " + getTimeString(), (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
    // --------------- mqttDebug: --------- 
  }
      

// Timer
  if (atoi(tiMerStatusValue) == 1) {
      if (millis() - timerMillis > 20000) {   // Run timer every 20 seconds
        runTimer();  
        timerMillis = millis();
        }
  }


//Publica info
if (iotWebConf.getState() == iotwebconf::OnLine) {
  if ( atoi(updateIntervalValue) > 0 && millis() - lastPublish > (atoi(updateIntervalValue) * 1000) ) { 
        // --------------- SerialDebug: --------- 
        Serial.println(getTimeString().substring(1) + " - Keep Alive interval elapsed. Automatically publishing...");
        // --------------- SerialDebug: --------- 
        // --------------- mqttDebug: --------- 
        if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - [" + getTimeString().substring(1) + "] - Keep Alive interval elapsed. Automatically publishing...", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
        // --------------- mqttDebug: --------- 
  publicaEstados();
  lastPublish = millis();
  }
}



 // Lo que sigue detiene dsc.stop(); el proceso de la alarma si el wifi esta desconectado para que no genere conflicto
 // con las interrumpciones
 // El dsc.begin(); esta en la rutina de callback de cuando se conecta el wifi
 // El dsc.stop() lo agregue yo pero se supone que en la proxima version de la librera del bus ya va a venir.
 


if (iotWebConf.getState() != iotwebconf::OnLine) {
  if (DSCBegined){
    dsc.stop();
    DSCBegined = false;
    // --------------- SerialDebug: ---------
    Serial.println(F("DSC Keybus Interface stoped."));
    // --------------- SerialDebug: ---------
    // --------------- mqttDebug: --------- 
    // ESTO NO PORQU NO ESTA CONECTADO: if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " DSC Keybus Interface stoped.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
    // --------------- mqttDebug: --------- 
  }
}

if (DSCBegined){   //Si el bus esta iniciado, hace lo propio con la alarma
  doDSC();

  // Publica la particion activa para la escritura (si cambio)
  if (activePartition != dsc.writePartition){
      activePartition = dsc.writePartition;
      
      if (atoi(forceSecureAllTrafficValue) != 1){
        mqttClient.publish(mqttActivePartitionTopicValue, String(activePartition), true, atoi(mqttQoSValue));
        }
      if (atoi(forceSecureAllTrafficValue) == 1){
        mqttClientRConf.publish(mqttActivePartitionTopicValue, String(activePartition), true, atoi(mqttQoSValue));
        }
   }      
}



  //Agregar eco para corregir estados (primero verificar por que no manda el online)
  //si no es force o si es y en funcion de lastSentStatus
 




}  ////////// end of LOOP ////////////////////////////////////////////////////////////////////////////////////


/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  //s += "<title>Coiaca: IoT Device Configuration</title></head><body><center><br /><b>Coiaca</b> device";
  
  // Para customizar HTML
    s += "<title>Coiaca: IoT Device Configuration</title></head><body><center>";
    s += FPSTR(CUSTOMHTML_BODY_INNER);
  // FIN Para customizar HTML
  
  s += "<br /><br />Device ID: <b>";
  s += deviceIdValue;
  s += "</b><br /><br />";
/*  s += "<ul>";
  s += "<li>MQTT server: ";
  s += mqttServerValue;
  s += "</ul>";*/
  s += "<a href='config'>Config</a><br />";
  s += "</center></body></html>\n";

  // Para customizar HTML
  // server.send(200, "text/html", s);
  server.send(200, "text/html; charset=UTF-8", s);
  // FIN Para customizar HTML
}

void wifiConnected(){
  Serial.println("================================LLEGO AL WIFI conectado");
  needMqttConnect = true;
  needRConfMqttConnect = true;
  
  dsc.begin();
  DSCBegined = true;
  /* --------------- SerialDebug: --------- */
  Serial.println(F("DSC Keybus Interface begined."));
  /* --------------- SerialDebug: --------- */
  /* --------------- mqttDebug: --------- */
  // NO ESTA CONECTADO AL BROKER POR LO QUE NO LO PUEDE PUBLICAR if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " DSC Keybus Interface begined.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  /* --------------- mqttDebug: --------- */
  
  //Publico la particion activa para la escritura
  activePartition = dsc.writePartition;
  if (atoi(forceSecureAllTrafficValue) != 1){
    mqttClient.publish(mqttActivePartitionTopicValue, String(activePartition), true, atoi(mqttQoSValue));
  }
  if (atoi(forceSecureAllTrafficValue) == 1){
    mqttClientRConf.publish(mqttActivePartitionTopicValue, String(activePartition), true, atoi(mqttQoSValue));
  }   
}

void configSaved(){
/* --------------- SerialDebug: --------- */
Serial.println("Configuration updated.");
/* --------------- SerialDebug: --------- */
/* --------------- mqttDebug: --------- */
if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Configuration updated.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
/* --------------- mqttDebug: --------- */
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
