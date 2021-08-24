/*
 * 
 * Interface para sistemas de alarma DSC Power Series
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
# include <ESP8266HTTPUpdateServer.h>
//#include <ESP8266HTTPUpdateServer.h>
#include <MQTT.h>
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

String deviceID = "DSC010000000003"; // >>REPLACE_FOR_PERSO<<
char char_deviceID[18]="DSC010000000003";
// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "coiaca-DSC010000000003";  // >>REPLACE_FOR_PERSO<<

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "cEw5oTQ0"; // >>REPLACE_FOR_PERSO<<

// Default Remote Management password and Coiaca MQTT broker pwd  
String defRConfigPwdValue = "wDJGtmE6"; // >>REPLACE_FOR_PERSO<<
String defRemoteConfigMqttPwdValue = "3XrTOdcc"; // >>REPLACE_FOR_PERSO<<
char* mqttRCClientIDValue = "DSC010000000003RM"; //Para diferenciar el clientID entre el de la func ppal y el e RC  >>REPLACE_FOR_PERSO<<
////// FIN CUSTOM DATA /////////////

#define STRING_LEN 128
#define NUMBER_LEN 8

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "DSC_v0.6.1"

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
const char CUSTOMHTML_BODY_INNER[] PROGMEM = "<div><img src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAF4AAABaCAIAAAC+IXYvAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAACxMAAAsTAQCanBgAABBeSURBVHja7VyHW5PXHu6/cG+HVgVHrW1va5etvb0MRZaIIuJCBbVVe60DR4dY7bLVqm2vbW2LZLBXEMqQXZaAikxlQ8LeJBAIECCD3vf7DsRISAgQDPXJ95yHh3w553znvN9vvL9zfidP/GW41FxPGCAwQGOAxgCNARoDNAZolC+pTN7RJU64VXvFN//4pXRX9zin49Gb3KJ3fhp39Hzqd6yciJSquuZuiUQ2PDz8+EODScrkw7cLmw9/m/LvHcELLJkLLFnGVqyF1uxFNg8KPhpbs42sWPPXMF/fHLDns4SIZK4MLR8tRk88MlCEPQNX/PLN9oTOXc0wtmYttvXSsgCpeRYMYHT2l1tNbSK5XP74QAPdOc+4++omv4XWLAiF9qCoYrTUzuv4xXRh78DjAE18Zu27O4OhNdMBRVFodWP9y8H395B7f2NoBoek31zLNrZiTh8R1WJkyXQ5FScUDc6cAZohaCjLsv2jmGlq0ET6xTJxCS7lCf5O0HR2i1fvDV04GVs7Rf2yZr/i6FdV1/X3gKZbNPD8Ou+ZExZV6zPfklnCFcxqaKD2vf1D6w9Hao8L0bhl9j5gMdNB5zUn/7qmnuFZCw1Y2YlLacZaTxLO2NEtilsvrG8RnbiUTojfuDMnRXNXNgfCBwalsxSa1LsNE85BecLQu37xUHWDsLxaIJPLNx6NVm2LOUMiLPeFL7JhTYQO60ffPB06LJ1B09AqmnD0Y2ZifygCbNDhSNSzFp5Sqezzq7cUzalYwYqFSOIiK0cilfeLJXXNPSudgzT0j6/mrPLMKmiaXdDgXR29kAJ/MSkD8cJ6H6FooJTHzy9tgzJuoI0UKe99nng9qSohq048IGX/UWzqGtou6Pvi11uatXWRjRc8I/jULIKmhMs3nryrhuBsPBqFsAi4HL2QCnJIuZs1zCv+eXL5cGtHb0VNJ776wScXIPK7+r+lCOQET0GFyBTubIEGZuLDcynqWAxmC+qx7aPYF9f7qFoizCQuo0bQJX7a1IPceWNLIPoMiil/2uzaM+ae/tFlJOYW9Q1ZvH99QoVFBRMXTv+AZFZA09gmWmyr1vpCTTq7qWgQ9mL3ZwljEIRNSciq7eoemGPuSe7YfxiByiauHNIh/u77IgmqZObK0YZDoj6kLz23YVZAczWgkKIn40Y6Vqy7Ra3NHb1Ox6KTbteB1D+31kszNA5HItHnim2BCmhQZ1IBB2riHegfGoi6+d5QDZpfWN4OS7RkrRczrJjX0LXU7kEA/da2oM0nYnKKW3tEg9YHwiFfKGBG6NblVDx4CnRQHdnRXPAC4Pv0DA1cDFysBkP7yY8ZEqlM1DcIk/STXwFRCvy9xM4dHJQCWcwB1kQqHS0yukjlqI9WZ37OmkIs9uxqxo30aj1Dwwwrmm/B1OwytpyIweRP/S+T6AX+7j6dgMlz4iss94WZ7w5dtWdswU3r/WGRqTyJRLbJLVrVQk1I/xD3T3M9cFrQ4NnbPorRPEp8+/rmAAjOjk9iyR1IWWZ+Y0FZ+yhS4xcCYlFlR+zNaoVg4uaStWyQl+WO/ktsNQFk6sLBQ/UGDSR/9XvXtaF2fGE/JJxE5GAuzW0iSARmjnkm36nnNQjHlPjMWng9ABSfWVNR2zl/DWPE8a9l1zb1wDcjXMoraRtj1JXL8o1+4um58GlBA9755tYAbVzGN9eyhyQyhEsQlvyyNvwfkcyFl33NKUBd58QGg/WIB6VoUljWjlJe0zkkkdod/AMeHbTQcp9apgPDDyqkN2gw6JccfLWBBgWELSqFV8zlF3H5mB4NDeuNzeqh2eBLoBkYkKBJCU+AUt0khJrs/DTu5OWbw38Nr9obqg4aIytmK79Pb9D0iSVL7bwnsea0hgnf8ZTpteb23sgUnlbQ0Ar1lKkHGlLFggFnT7xYVn7TEvUKBUoFWdMjNEMwFpqFhcg2/b/XqBlmtXRMApqqui4lM0x5nz2nE0B8ALQGMwxoYOz1KzXqPagN6/SVrOaOPnCfC4zsefRMiOy0tE8RGuWVLQXWi9US8Ra9QQNPAWOpTmTedg6CXUjMqo1O44HafXH19olL6SjHLqQJewYitIQmo6a5XfT+2cQDXyZtOha9kN7P0oYfG1my4Mv06aHgYtStSLq4x6GO1f7w5ZvG8aMww0aWTE3QjHoo5ZulXIHZbo420CyyYfX0DuoNGgiFiQtHndQsd/SDxtW39FQ3CsGGbQ+Eg4ahvLMjuI3fS0kNxWu8vCNKwpKqxhRWWDG+IgoFl79yR7CJSwgcE6L8+5UdxHhphualDb79Yv05b7BhR7coDSECCEj2vZayasHGI1HGo7oAK5CR15hf1m5sORI3jFvIV3D2USmUfBHe7P5TJsjeq5v8JoRmpXMQKILeoIEF+fr3O0bqt26VU0OUAxz4F4lU7htZCqHDHFY6B6uUIFNXTnB8BcjhlpM3RoJSG/aBL//EnRVbAyeMTtYfjtRnDEWtlreI5qzynELOw0VWTnfvIOYJzjpugbZ2i+jIm0YBfx3douH18T5KuHywcA0easEaxm9B080X0MF6DcR7ajvWrzj6mbqGwGehH9iR1e+Fopjt5eBjQlatya6Q5RspJzVC/G3ZpdWCUp5g/aEI8Km03EYN68Rw9qK+QT1Dg8v9SuakluAU+kWsCSJydGK+J5Tcx4TxMSyx0ohaRX9odaqpTXSBcRd1uPVdheXt6jY8UQE2blYsgFLbCVZMbRa0oRSvOvnbH4p4ezu9o0Tf2XuGWqw03c1R6BoFTVLVGKGAMw6OK5dK5bXN3VBDam9HzVYnBhMUWz4roMFA1x78QxtorPaFCYRiaCEIEdlUAjOOuUktxy2z91b4NXy8nlipAg0bPvvg18lXAwsdjkZp2MAAIerqFs+Wfaik23UTLlNC/wNulPEahK9vCQiKKYehTblTX9vULZcPn7ycTqsPG04adpe27j02HwBulkrwwV5gqWkJXYfbu7qBBnGw1f4wzYID0xAcV1HGE0Cb/G+U9vYP5RS1JN+pRwRABATNd52Kh8dFxAyzwu/q1xCgqROZlxx8acGcTXveeaVtT5tem0Ch9odDWGAvICkX2TmE1ykCRXy8GlTY2S1+djVjlzswGn55o6/mGHIcwYwp01VGgM6gwYAus3M1qBWhs8vWe59nZMMrLVBZUqC2us8mAhGPkMLbhc2wF5Aa7aFB83UHI+SzMFOCWvQbkFgfCJ9ynhpQgFX+wSeXWy+8X9Hh6p6w0HoSnABBbGOrSIfT0XFWVlVd1+ubA6acrbaIVqvn13mT2FJ7XMADEJfpNhtU97l8OUWti23ZjzKXD37N+49inWfJzkgGKOj8MnufR4AOZb+sWN6RpTORPTxTKdW5Ja1vbgmYUXRIyhsIwQxNYQazzetbekxdQmYIHZK2U1jWPnOHXWb2jAJiiMvsnDmrGDoEiCy8bz1xY5rrm3qGhlz3ytvtP4wgtnnqzouQQ2v2u7uC/7xTB/oz08N+ROehZDL5rYLmTW7RxlM6tUAvmzJNXTnRqTzdJgfrHxrCegAQ4slzHnfe2RE8x9zTyJI54XLXM+aerzr5H7+Yll/SJn20B+n0dvaSLxRf8S9AZLDhcKT5ntC3tgW+ttn/NSf/FdsCTV04dgcjXN3jv/z1NiJ1fY1Qzyd2YTIgShIpigw2G0WCQudmPQJrMquhmc2XARoDNLMfGiqp85esn/zzxbpICH+soNlzJhEu2diKdfaXWwZoHro2HYsm208fnks2QPPQlVfSttzR792dwdx64WMFTXWjMOZmdVQqr4TL17RewxVcT6y8mdsglcpViIxcKpFJpWN/RAMfs++1cBIq8kupBLxhKjSVkqLav6BbnHy7Ho8oquxQtxiMDjPyGgNjyklGH547OCShNtK1ptRaQYPeOPEVZrtDQdvnWTBRQPNB9q8nVCoTMzwyIbP23V0hc1d5koxG/HPom5T2zn5Fnd9DCp8yvfaP//yeeKtOwfrQ6mVHP9JqzirPt7cHscOL8fEps2srtgYq88Pbhc2IVJ8x83zWgkEqv+Loy7h+f2hIplyNk1D5ioMvBoA6c809zVw5wOhJE48nTT2cP47RkkxqBc23niNntGAjjKyoQhZuF1qxznlkK6px4ivJmjlJoqF+YoQOl1c6B7d09JI6zLAi8m1Kdj25E5lcRQLOhfSvjxhZsuhfIiH7MF54HyMTHh72CLm3wJKh6J96xOiJh20fxfT1D5HXww4vUmxsoTcjehjGo2ko751J1Bk0flFlC+jMn+fX+VwLvX+vor2oiu8dWfqcndcye+/vvXPJGZJSngDvmWy//hyQn1PSmnq33sU9Hm0x9HX0KSdVaHp6B5fTeUSQAvcrmfcqOtDPDz55o3N7AE1BWfvokjOVoJ1T3FpeIwiKrXjbOeifJh5nfs7qpDdzW/l9S+mcdpRjF9Oo5Aou/6vf7iwazUTRGTR4Cav3juQw5Je2Kn+FGFp5McntQuoiateVGf5nlXK1rSdv4P7c1Yzs+y2q0ATHlUPmgZ37lQzlVr8GFdAbeCPQYBgHz/2JhtBl74gS5ZoDQ1Llw5bonxxpdbuQplztMiuHSLHOoAFDm2dBrdHtPp0g03jCCKYH1eB6xiy+pec0kslfYNxVhWb/l0n4CoBCBJRbNbaKnrN7IDUwdiYuIXjtCNA1b8LhDaEaTGG36KFhwN4ttdOp1JTy+CTf9+Pvb2qu+eIGagvB1JUz5mRAUSUf4GK4H3+foQqNw5FIvOT5axgdgoeS5oHv8/beD6CRyV+gs3CdjkVPQClPJ9BZWcwx94U9Ay87+OkSGkFX/zz6VMmuU/Gaz6VhDqj25tbArp6HfncnPIkLoYAw/+ibqwrN4W9TjCmDyiQ+W3FV1XXRaZ4KqZG/4xxEHVrdHKBZak5eTkcreNLG9l7l+/ADJMFHl2bYcl8Y4a8KdzviWVJ5BeXtCo4A4k92y34LLlR++Wvo5vCjlbWdqtAk3qqdb0FJ5ZaTMYpjtngH7iPnyh7YGrfv0siC+c8BBcqqXVTVcbeoRTFbkAzy41uOR6MVNghs5sj5VCqxVLfQxGXUGNF5rUts2UfOp0QkV8HQ/vfrZNyB9sKbkGr1LT2k2mIbtuvp+KDY8l8CCkx3c+iBsj74KnlcDwU4iJnHHftDEQE3ysKSqlxOxRlbj/VQNY3ddHYA5aF2fBJ3PbEqOr360x8yyHFCmBiCF17Giq0BxFtvPBr1k19+aEKF4hdAdAwNLu/IEgWvGSn0k6hT2jcfZIMXlrUpCIUi95fkjigkQpXX1DZ3w06RcStaGVmO5TUQnPjMGqCg3L/iJCdklug7qkE3yZkG5WGQO7qHBlfa3YY19Al0Rdn5aewYA4GrrFrg/Emcog5s5/988vrFD9YfmOFFZKtfAQ2RuG0nYxeRQ4U2rM3Ho8GJSFLAqj2hD7sFwfaPYymCYzuSCmh3MOKmSiIAAjQntxuKYYBeIVygmth6vX9W19CQq6OzH4OrrOnkd/VrWhLvEqNaTVO3bDLru+i8uIqvTSIItKaiphOP6OjUNIzmjl74R+BuWOUzLIAaoDFAY4DGAI0BGgM0BmgM17jX/wG/ZEkC0m7XzAAAAABJRU5ErkJggg=='/></div>\n";

// -- This is an OOP technique to override behaviour of the existing
// IotWebConfHtmlFormatProvider. Here two method are overriden from
// the original class. See IotWebConf.h for all potentially overridable
// methods of IotWebConfHtmlFormatProvider .

// FIN Para customizar HTML

IotWebConfParameterGroup group1 =  IotWebConfParameterGroup("group1", "");
IotWebConfTextParameter deviceIDParam = IotWebConfTextParameter("Device ID12", "deviceId", deviceIdValue, STRING_LEN, char_deviceID, nullptr, char_deviceID);

IotWebConfParameterGroup group2 =  IotWebConfParameterGroup("group2", "MQTT Config");
IotWebConfTextParameter mqttServerParam = IotWebConfTextParameter("MQTT Server", "mqttServer", mqttServerValue, STRING_LEN, "text", nullptr, "mqtt.coiaca.com");
IotWebConfNumberParameter mqttPortParam = IotWebConfNumberParameter("MQTT server port (unsecure)", "MQTTPort", mqttPortValue, NUMBER_LEN, "1883", "1..9999", "min='1' max='9999' step='1'");
IotWebConfTextParameter mqttUserNameParam = IotWebConfTextParameter("MQTT user", "mqttUser", mqttUserNameValue, STRING_LEN, "mqttusr", nullptr, "mqttusr");
IotWebConfPasswordParameter mqttUserPasswordParam = IotWebConfPasswordParameter("MQTT password", "mqttPass", mqttUserPasswordValue, STRING_LEN, "mqttpwd", nullptr, "mqttpwd");
IotWebConfTextParameter mqttClientIDParam = IotWebConfTextParameter("MQTT Client ID", "mqttClientID", mqttClientIDValue, STRING_LEN,"CoiacaDSC010000000011", nullptr, "CoiacaDSC010000000011");

IotWebConfTextParameter accessCodeParam = IotWebConfTextParameter("Access Code", "accessCode", accessCodeValue, NUMBER_LEN, "password", nullptr, "password");
IotWebConfTextParameter mqttStatusTopicParam = IotWebConfTextParameter("Status Topic", "mqttStatusTopic", mqttStatusTopicValue, STRING_LEN, "DSC010000000011/Status", nullptr,"DSC010000000011/Status");
IotWebConfTextParameter mqttBirthMessageParam = IotWebConfTextParameter("Birth Message", "mqttBirthMessage", mqttBirthMessageValue, STRING_LEN, "online", nullptr, "online");
IotWebConfTextParameter mqttLwtMessageParam = IotWebConfTextParameter("LWT Message", "mqttLwtMessage", mqttLwtMessageValue, STRING_LEN, "offline", nullptr, "offline");
IotWebConfTextParameter mqttNoDSCParam = IotWebConfTextParameter("Disconnected Message", "mqttNoDSC", mqttNoDSCValue, STRING_LEN, "Alarm Disconnected", nullptr, "Alarm Disconnected");
IotWebConfTextParameter mqttPartitionTopicParam = IotWebConfTextParameter("Partition Topic Prefix", "mqttPartitionTopic", mqttPartitionTopicValue, STRING_LEN, "DSC010000000011/Partition", nullptr, "DSC010000000011/Partition");
IotWebConfTextParameter mqttActivePartitionTopicParam = IotWebConfTextParameter("Active Partition Topic", "mqttActivePartitionTopic", mqttActivePartitionTopicValue, STRING_LEN, "DSC010000000011/activePartition", nullptr, "DSC010000000011/activePartition");
IotWebConfTextParameter mqttZoneTopicParam = IotWebConfTextParameter("Zone Topic Prefix", "mqttZoneTopic", mqttZoneTopicValue, STRING_LEN, "DSC010000000011/Zone", nullptr, "DSC010000000011/Zone");
IotWebConfTextParameter mqttFireTopicParam = IotWebConfTextParameter("Fire Topic Prefix", "mqttFireTopic", mqttFireTopicValue, STRING_LEN, "DSC010000000011/Fire", nullptr, "DSC010000000011/Fire");
IotWebConfTextParameter mqttTroubleTopicParam = IotWebConfTextParameter("Trouble Topic", "mqttTroubleTopic", mqttTroubleTopicValue, STRING_LEN, "DSC010000000011/Trouble", nullptr, "DSC010000000011/Trouble");
IotWebConfTextParameter mqttCommandTopicParam = IotWebConfTextParameter("Commands Topic", "mqttCommandTopic", mqttCommandTopicValue, STRING_LEN, "DSC010000000011/cmd", nullptr, "DSC010000000011/cmd");
IotWebConfTextParameter mqttKeepAliveTopicParam = IotWebConfTextParameter("Keep Alive Topic", "mqttKeepAliveTopic", mqttKeepAliveTopicValue, STRING_LEN, "DSC010000000011/keepAlive", nullptr, "DSC010000000011/keepAlive");
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
IotWebConfTextParameter remoteConfigMqttUserParam = IotWebConfTextParameter("Remote Management MQTT user", "remoteConfigMqttUser", remoteConfigMqttUserValue, STRING_LEN, "DSC010000000011", nullptr, "DSC010000000011");
IotWebConfTextParameter remoteConfigMqttPwdParam = IotWebConfTextParameter("Remote Management MQTT password", "remoteConfigMqttPwd", remoteConfigMqttPwdValue, STRING_LEN, defRemoteConfigMqttPwdValue.c_str(), nullptr, defRemoteConfigMqttPwdValue.c_str()); 
IotWebConfSelectParameter forceSecureAllTrafficParam = IotWebConfSelectParameter("Force all traffic through this secure connection", "forceSecureAllTraffic", forceSecureAllTrafficValue, NUMBER_LEN,(char*)secureAllTrafficPVal, (char*)secureAllTrafficPNam, sizeof(secureAllTrafficPVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfTextParameter remoteConfigTopicParam = IotWebConfTextParameter("Remote Management Command Topic", "remoteConfigTopic", remoteConfigTopicValue, STRING_LEN, "RMgmt/DSC010000000011", nullptr, "RMgmt/DSC010000000011");
IotWebConfTextParameter remoteConfigResultTopicParam = IotWebConfTextParameter("Remote Management Result Topic", "remoteConfigResultTopic", remoteConfigResultTopicValue, STRING_LEN, "RMgmt/DSC010000000011/results", nullptr, "RMgmt/DSC010000000011/results");
IotWebConfSelectParameter enableMonitoringParam = IotWebConfSelectParameter("Enable Monitoring", "enableMonitoring", enableMonitoringValue, NUMBER_LEN, (char*)enableMonitoringPVal, (char*)enableMonitoringPNam, sizeof(enableMonitoringPVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfTextParameter monitoringTopicParam = IotWebConfTextParameter("Monitoring Topic Prefix", "monitoringTopic", monitoringTopicValue, STRING_LEN, "MNTR/DSC010000000011", nullptr, "MNTR/DSC010000000011");
IotWebConfSelectParameter enableMqttDebugParam = IotWebConfSelectParameter("Enable MQTT Debug", "enableMqttDebug", enableMqttDebugValue, NUMBER_LEN,(char*)enableMqttDebugPVal, (char*)enableMqttDebugPNam, sizeof(enableMqttDebugPVal) / NUMBER_LEN, NUMBER_LEN);
IotWebConfTextParameter MqttDebugTopicParam = IotWebConfTextParameter("MQTT Debug Topic", "MqttDebugTopic", MqttDebugTopicValue, STRING_LEN, "RMgmt/DSC010000000011/debug", nullptr, "RMgmt/DSC010000000011/debug");
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

  iotWebConf.setConfigPin(CONFIG_PIN);
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
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);

  iotWebConf.setWifiConnectionCallback(&wifiConnected);
//  iotWebConf.setupUpdateServer(&httpUpdater);

  iotWebConf.setStatusPin (STATUS_PIN);

  // FIN Para customizar HTML

  // -- Initializing the configuration.
  boolean validConfig = iotWebConf.init();
  // meter aca las validaciones y los reemplazos por los valores default
  if (!validConfig)
  {
    mqttServerValue[0] = '\0';
    mqttUserNameValue[0] = '\0';
    mqttUserPasswordValue[0] = '\0';
  }


  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });


  // Conexion mqtt para la funcionalidad principal
  if (atoi(forceSecureAllTrafficValue) != 1 || atoi(enableRConfigValue) != 1) {
      mqttClient.begin(mqttServerValue, atoi(mqttPortValue), net);
      mqttClient.onMessage(mqttMessageReceived);
  }

  // Conexion mqtt para la Configuraci�n remota
  if (atoi(enableRConfigValue) == 1 || atoi(forceSecureAllTrafficValue) == 1)  {  
    mqttClientRConf.begin(remoteConfigMqttServerValue, atoi(remoteConfigMqttPortValue), netRConf);
    mqttClientRConf.onMessage(mqttRConfMessageReceived);
  }


  ///NTP
Udp.begin(localPort);
//setSyncProvider(getNtpTime);
setSyncInterval(atoi(ntpUpdateIntervalValue));

}    ////////// end of SET UP ////////////////////////////////////////////////////////////////////////////////////



void loop() {
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();

  if (atoi(forceSecureAllTrafficValue) != 1) {mqttClient.loop();}
  if (atoi(enableRConfigValue) == 1 || atoi(forceSecureAllTrafficValue) == 1) {mqttClientRConf.loop();}

 
  // Conexion funcionalidad principal 
    if (atoi(forceSecureAllTrafficValue) != 1) {
      if (needMqttConnect){
            if (connectMqtt()){
              needMqttConnect = false;
            }
      }
          else if ((iotWebConf.getState() == iotwebconf::OnLine) && (!mqttClient.connected())){
                                
            /* --------------- SerialDebug: --------- */
            Serial.println("MQTT reconnect");
            /* --------------- SerialDebug: --------- */
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - MQTT reconnect", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            connectMqtt();
          }
    }


  // Conexion configuraci�n remota  
    if (atoi(enableRConfigValue) == 1 || atoi(forceSecureAllTrafficValue) == 1) {
      if (needRConfMqttConnect){
        if (connectMqttRConf()){
          needRConfMqttConnect = false;
        }
      }
    else if ((iotWebConf.getState() == iotwebconf::OnLine) && (!mqttClientRConf.connected())){
      /* --------------- SerialDebug: --------- */
      Serial.println("MQTT Rconf reconnect");
      /* --------------- SerialDebug: --------- */          
      connectMqttRConf();
    }
    }

  // Restart
  if (needReset){
    /* --------------- SerialDebug: --------- */
    Serial.println("Rebooting after 1 second.");
    /* --------------- SerialDebug: --------- */
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Rebooting after 1 second..." , (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
    /* --------------- mqttDebug: --------- */
    
    iotWebConf.delay(1000);
    ESP.restart();
  }

  //NTP sync
  if (timeStatus() == timeNotSet && (iotWebConf.getState() == iotwebconf::OnLine)) {
    setSyncProvider(getNtpTime);
    
    /* --------------- SerialDebug: --------- */
    Serial.println((String)"DateTime: " + getReadableTime() + " - Timezone: " + timeZoneValue + " DST: " + timeDSTValue + " - TimeString: " + getTimeString());
    /* --------------- SerialDebug: --------- */
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - DateTime: " + getReadableTime() + " - Timezone: " + timeZoneValue + " DST: " + timeDSTValue + " - TimeString: " + getTimeString(), (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
    /* --------------- mqttDebug: --------- */
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
        /* --------------- SerialDebug: --------- */
        Serial.println(getTimeString().substring(1) + " - Keep Alive interval elapsed. Automatically publishing...");
        /* --------------- SerialDebug: --------- */
        /* --------------- mqttDebug: --------- */
        if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - [" + getTimeString().substring(1) + "] - Keep Alive interval elapsed. Automatically publishing...", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
        /* --------------- mqttDebug: --------- */
  publicaEstados();
  lastPublish = millis();
  }
}


/*
 * Lo que sigue detiene dsc.stop(); el proceso de la alarma si el wifi esta desconectado para que no genere conflicto
 * con las interrumpciones
 * El dsc.begin(); esta en la rutina de callback de cuando se conecta el wifi
 * El dsc.stop() lo agregue yo pero se supone que en la pr�xima versi�n de la librer�a del bus ya va a venir.
 */


if (iotWebConf.getState() != iotwebconf::OnLine) {
  if (DSCBegined){
    dsc.stop();
    DSCBegined = false;
    /* --------------- SerialDebug: --------- */
    Serial.println(F("DSC Keybus Interface stoped."));
    /* --------------- SerialDebug: --------- */
    /* --------------- mqttDebug: --------- */
    // ESTO NO PORQU NO ESTA CONECTADO: if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " DSC Keybus Interface stoped.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
    /* --------------- mqttDebug: --------- */
  }
}

if (DSCBegined){   //Si el bus esta iniciado, hace lo propio con la alarma
  doDSC();

  // Publica la particion activa para la escritura (si cambi�)
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


/*
 * Agregar ec� para corregir estados (primero verificar por que no manda el online)
 * si no es force o si es y en funcion de lastSentStatus
 */




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

boolean connectMqtt() {
  unsigned long now = millis();
  if ((lastMqttConnectionAttempt + 1000) > now)
  {
    // Do not repeat within 1 sec.
    return false;
  }
   /* --------------- SerialDebug: --------- */
  Serial.println("Connecting to MQTT server...");
  /* --------------- SerialDebug: --------- */
  /* --------------- mqttDebug: --------- */
  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Connecting to MQTT server...", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  /* --------------- mqttDebug: --------- */
  
  if (!connectMqttOptions()) {
    lastMqttConnectionAttempt = now;
    return false;
  }

  /* --------------- SerialDebug: --------- */
  Serial.println("Connected MQTT server!");
  /* --------------- SerialDebug: --------- */
  /* --------------- mqttDebug: --------- */
  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Connected to MQTT server!", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  /* --------------- mqttDebug: --------- */
  

  if (atoi(forceSecureAllTrafficValue) != 1){
    if (dsc.keybusConnected) {
      mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
      lastSentStatus = String(mqttBirthMessageValue);

      /* --------------- SerialDebug: --------- */
      Serial.println("Status message published: " + lastSentStatus);
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

      
    }
    if (!dsc.keybusConnected) {
      mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
      lastSentStatus = String(mqttNoDSCValue);

      /* --------------- SerialDebug: --------- */
      Serial.println("Status message published: " + lastSentStatus);
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }
}

  
  if (atoi(forceSecureAllTrafficValue) == 1){
    if (dsc.keybusConnected) {
      mqttClientRConf.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
      lastSentStatus = String(mqttBirthMessageValue);

      /* --------------- SerialDebug: --------- */
      Serial.println("Status message published: " + lastSentStatus);
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }
    if (!dsc.keybusConnected) {
      mqttClientRConf.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
      lastSentStatus = String(mqttNoDSCValue);

      /* --------------- SerialDebug: --------- */
      Serial.println("Status message published: " + lastSentStatus);
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }
  }

  
  
                    

// Para publicar estados iniciales
  publicaEstados();

//Subcribe al topic de comando
  mqttClient.subscribe(mqttCommandTopicValue);
  
    /* --------------- SerialDebug: --------- */
    Serial.println((String) "Subcribed to: " + mqttCommandTopicValue);
    /* --------------- SerialDebug: --------- */
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Subcribed to: " + mqttCommandTopicValue), (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue);}
    /* --------------- mqttDebug: --------- */
 

  return true;
}

boolean connectMqttOptions(){
  
    boolean result;
    mqttClient.setWill(mqttStatusTopicValue, mqttLwtMessageValue, true, 0);
    if (mqttUserPasswordValue[0] != '\0')
    {
      result = mqttClient.connect(mqttClientIDValue, mqttUserNameValue, mqttUserPasswordValue);
    }
    else if (mqttUserNameValue[0] != '\0')
    {
      result = mqttClient.connect(mqttClientIDValue, mqttUserNameValue);
    }
    else
    {
      result = mqttClient.connect(mqttClientIDValue);
    }

    /*
    Serial.println((String)"mqttClientID: " + mqttClientIDValue);
    Serial.println((String)"mqttUserNameValue: " + mqttUserNameValue);
    Serial.println((String)"mqttUserPasswordValue: " + mqttUserPasswordValue);
    */
    return result;
}


boolean connectMqttRConf() {
  if (atoi(enableRConfigValue) == 1) {
  
        unsigned long aHora = millis();
        if ((lastRConfMqttConnectionAttempt + 1000) > aHora)
        {
          // Do not repeat within 1 sec.
          return false;
        }
        /* --------------- SerialDebug: --------- */
        Serial.println("Connecting to RConf MQTT server...");
        /* --------------- SerialDebug: --------- */                  

                 
        boolean result;
        mqttClientRConf.setWill(mqttStatusTopicValue, mqttLwtMessageValue, true, 0);
        result = mqttClientRConf.connect(mqttRCClientIDValue, remoteConfigMqttUserValue, remoteConfigMqttPwdValue);
        if (!result) {
          lastRConfMqttConnectionAttempt = aHora;
          return false;
        }

        /* --------------- SerialDebug: --------- */
        Serial.println("Connected RConf MQTT server!");
        /* --------------- SerialDebug: --------- */

        if (atoi(forceSecureAllTrafficValue) != 1){
          if (dsc.keybusConnected) {
            mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
            lastSentStatus = String(mqttBirthMessageValue);

            /* --------------- SerialDebug: --------- */
            Serial.println("Status message published: " + lastSentStatus);
            /* --------------- SerialDebug: --------- */
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
          }
          if (!dsc.keybusConnected) {
            mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
            lastSentStatus = String(mqttNoDSCValue);

            /* --------------- SerialDebug: --------- */
            Serial.println("Status message published: " + lastSentStatus);
            /* --------------- SerialDebug: --------- */
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
          }
        }

        
        if (atoi(forceSecureAllTrafficValue) == 1 || atoi(enableRConfigValue) == 1){
          if (dsc.keybusConnected) {
            mqttClientRConf.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
            lastSentStatus = String(mqttBirthMessageValue);

            /* --------------- SerialDebug: --------- */
            Serial.println("Status message published: " + lastSentStatus);
            /* --------------- SerialDebug: --------- */
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
          }
          if (!dsc.keybusConnected) {
            mqttClientRConf.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
            lastSentStatus = String(mqttNoDSCValue);

            /* --------------- SerialDebug: --------- */
            Serial.println("Status message published: " + lastSentStatus);
            /* --------------- SerialDebug: --------- */
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
          }
        }
        
        
      //Subcribe al topic de configuracion remota
        mqttClientRConf.subscribe(remoteConfigTopicValue);

        /* --------------- SerialDebug: --------- */
        Serial.println((String) "Subcribed to: " + remoteConfigTopicValue);
        /* --------------- SerialDebug: --------- */
        /* --------------- mqttDebug: --------- */
        if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Subcribed to: " + remoteConfigTopicValue), (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue);}
        /* --------------- mqttDebug: --------- */                      

      //Env�a mensaje de HELLO a RConfig
      //String helloMsg = "{\"deviceId\":\"" + String(deviceIdValue) + "\", \"fwVer\":\"" + String(CONFIG_VERSION) + "\", \"RCTopic\":\"" + String(remoteConfigTopicValue) + "\", \"RCRTopic\":\"" + String(remoteConfigResultTopicValue) + "\", \"notice\":\"HELLO\"}";
      // La linea de arriba esta comentada porque el mensaje al tener los topicos era muy largo y pinchaba... La reemplac� por la de abajo sin los topicos de RM.
      String helloMsg = "{\"deviceId\":\"" + String(deviceIdValue) + "\", \"fwVer\":\"" + String(CONFIG_VERSION) + "\", \"notice\":\"HELLO\"}";
       
      mqttClientRConf.publish(remoteConfigResultTopicValue, helloMsg, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Message sent. Topic: " + remoteConfigResultTopicValue + " Payload: " + helloMsg );
      /* --------------- SerialDebug: --------- */    
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Message sent: Topic: " + remoteConfigResultTopicValue + " Payload: " + helloMsg, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */


      //Subcribe al topic de commandos y de Status
      if(atoi(forceSecureAllTrafficValue) == 1){
            mqttClientRConf.subscribe(mqttCommandTopicValue);
              /* --------------- SerialDebug: --------- */
              Serial.println((String) "Subcribed to: " + mqttCommandTopicValue);
              /* --------------- SerialDebug: --------- */
              /* --------------- mqttDebug: --------- */
              if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Subcribed to: " + mqttCommandTopicValue, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
              /* --------------- mqttDebug: --------- */

      }  
       
       return true;
  }
}


void mqttMessageReceived(String &topic, String &payload){
/* --------------- SerialDebug: --------- */
Serial.println("Message received: " + topic + " - " + payload);
/* --------------- SerialDebug: --------- */
/* --------------- mqttDebug: --------- */
if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Message received: Topic: " + topic + " Payload: " + payload, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
/* --------------- mqttDebug: --------- */
   
// Actualiza con el �ltimo estado publicado
if (topic == mqttStatusTopicValue) lastSentStatus = payload;

if (topic == mqttCommandTopicValue){

String partitionN = payload.substring(0,1);
String acTion = payload.substring(1);    

Serial.println("partitionN: " + partitionN);
Serial.println("acTion: " + acTion);


  if (partitionN >= "0" && partitionN <= "8" && (acTion == "S" || acTion == "A" || acTion == "D" ) ){
  /* --------------- mqttDebug: --------- */
  //if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Inside IF - Decode: Partition: " + partitionN + " Action: " + acTion, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  /* --------------- mqttDebug: --------- */
  
Serial.println("entro en el IF");
 
            // Arm stay  REEMPLAZAR ESTO POR controlDSC("arm_stay",partitionN.toInt());
            if (acTion == "S" && !dsc.armed[partitionN.toInt()-1] && !dsc.exitDelay[partitionN.toInt()-1]) {
              while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              //dsc.writePartition = partitionN.toInt() + 1;         // Sets writes to the partition number
              dsc.writePartition = partitionN.toInt();         // Sets writes to the partition number
              dsc.write('s');                             // Virtual keypad arm stay
            
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " ARM_STAY called", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Arm away REEMPLAZAR ESTO POR controlDSC("arm_away",partitionN.toInt());
            else if (acTion == "A" && !dsc.armed[partitionN.toInt()-1] && !dsc.exitDelay[partitionN.toInt()-1]) {
              while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              //dsc.writePartition = partitionN.toInt() + 1;         // Sets writes to the partition number
              dsc.writePartition = partitionN.toInt();         // Sets writes to the partition number
              dsc.write('w');                             // Virtual keypad arm away

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " ARM_AWAY called", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Disarm  REEMPLAZAR ESTO POR controlDSC("disarm",partitionN.toInt());
            else if (acTion == "D" && (dsc.armed[partitionN.toInt()-1] || dsc.exitDelay[partitionN.toInt()-1])) {
              while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              //dsc.writePartition = partitionN.toInt() + 1;         // Sets writes to the partition number
              dsc.writePartition = partitionN.toInt();         // Sets writes to the partition number
              dsc.write(accessCodeValue);

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Access code sent to DISARM", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            }
            
          }else {    //Trarammiento no documentado de PINES 1(14) y 2(13)
                
                if (payload == "P2H"){   //Pone el Pin2 en High
                  digitalWrite(13,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 in now HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin2 in now HIGH.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (payload == "P2H1s"){ //Pone el Pin2 en High y despu�s de un segundo lo pone en LOW
                  digitalWrite(13,HIGH);
                  delay(1000);
                  digitalWrite(13,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 was put to HIGH and now is LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin2 was put to HIGH and now is LOW.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }
                
                if (payload == "P2L"){ //Pone el Pin2 en LOW
                  digitalWrite(13,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 in now LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin2 in now LOW.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (payload == "P2L1s"){ //Pone el Pin2 en LOW y despu�s de un segundo lo pone en HIGH
                  digitalWrite(13,LOW);
                  delay(1000);
                  digitalWrite(13,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 was put to LOW and now is HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin2 was put to LOW and now is HIGH.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }
                
                if (payload == "P1H"){  //Pone el Pin1 en HIGH
                  digitalWrite(14,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 in now HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin1 in now HIGH.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (payload == "P1H1s"){ //Pone el Pin1 en HIGH y despu�s de un segundo lo pone en LOW
                  digitalWrite(14,HIGH);
                  delay(1000);
                  digitalWrite(14,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 was put to HIGH and now is LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin1 was put to HIGH and now is LOW", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }
                
                if (payload == "P1L"){ //Pone el Pin1 en LOW
                  digitalWrite(14,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 in now LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin1 in now LOW.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (payload == "P1L1s"){ //Pone el Pin1 en LOW y despu�s de un segundo lo pone en HIGH
                  digitalWrite(14,LOW);
                  delay(1000);
                  digitalWrite(14,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 was put to LOW and now is HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin1 was put to LOW and now is HIGH", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }

            
            if (payload != "P2H" && payload != "P2L" && payload != "P1H" && payload != "P1L") dsc.write(payload.c_str());  //Chequear por que no funciona con panic y con auxiliar
            
            
          }
          
   }
} 


void publicaEstados(){

// Esto es para el keepAlive y para los accesorios, ademas tiene el publish timer string

      
      if(atoi(forceSecureAllTrafficValue) != 1){
        if(mqttClient.connected()){
        
            // PUBLISH TIMER STRING This needs to be improved being moved to Arduino Json
            if (String(tiMerStringValue).length() > 0 && atoi(publishTimerStringValue) == 1){
              mqttClient.publish(mqttKeepAliveTopicValue, "{\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}" , (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
              
              /* --------------- mqttDebug: --------- */
              if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Data published on: " + mqttServerValue + " Topic: " + mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}", false, atoi(remoteConfigQoSValue));}
              /* --------------- mqttDebug: --------- */
              /* --------------- SerialDebug: --------- */
                Serial.println("Publishing data...");
                Serial.println((String) "Message sent. Topic: " + mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}" );
              /* --------------- SerialDebug: --------- */
            }
            // end PUBLISH TIMER STRING
    
            mqttClient.publish(mqttKeepAliveTopicValue, (String) "{\"deviceID\":\"" + deviceIdValue + "\",\"dateTime\":\"" + getTimeString().substring(1) + "\",\"DSC\":" + dsc.keybusConnected + ",\"MQTT\":" + mqttClient.connected() + ",\"MQTTRM\":" + mqttClientRConf.connected() + ",\"dBm\":" + String(WiFi.RSSI()) + "}", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); 
        
        } else {
              /* --------------- mqttDebug: --------- */
              if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - FAIL: Data cound not be published because not connected to broker: " + mqttServerValue, false, atoi(remoteConfigQoSValue));}
              /* --------------- mqttDebug: --------- */
              /* --------------- SerialDebug: --------- */
                Serial.println("Publishing data...");
                Serial.println((String) "FAIL: Data cound not be published because not connected to broker" );
              /* --------------- SerialDebug: --------- */
            }
      }
      


      
      if(atoi(forceSecureAllTrafficValue) == 1){
          if(mqttClientRConf.connected()){
          
              // PUBLISH TIMER STRING This needs to be improved being moved to Arduino Json
              if (String(tiMerStringValue).length() > 0 && atoi(publishTimerStringValue) == 1){
                mqttClientRConf.publish(mqttKeepAliveTopicValue, "{\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}" , (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                
                /* --------------- mqttDebug: --------- */
                if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Data published on: " + mqttServerValue + " Topic: " + mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}", false, atoi(remoteConfigQoSValue));}
                /* --------------- mqttDebug: --------- */
                /* --------------- SerialDebug: --------- */
                  Serial.println("Publishing data...");
                  Serial.println((String) "Message sent. Topic: " + mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}" );
                /* --------------- SerialDebug: --------- */
              }
              // end PUBLISH TIMER STRING
              
          mqttClientRConf.publish(mqttKeepAliveTopicValue, (String) "{\"deviceID\":\"" + deviceIdValue + "\",\"dateTime\":\"" + getTimeString().substring(1) + "\",\"DSC\":" + dsc.keybusConnected + ",\"MQTT\":" + mqttClient.connected() + ",\"MQTTRM\":" + mqttClientRConf.connected() + ",\"dBm\":" + String(WiFi.RSSI()) + "}", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); 
          } else {
                /* --------------- mqttDebug: --------- */
                if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - FAIL: Data cound not be published because not connected to broker: " + remoteConfigMqttServerValue, false, atoi(remoteConfigQoSValue));}
                /* --------------- mqttDebug: --------- */
                /* --------------- SerialDebug: --------- */
                  Serial.println("Publishing data...");
                  Serial.println((String) "FAIL: Data cound not be published because not connected to broker" );
                /* --------------- SerialDebug: --------- */
              }
      }

}



void doDSC(){
  
            
            if (dsc.handlePanel() && dsc.statusChanged) {  // Processes data only when a valid Keybus command has been read
                dsc.statusChanged = false;                   // Reset the status tracking flag
            
                // If the Keybus data buffer is exceeded, the sketch is too busy to process all Keybus commands.  Call
                // handlePanel() more often, or increase dscBufferSize in the library: src/dscKeybusInterface.h
                if (dsc.bufferOverflow) Serial.println(F("Keybus buffer overflow"));
                dsc.bufferOverflow = false;


                // Checks if the interface is connected to the Keybus
                if (dsc.keybusChanged) {
                  dsc.keybusChanged = false;  // Resets the Keybus data status flag

                  if (atoi(forceSecureAllTrafficValue) != 1){
                    if (dsc.keybusConnected) {
                      mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
                      lastSentStatus = String(mqttBirthMessageValue);
                    }
                    else {
                      mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
                      lastSentStatus = String(mqttNoDSCValue);
                    }
                  }
                  
                  if (atoi(forceSecureAllTrafficValue) == 1){
                    if (dsc.keybusConnected) {
                      mqttClientRConf.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
                      lastSentStatus = String(mqttBirthMessageValue);
                    }
                    else {
                      mqttClientRConf.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
                      lastSentStatus = String(mqttNoDSCValue);
                    }
                  }
                  
                }
                
            
                // Sends the access code when needed by the panel for arming
                if (dsc.accessCodePrompt && dsc.writeReady) {
                  dsc.accessCodePrompt = false;
                  dsc.write(accessCodeValue);
                }
            
                if (dsc.troubleChanged) {
                  dsc.troubleChanged = false;  // Resets the trouble status flag
            
                  if (atoi(forceSecureAllTrafficValue) != 1){
                    if (dsc.trouble) mqttClient.publish(mqttTroubleTopicValue, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                    else mqttClient.publish(mqttTroubleTopicValue, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                  }
                  if (atoi(forceSecureAllTrafficValue) == 1){
                    if (dsc.trouble) mqttClientRConf.publish(mqttTroubleTopicValue, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                    else mqttClientRConf.publish(mqttTroubleTopicValue, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                  }
                  
                  //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
      
                        if (dsc.trouble) mqttClientRConf.publish((String) monitoringTopicValue + "/Trouble", "1", true, atoi(mqttQoSValue));
                          else mqttClientRConf.publish((String) monitoringTopicValue + "/Trouble", "0", true, atoi(mqttQoSValue));
      
      
                        /* --------------- SerialDebug: --------- */
                        Serial.println("MONITORING: online Status published");
                        /* --------------- SerialDebug: --------- */
                        /* --------------- mqttDebug: --------- */
                        if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: Trouble status published", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                        /* --------------- mqttDebug: --------- */
                        }
                  // END Monitoring
                
                }
            
                // Publishes status per partition
                for (byte partition = 0; partition < dscPartitions; partition++) {
            
                  // Publishes exit delay status
                  if (dsc.exitDelayChanged[partition]) {
                    dsc.exitDelayChanged[partition] = false;  // Resets the exit delay status flag
            
                    // Appends the mqttPartitionTopic with the partition number
                    char publishTopic[strlen(mqttPartitionTopicValue) + 1];
                    char partitionNumber[2];
                    strcpy(publishTopic, mqttPartitionTopicValue);
                    itoa(partition + 1, partitionNumber, 10);
                    strcat(publishTopic, partitionNumber);
            
                    if (atoi(forceSecureAllTrafficValue) != 1){
                      if (dsc.exitDelay[partition]) mqttClient.publish(publishTopic, "pending", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); // Publish as a retained message
                      else if (!dsc.exitDelay[partition] && !dsc.armed[partition]) mqttClient.publish(publishTopic, "disarmed", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                    }
                    if (atoi(forceSecureAllTrafficValue) == 1){
                      if (dsc.exitDelay[partition]) mqttClientRConf.publish(publishTopic, "pending", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); // Publish as a retained message
                      else if (!dsc.exitDelay[partition] && !dsc.armed[partition]) mqttClientRConf.publish(publishTopic, "disarmed", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                   }

                   //Monitoring
                        if (atoi(enableMonitoringValue) >1) {
                        if (dsc.exitDelay[partition]){ mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "pending", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: PENDING Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: PENDING status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }
                          else if (!dsc.exitDelay[partition] && !dsc.armed[partition]){ mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "disarmed", true, atoi(mqttQoSValue));
                              /* --------------- SerialDebug: --------- */
                              Serial.println((String)"MONITORING: DISARMED Status published for partition " + partitionNumber);
                              /* --------------- SerialDebug: --------- */
                              /* --------------- mqttDebug: --------- */
                              if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: DISARMED status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                              /* --------------- mqttDebug: --------- */ 
                          }
                        }
                        if (atoi(enableMonitoringValue) == 1) {mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "normal", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: NORMAL status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }
                  // END Monitoring
                  }
            
                  // Publishes armed/disarmed status
                  if (dsc.armedChanged[partition]) {
                    dsc.armedChanged[partition] = false;  // Resets the partition armed status flag
            
                    // Appends the mqttPartitionTopic with the partition number
                    char publishTopic[strlen(mqttPartitionTopicValue) + 1];
                    char partitionNumber[2];
                    strcpy(publishTopic, mqttPartitionTopicValue);
                    itoa(partition + 1, partitionNumber, 10);
                    strcat(publishTopic, partitionNumber);
            
                    if (dsc.armed[partition]) {
            
                      if (atoi(forceSecureAllTrafficValue) != 1) {
                        if (dsc.armedAway[partition]) mqttClient.publish(publishTopic, "armed_away", true, atoi(mqttQoSValue));
                        else if (dsc.armedStay[partition]) mqttClient.publish(publishTopic, "armed_home", true, atoi(mqttQoSValue));
                      }
                      if (atoi(forceSecureAllTrafficValue) == 1) {
                        if (dsc.armedAway[partition]) mqttClientRConf.publish(publishTopic, "armed_away", true, atoi(mqttQoSValue));
                        else if (dsc.armedStay[partition]) mqttClientRConf.publish(publishTopic, "armed_home", true, atoi(mqttQoSValue));
                      }
                    
                    
                    //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
                        if (dsc.armedAway[partition]){ mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "armed_away", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: ARMED_AWAY Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: ARMED_AWAY status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }
                          else if (dsc.armedStay[partition]){ mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "armed_home", true, atoi(mqttQoSValue));
                              /* --------------- SerialDebug: --------- */
                              Serial.println((String)"MONITORING: ARMED_HOME Status published for partition " + partitionNumber);
                              /* --------------- SerialDebug: --------- */
                              /* --------------- mqttDebug: --------- */
                              if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: ARMED_HOME status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                              /* --------------- mqttDebug: --------- */ 
                          }
                        }

                        if (atoi(enableMonitoringValue) == 1) {mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "normal", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: NORMAL status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }

                        
                    }
                    else {
                                if (atoi(forceSecureAllTrafficValue) != 1) {
                                    mqttClient.publish(publishTopic, "disarmed", true, atoi(mqttQoSValue));
                                }
                                if (atoi(forceSecureAllTrafficValue) == 1) {
                                    mqttClientRConf.publish(publishTopic, "disarmed", true, atoi(mqttQoSValue));
                                }
                    
                    
                    
                    //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
                          mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "disarmed", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: DISARMED Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: DISARMED status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }

                        if (atoi(enableMonitoringValue) == 1) {mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "normal", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: NORMAL status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }
                    // END Monitoring
                    }
                  }
            
                  // Publishes alarm status
                  if (dsc.alarmChanged[partition]) {
                    dsc.alarmChanged[partition] = false;  // Resets the partition alarm status flag
                    if (dsc.alarm[partition]) {
            
                      // Appends the mqttPartitionTopic with the partition number
                      char publishTopic[strlen(mqttPartitionTopicValue) + 1];
                      char partitionNumber[2];
                      strcpy(publishTopic, mqttPartitionTopicValue);
                      itoa(partition + 1, partitionNumber, 10);
                      strcat(publishTopic, partitionNumber);
            
                      if (atoi(forceSecureAllTrafficValue) != 1){
                        mqttClient.publish(publishTopic, "triggered", true, atoi(mqttQoSValue)); // Alarm tripped
                      }
                      if (atoi(forceSecureAllTrafficValue) == 1){
                        mqttClientRConf.publish(publishTopic, "triggered", true, atoi(mqttQoSValue)); // Alarm tripped
                      }


                      //Monitoring
                        if (atoi(enableMonitoringValue) > 0) {
                          mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "triggered", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: TRIGGERED Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: TRIGGERED status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }
                      // END Monitoring

                      
                    }
                  }
            
                  // Publishes fire alarm status
                  if (dsc.fireChanged[partition]) {
                    dsc.fireChanged[partition] = false;  // Resets the fire status flag
            
                    // Appends the mqttFireTopic with the partition number
                    char firePublishTopic[strlen(mqttFireTopicValue) + 1];
                    char partitionNumber[2];
                    strcpy(firePublishTopic, mqttFireTopicValue);
                    itoa(partition + 1, partitionNumber, 10);
                    strcat(firePublishTopic, partitionNumber);
            
            
                    if (atoi(forceSecureAllTrafficValue) != 1){
                        if (dsc.fire[partition]) mqttClient.publish(firePublishTopic, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));  // Fire alarm tripped
                        else mqttClient.publish(firePublishTopic, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));                      // Fire alarm restored
                    }
                    if (atoi(forceSecureAllTrafficValue) == 1){
                        if (dsc.fire[partition]) mqttClientRConf.publish(firePublishTopic, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));  // Fire alarm tripped
                        else mqttClientRConf.publish(firePublishTopic, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));                      // Fire alarm restored
                    }


                    //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
      
                        if (dsc.fire[partition]) mqttClientRConf.publish((String) monitoringTopicValue + "/Fire" + partitionNumber, "1", true, atoi(mqttQoSValue));
                          else mqttClientRConf.publish((String) monitoringTopicValue + "/Fire" + partitionNumber, "0", true, atoi(mqttQoSValue));
      
      
                        /* --------------- SerialDebug: --------- */
                        Serial.println((String)"MONITORING: FIRE Status published for partition " + partitionNumber);
                        /* --------------- SerialDebug: --------- */
                        /* --------------- mqttDebug: --------- */
                        if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: FIRE status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                        /* --------------- mqttDebug: --------- */
                        }
                  // END Monitoring


                    
                  }
                }
            
                // Publishes zones 1-64 status in a separate topic per zone
                // Zone status is stored in the openZones[] and openZonesChanged[] arrays using 1 bit per zone, up to 64 zones:
                //   openZones[0] and openZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
                //   openZones[1] and openZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
                //   ...
                //   openZones[7] and openZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
                if (dsc.openZonesStatusChanged) {
                  dsc.openZonesStatusChanged = false;                           // Resets the open zones status flag
                  for (byte zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
                    for (byte zoneBit = 0; zoneBit < 8; zoneBit++) {
                      if (bitRead(dsc.openZonesChanged[zoneGroup], zoneBit)) {  // Checks an individual open zone status flag
                        bitWrite(dsc.openZonesChanged[zoneGroup], zoneBit, 0);  // Resets the individual open zone status flag
            
                        // Appends the mqttZoneTopic with the zone number
                        char zonePublishTopic[strlen(mqttZoneTopicValue) + 2];
                        char zone[3];
                        strcpy(zonePublishTopic, mqttZoneTopicValue);
                        itoa(zoneBit + 1 + (zoneGroup * 8), zone, 10);
                        strcat(zonePublishTopic, zone);
            
                        if (bitRead(dsc.openZones[zoneGroup], zoneBit)) {
                          if (atoi(forceSecureAllTrafficValue) != 1){
                            mqttClient.publish(zonePublishTopic, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));            // Zone open
                          }
                          if (atoi(forceSecureAllTrafficValue) == 1){
                            mqttClientRConf.publish(zonePublishTopic, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));            // Zone open
                          }

                          //Monitoring
                                if (atoi(enableMonitoringValue) == 3) {
                                  mqttClientRConf.publish((String) monitoringTopicValue + "/Zone" + zone, "1", true, atoi(mqttQoSValue));
                                  /* --------------- SerialDebug: --------- */
                                  Serial.println((String)"MONITORING: ACTIVE status published for zone " + zone);
                                  /* --------------- SerialDebug: --------- */
                                  /* --------------- mqttDebug: --------- */
                                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: ACTIVE status published for zone " + zone, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                                  /* --------------- mqttDebug: --------- */
                                }
                          // END Monitoring
                          
                        }
                        else {
                          if (atoi(forceSecureAllTrafficValue) != 1){
                            mqttClient.publish(zonePublishTopic, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));         // Zone closed
                          }
                          if (atoi(forceSecureAllTrafficValue) == 1){
                            mqttClientRConf.publish(zonePublishTopic, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));         // Zone closed
                          }

                          //Monitoring
                                if (atoi(enableMonitoringValue) == 3) {
                                  mqttClientRConf.publish((String) monitoringTopicValue + "/Zone" + zone, "0", true, atoi(mqttQoSValue));
                                  /* --------------- SerialDebug: --------- */
                                  Serial.println((String)"MONITORING: INACTIVE status published for zone " + zone);
                                  /* --------------- SerialDebug: --------- */
                                  /* --------------- mqttDebug: --------- */
                                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: INACTIVE status published for zone " + zone, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                                  /* --------------- mqttDebug: --------- */
                                }
                          // END Monitoring
                          
                        }
                      }
                    }
                  }
                }
            
            //    mqtt.subscribe(mqttCommandTopicValue);
              }


}


void controlDSC(String coMMand, int targetPartition){
            
            // Arm stay
            if (coMMand == "arm_stay" && !dsc.armed[targetPartition-1] && !dsc.exitDelay[targetPartition-1]) {
              while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              dsc.writePartition = targetPartition;         // Sets writes to the partition number
              dsc.write('s');                             // Virtual keypad arm stay
            
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " ARM_STAY called", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Arm away
            else if (coMMand == "arm_away" && !dsc.armed[targetPartition-1] && !dsc.exitDelay[targetPartition-1]) {
              while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              dsc.writePartition = targetPartition;         // Sets writes to the partition number
              dsc.write('w');                             // Virtual keypad arm away

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " ARM_AWAY called", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Disarm
            else if (coMMand == "disarm" && (dsc.armed[targetPartition-1] || dsc.exitDelay[targetPartition-1])) {
              while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              dsc.writePartition = targetPartition;         // Sets writes to the partition number
              dsc.write(accessCodeValue);

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Access Code sent to DISARM", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            }
}





///NTP

String getTimeString() {
  String timeString = String(weekday()) + String(year());
  if (month()<10){timeString = timeString + "0" + month();} else {timeString = timeString + month();}
  if (day()<10){timeString = timeString + "0" + day();} else {timeString = timeString + day();}
  if (hour()<10){timeString = timeString + "0" + hour();} else {timeString = timeString + hour();}
  if (minute()<10){timeString = timeString + "0" + minute();} else {timeString = timeString + minute();}
  if (second()<10){timeString = timeString + "0" + second();} else {timeString = timeString + second();}
  return timeString;
}

String getReadableTime() {

  String ReadableTime;

  if (weekday() == 1) {ReadableTime = "Sunday, " + String(year());}
  if (weekday() == 2) {ReadableTime = "Monday, " + String(year());}
  if (weekday() == 3) {ReadableTime = "Tuesday, " + String(year());}
  if (weekday() == 4) {ReadableTime = "Wednesday, " + String(year());}
  if (weekday() == 5) {ReadableTime = "Thursday, " + String(year());}
  if (weekday() == 6) {ReadableTime = "Friday, " + String(year());}
  if (weekday() == 7) {ReadableTime = "Saturday, " + String(year());}
  
  if (month() == 1){ReadableTime = ReadableTime + " January ";}
  if (month() == 2){ReadableTime = ReadableTime + " February ";}
  if (month() == 3){ReadableTime = ReadableTime + " March ";}
  if (month() == 4){ReadableTime = ReadableTime + " April ";}
  if (month() == 5){ReadableTime = ReadableTime + " May ";}
  if (month() == 6){ReadableTime = ReadableTime + " June ";}
  if (month() == 7){ReadableTime = ReadableTime + " July ";}
  if (month() == 8){ReadableTime = ReadableTime + " August ";}
  if (month() == 9){ReadableTime = ReadableTime + " September ";}
  if (month() == 10){ReadableTime = ReadableTime + " October ";}
  if (month() == 11){ReadableTime = ReadableTime + " November ";}
  if (month() == 12){ReadableTime = ReadableTime + " December ";}
      
  ReadableTime = ReadableTime + day() + " - ";
  if (hour()<10){ReadableTime = ReadableTime + "0" + hour();} else {ReadableTime = ReadableTime + hour();}
  if (minute()<10){ReadableTime = ReadableTime + ":0" + minute();} else {ReadableTime = ReadableTime + ":" + minute();}
  if (second()<10){ReadableTime = ReadableTime + ":0" + second();} else {ReadableTime = ReadableTime + ":" + second();}
  
  return ReadableTime;
}
    
/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime(){
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
    /* --------------- SerialDebug: --------- */
  //Serial.println((String)"Sending NTP request to " + ntpServerValue + " (" + ntpServerIP + ")");
  Serial.println((String)"Sending NTP request to " + ntpServerValue);
  /* --------------- SerialDebug: --------- */
  /* --------------- mqttDebug: --------- */
  //if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Sending NTP request to " + ntpServerValue + " (" + ntpServerIP + ")", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Sending NTP request to " + ntpServerValue, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  /* --------------- mqttDebug: --------- */
  // get a random server from the pool
  WiFi.hostByName(ntpServerValue, ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      /* --------------- SerialDebug: --------- */
      Serial.println("NTP response received");
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - NTP response received", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + atoi(timeZoneValue) * SECS_PER_HOUR;
    }
  }
  //Serial.println("No NTP Response :-(");
  /* --------------- SerialDebug: --------- */
  Serial.println("No response from NTP server");
  /* --------------- SerialDebug: --------- */
  /* --------------- mqttDebug: --------- */
  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - No response from NTP server", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  /* --------------- mqttDebug: --------- */
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address){
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
 ///// Fin NTP

//Timer
 void runTimer() {

String tmrStr = String(tiMerStringValue);

  if(tmrStr.length()%7 == 0) {
      int eventos = tmrStr.length()/7;
    
      if (timeStatus() != timeNotSet) {
      
            for (int i = 0; i < eventos; i++) {
                String timerEventString = tmrStr.substring(i*7,(i*7)+7);
            
                // Aca va la magia de cada evento
                
    
    /*
    Domingo = 1 = G, H, J, P
    Lunes = 2 = A, H, I, K, L, N, P
    Martes = 3 = B, H, I, K, M, N
    Miercoles = 4 = C, H, I, K, L, N, P
    Jueves = 5 = D, H, I, K, M, O
    Viernes = 6 = E, H, I, K, L, O, P
    Sabado = 7 = F, H, J, K, M, O
    
    getTimeString().substring(9,13) == timerEventString.substring(1,3) + timerEventString.substring(3,5)
    
    Accion: timerEventString.substring(5,6)
    switch: timerEventString.substring(6)
     */
    
    
                  if (
                      (
                      (String(weekday()) == "1" && (                      //Domingo = 1 = G, H, J, P
                      timerEventString.substring(0,1) == "G" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "J" ||
                      timerEventString.substring(0,1) == "P" ))
                      ||
                      (String(weekday()) == "2" && (                      //Lunes = 2 = A, H, I, K, L, N, P
                      timerEventString.substring(0,1) == "A" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "I" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "L" ||
                      timerEventString.substring(0,1) == "N" ||
                      timerEventString.substring(0,1) == "P" ))
                      ||
                      (String(weekday()) == "3" && (                      //Martes = 3 = B, H, I, K, M, N
                      timerEventString.substring(0,1) == "B" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "I" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "M" ||
                      timerEventString.substring(0,1) == "N" ))
                      ||
                      (String(weekday()) == "4" && (                      //Miercoles = 4 = C, H, I, K, L, N, P
                      timerEventString.substring(0,1) == "C" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "I" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "L" ||
                      timerEventString.substring(0,1) == "N" ||
                      timerEventString.substring(0,1) == "P" ))
                      ||
                      (String(weekday()) == "5" && (                      //Jueves = 5 = D, H, I, K, M, O
                      timerEventString.substring(0,1) == "D" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "I" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "M" ||
                      timerEventString.substring(0,1) == "O" ))
                      ||
                      (String(weekday()) == "6" && (                      //Viernes = 6 = E, H, I, K, L, O, P
                      timerEventString.substring(0,1) == "E" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "I" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "L" ||
                      timerEventString.substring(0,1) == "O" ||
                      timerEventString.substring(0,1) == "P" ))
                      ||
                      (String(weekday()) == "7" && (                      //Sabado = 7 = F, H, J, K, M, O
                      timerEventString.substring(0,1) == "F" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "J" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "M" ||
                      timerEventString.substring(0,1) == "O" ))
                      )
                      &&
                      (getTimeString().substring(9,13) == timerEventString.substring(1,3) + timerEventString.substring(3,5))
                      &&
                      timerEventDone[i] == 0
                    )
                    {
                      /* --------------- SerialDebug: --------- */
                      Serial.println("Executing Timer event: " + timerEventString);
                      /* --------------- SerialDebug: --------- */
                      /* --------------- mqttDebug: --------- */
                      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Executing Timer event: " + timerEventString, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                      /* --------------- mqttDebug: --------- */

                      /////////// Magic! 

                      // La particion esta en el timerEventString.substring(6)
                              
                      if (timerEventString.substring(5,6) == "1"){  //Action Armar Stay
                        //Armar Stay
                        controlDSC("arm_stay",timerEventString.substring(6).toInt()); 
                      }

                      if (timerEventString.substring(5,6) == "2"){  //Action Armar Away
                        //Armar Away
                        controlDSC("arm_away",timerEventString.substring(6).toInt()); 
                      }
                      
                      if (timerEventString.substring(5,6) == "0"){  //Action desarmar
                        //desarmar
                        controlDSC("disarm",timerEventString.substring(6).toInt());
                      }
                      
                      ////////////// End Magic!
                      timerEventDone[i] = 1;
                      return;
                    }
                    else if (getTimeString().substring(9,13) != timerEventString.substring(1,3) + timerEventString.substring(3,5) && timerEventDone[i] == 1) { 

                      timerEventDone[i] = 0; }
            
            
            }
            }
      
      else {
          /* --------------- SerialDebug: --------- */
          Serial.println("Timer: No DateTime info from NTP yet. Nothing can be done!");
          /* --------------- SerialDebug: --------- */
          /* --------------- mqttDebug: --------- */
          if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Timer: No DateTime info from NTP yet. Nothing can be done!", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
          /* --------------- mqttDebug: --------- */
      }
    }
    if(tmrStr.length()%7 != 0 || tmrStr.length() == 0) {
      /* --------------- SerialDebug: --------- */
      Serial.println("ERROR: Wrong Timer String or not specified");
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - ERROR: Wrong Timer String or not specified", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */   
  }
  

}


 
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// Remote Managemet ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


void mqttRConfMessageReceived(String &topic, String &payload)
{

/* --------------- SerialDebug: --------- */
Serial.println("Message received: " + topic + " - " + payload);
/* --------------- SerialDebug: --------- */
/* --------------- mqttDebug: --------- */
if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Message received: Topic: " + topic + " Payload: " + payload, false, atoi(remoteConfigQoSValue));}
/* --------------- mqttDebug: --------- */


if (atoi(forceSecureAllTrafficValue) == 1 && topic != remoteConfigTopicValue) {
        /* --------------- SerialDebug: --------- */
        Serial.println("Non Remote Management MQTT message received on Remote Management connection. Forwarding...");
        /* --------------- SerialDebug: --------- */
        /* --------------- mqttDebug: --------- */
        if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + "Non Remote Management MQTT message received on Remote Management connection. Forwarding...", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
        /* --------------- mqttDebug: --------- */
  String topIco = topic;
  String payLoad = payload;
  mqttMessageReceived(topIco, payLoad);
  return;
}

 
// Interpreta comandos de configuraci�n remota
 if (topic == remoteConfigTopicValue){

    //Configuraci�n remota
    /* --------------- SerialDebug: --------- */
    Serial.println("Mensaje de configiracion remota recibido: " + payload);
    /* --------------- SerialDebug: --------- */
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote config message received: " + payload, false, atoi(remoteConfigQoSValue));}
    /* --------------- mqttDebug: --------- */

    String RConfCommandResult;
    int commandResponses = 0;
    
    StaticJsonDocument<500> jsonBuffer;  // estaba en 300 pero lo sub� a 500 porque no entraban los comments

    deserializeJson(jsonBuffer, payload);
        String RCpaSSwd = jsonBuffer["pwd"];
        String coMMand = jsonBuffer["command"];
        String paRam1 = jsonBuffer["param1"];
        String paRam2 = jsonBuffer["param2"];
    
/* --------------- mqttDebug: --------- */
if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - JSON payload pharsed: " + payload + " Command: " + coMMand, false, atoi(remoteConfigQoSValue));}
/* --------------- mqttDebug: --------- */    
    //JsonObject& rootOut = jsonBuffer.createObject();
    
    /*
     * Comandos para administraci�n remota.
     * El comando getAllParams deberia devolver un json con todos los parametros de configuracion. (pero no pude armar un solo json porque por el largo no entra en un mensahe mqtt)
     * 
     * 
     * Faltan comandos para actualizar los comando basicos: SSID del AP, Password del AP, WifiSsid, WifiPassword y el timeout del AP
     * 
     * 
     * 
     * 
     */


// Verifica Pwd para la administracion remota
 if (RCpaSSwd != String(RConfigPwdValue)) {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "FAIL: Wrong Password";
      jsonBuffer["comments"] = "Wrong Password";
      // jsonBuffer["comments"] = RConfigPwdValue;
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
  
  return;  // con esto deber�a salir sin hacer nada cuando el pwd no coincide
 }                       
    /*    
     *   El comando getConfigVersion devuelve el deviceID y la version del firmware del dispositivo
     */
    if (coMMand == "getConfigVersion") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(CONFIG_VERSION);

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Remote Config result message sent. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */                    
    }


  /*    
     *   El comando getIP devuelve el deviceID y la ip privada asignda en la red local, la mascara de la subnet y el default gateway
     */
    if (coMMand == "getIP") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["localIP"] = WiFi.localIP().toString();
      jsonBuffer["subNetMask"] = WiFi.subnetMask().toString();
      jsonBuffer["gateWay"] = WiFi.gatewayIP().toString();

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Remote Config result message sent. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }

     /*    
     *   El comando getEnableMqttDebug devuelve el deviceID y estado del debug por mqtt
     */
   
    if (coMMand == "getEnableMqttDebug") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(enableMqttDebugValue);
      /*
      JsonArray& data = rootOut.createNestedArray("data");
      data.add(48.756080);
      data.add(2.302038);
      */

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }
    
    /*    
     *   El comando updateEnableMqttDebug modifica el estado del debug por mqtt
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateEnableMqttDebug") {

      memset(enableMqttDebugValue, 0, sizeof enableMqttDebugValue);
      strncpy(enableMqttDebugValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Enable MQTT debug state is now: " + String(enableMqttDebugValue);

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
                      
    }


     /*    
     *   El comando getMqttDebugTopic devuelve el deviceID y topic donde se publican los mensajes de debug
     */
   
    if (coMMand == "getMqttDebugTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(MqttDebugTopicValue);

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }
    
    /*    
     *   El comando updateMqttDebugTopic modifica el topic donde se publican los mensajes de debug
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttDebugTopic") {

      memset(MqttDebugTopicValue, 0, sizeof MqttDebugTopicValue);
      strncpy(MqttDebugTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT debug topic is now: " + String(MqttDebugTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      
      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


      

    /*    
     *   El comando triggerSendStatus devuelve el deviceID y hace que se publiquen los estados en el topic de stado del/los switches
     */
     
    if (coMMand == "triggerSendStatus") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";

      publicaEstados();
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }



/*    
     *   El comando getMqttConnStatus Devuelve el deviceID y el estado de la conexion al servidor MQTT principal
     */
     
    if (coMMand == "getMqttConnStatus") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      

      if(mqttClient.connected()){
        jsonBuffer["response"] = "1";
      }
      else {
        jsonBuffer["response"] = "0";
      }
      
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
    }



    /*    
     *   El comando getMqttServer devuelve el deviceID y la url del servidor MQTT configurado
     */
   
    if (coMMand == "getMqttServer") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
    }


 
    /*    
     *   El comando updateMqttServer actualiza la url del servidor MQTT por la proporcionada en campo parametro.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttServer") {

      //char paRam1C[sizeof(paRam1)];
      //paRam1.toCharArray(paRam1C, sizeof(paRam1));
      //strncpy(mqttServerValue, paRam1C, sizeof(paRam1C));
      memset(mqttServerValue, 0, sizeof mqttServerValue);
      strncpy(mqttServerValue, paRam1.c_str(), String(paRam1).length());  

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT server is now: " + String(mqttServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
                      
    }


/*    
     *   El comando getUpdateInterval devuelve el deviceID y el intervalo el segundos entre publicacion de info
     */
     
    if (coMMand == "getUpdateInterval") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(updateIntervalValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }

     if (coMMand == "updateUpdateInterval") {

      memset(updateIntervalValue, 0, sizeof updateIntervalValue);
      strncpy(updateIntervalValue, paRam1.c_str(), String(paRam1).length());


      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Update Interval is now: " + String(updateIntervalValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
                      
    }


/*    
     *   El comando getMqttPort devuelve el deviceID y el puerto�para la conexion al servidor MQTT de la funcionalidad principal
     */
   
    if (coMMand == "getMqttPort") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttPortValue);

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttPort actualiza el puerto para la conexion al servidor MQTT de la funcionalidad principal por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttPort") {

      memset(mqttPortValue, 0, sizeof mqttPortValue);
      strncpy(mqttPortValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT Port is now: " + String(mqttPortValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
                
    }

  /*    
     *   El comando getMqttClientID devuelve el deviceID y el clientID con el que esta confgigurado el cliente MQTT tanto para la funcionalidad principal como para RConfig
     */
   
    if (coMMand == "getMqttClientID") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttClientIDValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttClientID actualiza el el clienteID del cliente MQTT tabto de la funcionalidad principal como de RCOnfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttClientID") {

      memset(mqttClientIDValue, 0, sizeof mqttClientIDValue);
      strncpy(mqttClientIDValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT ClientID is now: " + String(mqttClientIDValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
                      
    }


   /*    
     *   El comando getMqttUserName devuelve el nombre de usuario utilizado para conectarse al servidor MQTT de la funcionalidad principal
     */
   
    if (coMMand == "getMqttUserName") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttUserNameValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttUserName actualiza el nombre de usuario con el que se conecta al server MQTT de la funcionalidad principal por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttUserName") {

      memset(mqttUserNameValue, 0, sizeof mqttUserNameValue);
      strncpy(mqttUserNameValue, paRam1.c_str(), String(paRam1).length());


      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT User Name is now: " + String(mqttUserNameValue);
      

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


   /*    
     *   El comando getMqttUserPassword devuelve la contrase�a utilizada para conectarse al servidor MQTT de la funcionalidad principal
     */
   
    if (coMMand == "getMqttUserPassword") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttUserPasswordValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttUserPassword actualiza la contrase�a con la que se conecta al server MQTT de la funcionalidad principal por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttUserPassword") {

      memset(mqttUserPasswordValue, 0, sizeof mqttUserPasswordValue);
      strncpy(mqttUserPasswordValue, paRam1.c_str(), String(paRam1).length());


      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT User Password updated";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


    /*    
     *   El comando updateDeviceID actualiza el deviceID del dispositivo por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateDeviceID") {

      memset(deviceIdValue, 0, sizeof deviceIdValue);
      strncpy(deviceIdValue, paRam1.c_str(), String(paRam1).length());


      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Device ID is now: " + String(deviceIdValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }







    /*    
     *   El comando disableRConfig deshabilita el Remote Management. Ojo porque luego solo se puede habilitar con acceso f�sico al dispositivo.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "disableRConfig") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;

      if (paRam1 == "DISABLE" && paRam2 == "YES"){
        memset(enableRConfigValue, 0, sizeof enableRConfigValue);
        strncpy(enableRConfigValue, String("0").c_str(), String("0").length());
        jsonBuffer["response"] = "OK";
        jsonBuffer["comments"] = "Remote Management feature has beed disabled.";
      }
      else {
        jsonBuffer["response"] = "FAIL";
        jsonBuffer["comments"] = "Forbidden - Wrong parameters";
     }
  
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }



   /*    
     *   El comando getMqttRetain devuelve el valor para el parametro RETAIN de MQTT para la publicaci�n de mensajes de de la funcionalidad principal
     */
   
    if (coMMand == "getMqttRetain") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttRetainValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttRetain actualiza el valor para el parametro RETAIN de MQTT para la publicaci�n de mensajes de de la funcionalidad principal por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttRetain") {

      memset(mqttRetainValue, 0, sizeof mqttRetainValue);
      strncpy(mqttRetainValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT Retain parameter is now: " + String(mqttRetainValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


   /*    
     *   El comando getMqttQoS devuelve el valor para el parametro QoS de MQTT para la publicaci�n de mensajes de de la funcionalidad principal
     */
   
    if (coMMand == "getMqttQoS") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttQoSValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttQoS actualiza el valor para el parametro QoS de MQTT para la publicaci�n de mensajes de de la funcionalidad principal por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttQoS") {

      memset(mqttQoSValue, 0, sizeof mqttQoSValue);
      strncpy(mqttQoSValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT QoS parameter is now: " + String(mqttQoSValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

   /*    
     *   El comando getRemoteConfigMqttServer devuelve el servidor al que se conectara para RConfig 
     */
   
    if (coMMand == "getRemoteConfigMqttServer") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigMqttServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigMqttServer actualiza el servidor al que se conecta para RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigMqttServer") {

      memset(remoteConfigMqttServerValue, 0, sizeof remoteConfigMqttServerValue);
      strncpy(remoteConfigMqttServerValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config MQTT server is now: " + String(remoteConfigMqttServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

   /*    
     *   El comando getRemoteConfigMqttPort devuelve el puerto de conexion para RConfig 
     */
   
    if (coMMand == "getRemoteConfigMqttPort") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigMqttPortValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigMqttPort actualiza el puerto de conexion para RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigMqttPort") {

      memset(remoteConfigMqttPortValue, 0, sizeof remoteConfigMqttPortValue);
      strncpy(remoteConfigMqttPortValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config MQTT Port is now: " + String(remoteConfigMqttPortValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

   /*    
     *   El comando getRemoteConfigMqttUser devuelve el nombre de usuario para la conexion para RConfig 
     */
   
    if (coMMand == "getRemoteConfigMqttUser") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigMqttUserValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigMqttUser actualiza el nombre de usuario para la conexion para RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigMqttUser") {

      memset(remoteConfigMqttUserValue, 0, sizeof remoteConfigMqttUserValue);
      strncpy(remoteConfigMqttUserValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config MQTT Username is now: " + String(remoteConfigMqttUserValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


   /*    
     *   El comando getRemoteConfigMqttPwd devuelve la contrase�a para la conexion para RConfig 
     */
   
    if (coMMand == "getRemoteConfigMqttPwd") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigMqttPwdValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigMqttPwd actualiza la contrase�a para la conexion para RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigMqttPwd") {

      memset(remoteConfigMqttPwdValue, 0, sizeof remoteConfigMqttPwdValue);
      strncpy(remoteConfigMqttPwdValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config MQTT Password updated";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


   /*    
     *   El comando getForceSecureAllTraffic devuelve el estado del flag forceSecureAllTraffic 
     */
   
    if (coMMand == "getForceSecureAllTraffic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(forceSecureAllTrafficValue);

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateForceSecureAllTraffic actualiza el flag ForceSecureAllTraffic por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateForceSecureAllTraffic") {

      memset(forceSecureAllTrafficValue, 0, sizeof forceSecureAllTrafficValue);
      strncpy(forceSecureAllTrafficValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Force Secure All Traffic flag is now: " + String(forceSecureAllTrafficValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }




    /*    
     *   El comando updateRemoteConfigPwd actualiza la contrase�a para enviar comandos de RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigPwd") {

      memset(RConfigPwdValue, 0, sizeof RConfigPwdValue);
      strncpy(RConfigPwdValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config Password updated";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


   /*    
     *   El comando getRemoteConfigTopic devuelve el topic en el que el dispositivo espera los comandos de RConfig 
     */
   
    if (coMMand == "getRemoteConfigTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigTopic actualiza el topico en el que el dispositivo espera comandos de RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigTopic") {

      memset(remoteConfigTopicValue, 0, sizeof remoteConfigTopicValue);
      strncpy(remoteConfigTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config Topic is now: " + String(remoteConfigTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

   /*    
     *   El comando getRemoteConfigResultTopic devuelve el topic en el que el dispositivo publica los resultados de los comandos de RConfig 
     */
   
    if (coMMand == "getRemoteConfigResultTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigResultTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigResultTopic actualiza el topico en el que el dispositivo publica los resultados comandos de RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigResultTopic") {

      memset(remoteConfigResultTopicValue, 0, sizeof remoteConfigResultTopicValue);
      strncpy(remoteConfigResultTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config Result Topic is now: " + String(remoteConfigResultTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


      /*    
     *   El comando getRemoteConfigRetain devuelve el estado del parametro retain para los mensajes MQTT publicados. (0 = false, 1 = true)  
     */
   
    if (coMMand == "getRemoteConfigRetain") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigRetainValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigRetain actualiza el parametro retain por el proporcionado en campo parametro pram1. (0 = false, 1 = true)
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigRetain") {

      memset(remoteConfigRetainValue, 0, sizeof remoteConfigRetainValue);
      strncpy(remoteConfigRetainValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config Retain value in now: " + String(remoteConfigRetainValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

/*    
     *   El comando getRemoteConfigQoS devuelve el estado del parametro QoS para los mensajes MQTT de Remote Management publicados. (0, 1 o 2)  
     */
   
    if (coMMand == "getRemoteConfigQoS") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigQoSValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigQoS actualiza el parametro QoS por el proporcionado en campo parametro pram1. (0, 1 o 2)
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigQoS") {

      memset(remoteConfigQoSValue, 0, sizeof remoteConfigQoSValue);
      strncpy(remoteConfigQoSValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config QoS parameter is now: " + String(remoteConfigQoSValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


/*    
     *   El comando updateAccessCode actualiza el codigo para armar y desarmar la alarma por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateAccessCode") {

      memset(accessCodeValue, 0, sizeof accessCodeValue);
      strncpy(accessCodeValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Access Code updated";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getMqttStatusTopic devuelve el topic en el que el dispositivo publicar� el Status
     */
   
    if (coMMand == "getMqttStatusTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttStatusTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttStatusTopic actualiza el topico en el que el dispositivo publica el Status por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttStatusTopic") {

      memset(mqttStatusTopicValue, 0, sizeof mqttStatusTopicValue);
      strncpy(mqttStatusTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT Status Topic is now: " + String(mqttStatusTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getMqttBirthMessage devuelve el payload del mensaje de Status cuando establece conexi�n
     */
   
    if (coMMand == "getMqttBirthMessage") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttBirthMessageValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttBirthMessage actualiza el payload que el dispositivo publica en el topcico de Status cuando se conecta por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttBirthMessage") {

      memset(mqttBirthMessageValue, 0, sizeof mqttBirthMessageValue);
      strncpy(mqttBirthMessageValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt Birth Message is now: " + String(mqttBirthMessageValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getMqttLwtMessage devuelve el payload del mensaje de Last Will para cuando se pierde la conexi�n
     */
   
    if (coMMand == "getMqttLwtMessage") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttLwtMessageValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttLwtMessage actualiza el payload que el broker publica en el topcico de Status cuando se pierde la conexi�n por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttLwtMessage") {

      memset(mqttLwtMessageValue, 0, sizeof mqttLwtMessageValue);
      strncpy(mqttLwtMessageValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt LWT message is now: " + String(mqttLwtMessageValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getMqttNoDSC devuelve el payload del mensaje que el dipositivo publica cuando se pierde la conexi�n con el sistema de alarma
     */
   
    if (coMMand == "getMqttNoDSC") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttNoDSCValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttNoDSC actualiza el payload que el dispositivo publica en el topcico de Status cuando se pierde la conexi�n con el sistema de alarma por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttNoDSC") {

      memset(mqttNoDSCValue, 0, sizeof mqttNoDSCValue);
      strncpy(mqttNoDSCValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt NoDSC message is now: " + String(mqttNoDSCValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getMqttPartitionTopic devuelve el prefijo con el que el dispositivo crea los topicos de estado de las particiones
     */
   
    if (coMMand == "getMqttPartitionTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttPartitionTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttPartitionTopic actualiza el prefijo con el que el dispositivo crea los topicos de estado de las particiones por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttPartitionTopic") {

      memset(mqttPartitionTopicValue, 0, sizeof mqttPartitionTopicValue);
      strncpy(mqttPartitionTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt partition topic prefix is now: " + String(mqttPartitionTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }



/*    
     *   El comando getMqttPartitionTopic devuelve el prefijo con el que el dispositivo crea los topicos de estado de las particiones
     */
   
    if (coMMand == "getMqttActivePartitionTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttActivePartitionTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttPartitionTopic actualiza el prefijo con el que el dispositivo crea los topicos de estado de las particiones por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttActivePartitionTopic") {

      memset(mqttActivePartitionTopicValue, 0, sizeof mqttActivePartitionTopicValue);
      strncpy(mqttActivePartitionTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt Active Partition topic is now: " + String(mqttPartitionTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }



  /*    
     *   El comando getMqttZoneTopic devuelve el prefijo con el que el dispositivo crea los topicos de estado de las zonas
     */
   
    if (coMMand == "getMqttZoneTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttZoneTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttZoneTopic actualiza el prefijo con el que el dispositivo crea los topicos de estado de las zonas por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttZoneTopic") {

      memset(mqttZoneTopicValue, 0, sizeof mqttZoneTopicValue);
      strncpy(mqttZoneTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt zone topic prefix is now: " + String(mqttZoneTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getMqttFireTopic devuelve el prefijo con el que el dispositivo crea los topicos de Fire
     */
   
    if (coMMand == "getMqttFireTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttFireTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttFireTopic actualiza el prefijo con el que el dispositivo crea los topicos de Fire por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttFireTopic") {

      memset(mqttFireTopicValue, 0, sizeof mqttFireTopicValue);
      strncpy(mqttFireTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt Fire topic prefix is now: " + String(mqttFireTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getMqttTroubleTopic devuelve el topico en el que el dispositivo publica el estado de Trouble
     */
   
    if (coMMand == "getMqttTroubleTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttTroubleTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttTroubleTopic actualiza el topico en el que el dispositivo publica el estado de Trouble por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttTroubleTopic") {

      memset(mqttTroubleTopicValue, 0, sizeof mqttTroubleTopicValue);
      strncpy(mqttTroubleTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt Trouble topic is now: " + String(mqttTroubleTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getMqttCommandTopic devuelve el topico en el que el dispositivo escucha commandos.
     */
   
    if (coMMand == "getMqttCommandTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttCommandTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttCommandTopic actualiza el topico en el que el dispositivo escucha comandos por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttCommandTopic") {

      memset(mqttCommandTopicValue, 0, sizeof mqttCommandTopicValue);
      strncpy(mqttCommandTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt Commands topic is now: " + String(mqttCommandTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getMqttKeepAliveTopic devuelve topico en el que el dispositivo pubica el mensaje de keepalive.
     */
   
    if (coMMand == "getMqttKeepAliveTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttKeepAliveTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttKeepAliveTopic actualiza el topico en el que el dispositivo publica el mensaje de keepalive por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttKeepAliveTopic") {

      memset(mqttKeepAliveTopicValue, 0, sizeof mqttKeepAliveTopicValue);
      strncpy(mqttKeepAliveTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt keepAlive topic is now: " + String(mqttKeepAliveTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getEnableMonitoring devuelve valor de inicialilzaci�n de la funcionalidad de monitoreo.
     */
   
    if (coMMand == "getEnableMonitoring") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(enableMonitoringValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateEnableMonitoring actualiza el valor de inicializaci�n de la funcionalidad de monitoreo por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateEnableMonitoring") {

      memset(enableMonitoringValue, 0, sizeof enableMonitoringValue);
      strncpy(enableMonitoringValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt monitoring is now: " + String(enableMonitoringValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getMonitoringTopic devuelve prefijo con el que el dispositivo crea los topicos para publicar la informaci�n de monitoreo.
     */
   
    if (coMMand == "getMonitoringTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(monitoringTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMonitoringTopic actualiza el prefijo con el que el dispositivo crea los topicos para publicar la informacion de monitoreo por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMonitoringTopic") {

      memset(monitoringTopicValue, 0, sizeof monitoringTopicValue);
      strncpy(monitoringTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt monitoring topic prefix is now: " + String(monitoringTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


/////////// TIMER

 /*    
     *   El comando getTimerStatus devuelve el valor correspondiente al estado del timer.
     */
   
    if (coMMand == "getTimerStatus") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(tiMerStatusValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateTimerStatus actualiza el valor correspondiente al estado del timer por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateTimerStatus") {

      memset(tiMerStatusValue, 0, sizeof monitoringTopicValue);
      strncpy(tiMerStatusValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Timer status is now: " + String(tiMerStatusValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getTimerString devuelve el timer string de la funcionalidad Timer.
     */
   
    if (coMMand == "getTimerString") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(tiMerStringValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateTimerString actualiza el timer string de la funcionalidad Timer por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateTimerString") {

      memset(tiMerStringValue, 0, sizeof tiMerStringValue);
      strncpy(tiMerStringValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Timer string updated";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getPublishTimerString devuelve el valor para la opcion de publicacion de timer string.
     */
   
    if (coMMand == "getPublishTimerString") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(publishTimerStringValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updatePublishTimerString actualiza el valor para la opcione de publicaci�n del timer string por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updatePublishTimerString") {

      memset(publishTimerStringValue, 0, sizeof publishTimerStringValue);
      strncpy(publishTimerStringValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Publish Timer String value is now: " + String(publishTimerStringValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

////////// END TIMER

///////// NTP

  /*    
     *   El comando getNtpServer devuelve el ntp server configurado.
     */
   
    if (coMMand == "getNtpServer") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(ntpServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateNtpServer actualiza el valor del NTP server por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateNtpServer") {

      memset(ntpServerValue, 0, sizeof ntpServerValue);
      strncpy(ntpServerValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "NTP server is now: " + String(ntpServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getTimeZone devuelve el valor de la time zone configurada.
     */
   
    if (coMMand == "getTimeZone") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(timeZoneValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateTimeZone actualiza el valor de la time zone configurada por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateTimeZone") {

      memset(timeZoneValue, 0, sizeof timeZoneValue);
      strncpy(timeZoneValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Time Zone is now: " + String(timeZoneValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getNtpUpdateInterval devuelve el valor en segundos del intervalo de consulta al NTP server.
     */
   
    if (coMMand == "getNtpUpdateInterval") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(ntpUpdateIntervalValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateNtpUpdateInterval actualiza el valor en segundos del intervalo de consulta al NTP server por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateNtpUpdateInterval") {

      memset(ntpUpdateIntervalValue, 0, sizeof ntpUpdateIntervalValue);
      strncpy(ntpUpdateIntervalValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "NTP update interval is now: " + String(ntpUpdateIntervalValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getTimeDST devuelve el valor del parametro DST.
     */
   
    if (coMMand == "getTimeDST") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(timeDSTValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateTimeDST actualiza el valor del parametro DST por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateTimeDST") {

      memset(timeDSTValue, 0, sizeof timeDSTValue);
      strncpy(timeDSTValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "DST value is now: " + String(timeDSTValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }



////////// END NTP



     /*    COMANDOS QUE FALTA IMPLEMENTAR
      * 
      * getTiMerStatus  tiMerStatusValue
      * updateTiMerStatus: param1 = new timer status
      * getTiMerString   tiMerStringValue
      * updateTiMerString: param1 = add/del, param2 = target timer string (ademas este comenado tiene que verificar que entren los strings en 98 caracteres)
      * getPublishTimerString publisgTimerStringValue
      * updatePublishTimerString param1 = new publisgTimerString value
      * 
      * getNtpServer   ntpServerValue
      * updateNtpServer: param1 =  new ntp server
      * getTimeZone   timeZoneValue
      * updateTimeZone: param1 = new timezone
      * 
      * get y update el intervalo de update del server
      * get (readable y timestring) y set la hora  
      * force update del NTP server
      * 
      * getTimeDST   timeDSTValue
      * updateTimeDST: param1 = new DST state
      * 
      * 
      * getHideSSID   hideSSIDValue
      * updateHideSSID: param1 = new hide SSID value (no implementar)
      * 
      */







    /*
     * El comando saveConfig guarda la configuraci�n para que persista luego de un reinicio del dispositivo y lo resetea
     */

    if (coMMand == "saveConfig") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Config saved. Rebooting...";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      
      iotWebConf.saveConfig();
     }

     /*
     * El comando reset reinicia el dispositivo
     */

    if (coMMand == "reset") {
      
      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Rebooting in 5 seconds...";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      
      needReset = true;
    }


    
   /*
    * Error comando desconocido
    */
    if (commandResponses == 0) {
      
      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "FAIL";
      jsonBuffer["comments"] = "Unknown command";
      /*
      JsonArray& data = rootOut.createNestedArray("data");
      data.add(48.756080);
      data.add(2.302038);
      */

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
    }

    Serial.println("Se proces� mensaje de configuracion remota y se enviaron " +  String(commandResponses) + " mensajes de respuesta.");
    
  }
}
