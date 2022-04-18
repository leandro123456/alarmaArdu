void PublicarConfiguracionInicial(){
  if(mqttClient.connected()){
    Serial.println("Publicar configuracion inicial");
    mqttClient.publish(mqttDeviceConfigValue, "{\"mail\":\""+ (String)emailValue + "\",\"pwd\":\"" + (String)passwordFinalValue + "\",\"NdvHATypDvid\":\"" + (String)isNewDeviceValue + "-" + (String)useHAmqttDiscoveryValue + "-" + (String)productType+"-"+(String)deviceIdFinalValue+"\"}", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); 
    /// --------------- SerialDebug: ----------
    Serial.println("Publishing Initial data to register...");
    Serial.println("Message sent to initial register. Topic: " + (String)mqttDeviceConfigValue + "{\"mail\":\""+ (String)emailValue + "\",\"NdvHATypDvid\":\"" + (String)isNewDeviceValue + "-" + (String)useHAmqttDiscoveryValue + "-" + (String)productType+"-"+(String)deviceIdFinalValue+"\"}" );
    // --------------- mqttDebug: ---------
    if (atoi(enableMqttDebugValue) == 1) {
      mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " - Data published on: " + (String)mqttServerValue + " Topic: " + (String)mqttDeviceConfigValue + " Payload: {\"deviceId\":\"" + String(deviceIdFinalValue) +"\"}", false, atoi(mqttQoSValue));
    }
  }else{
    // --------------- SerialDebug: ----------
    Serial.println("FAIL: Data cound not be published because not connected to broker" );
  }
}

void publicaEstados(){
  if(mqttClient.connected()){
    Serial.println("Devide ID: "+ String(deviceIdFinalValue));
    mqttClient.publish(mqttKeepAliveTopicValue, "{\"deviceID\":\"" + String(deviceIdFinalValue) +"\",\"DSC\":" + dsc.keybusConnected + ",\"MQTT\":" + mqttClient.connected() + ",\"dBm\":" + String(WiFi.RSSI()) + "}", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); 
    /// --------------- SerialDebug: ----------
    Serial.println("Message sent. Topic: " + (String)mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdFinalValue) + "\"}" );
    // --------------- mqttDebug: ---------
    if (atoi(enableMqttDebugValue) == 1) {
      mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " - Data published on: " + (String)mqttServerValue + " Topic: " + (String)mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdFinalValue) +"\"}", false, atoi(mqttQoSValue));
    }

    if(hanosended){
      Serial.println("##################################################################");
      Serial.println("##################################################################");
      Serial.println("##################################################################");
      //activePartition = dsc.writePartition;
      mqttClient.publish(mqttActivePartitionTopicValue, String(activePartition), true, atoi(mqttQoSValue));             
      Serial.println("Active partition sended: "+String(activePartition)+ " to: "+ String(mqttActivePartitionTopicValue));
      SendHaConfiguration();
      hanosended=false;
    }
  } else {
    // --------------- SerialDebug: ----------
    Serial.println("FAIL: Data cound not be published because not connected to broker" );
    // --------------- mqttDebug: ---------
    if (atoi(enableMqttDebugValue) == 1) {
      mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " - FAIL: Data cound not be published because not connected to broker: " + mqttServerValue, false, atoi(mqttQoSValue));
    }
  }
}


// Publishes the partition status message [nuevo]
void publishMessage(String sourceTopic, byte partition) {
  //char publishTopic[strlen(sourceTopic) + strlen(mqttPartitionMessageSuffix) + 2];
  char partitionNumber[2];

  // Appends the sourceTopic with the partition number and message topic
  //itoa(partition + 1, partitionNumber, 10);
  //strcpy(publishTopic, sourceTopic);
  //strcat(publishTopic, partitionNumber);
  //strcat(publishTopic, mqttPartitionMessageSuffix);

  // Publishes the current partition message
  switch (dsc.status[partition]) {
    case 0x01: mqttClient.publish(sourceTopic, "Partition ready",false, atoi(mqttQoSValue)); break;
    case 0x02: mqttClient.publish(sourceTopic, "Stay zones open",false, atoi(mqttQoSValue)); break;
    case 0x03: mqttClient.publish(sourceTopic, "Zones open",false, atoi(mqttQoSValue)); break;
    case 0x04: mqttClient.publish(sourceTopic, "Armed: Stay",false, atoi(mqttQoSValue)); break;
    case 0x05: mqttClient.publish(sourceTopic, "Armed: Away",false, atoi(mqttQoSValue)); break;
    case 0x06: mqttClient.publish(sourceTopic, "Armed: No entry delay",false, atoi(mqttQoSValue)); break;
    case 0x07: mqttClient.publish(sourceTopic, "Failed to arm",false, atoi(mqttQoSValue)); break;
    case 0x08: mqttClient.publish(sourceTopic, "Exit delay in progress",false, atoi(mqttQoSValue)); break;
    case 0x09: mqttClient.publish(sourceTopic, "Arming with no entry delay",false, atoi(mqttQoSValue)); break;
    case 0x0B: mqttClient.publish(sourceTopic, "Quick exit in progress",false, atoi(mqttQoSValue)); break;
    case 0x0C: mqttClient.publish(sourceTopic, "Entry delay in progress",false, atoi(mqttQoSValue)); break;
    case 0x0D: mqttClient.publish(sourceTopic, "Entry delay after alarm",false, atoi(mqttQoSValue)); break;
    case 0x0E: mqttClient.publish(sourceTopic, "Function not available"); break;
    case 0x10: mqttClient.publish(sourceTopic, "Keypad lockout",false, atoi(mqttQoSValue)); break;
    case 0x11: mqttClient.publish(sourceTopic, "Partition in alarm",false, atoi(mqttQoSValue)); break;
    case 0x12: mqttClient.publish(sourceTopic, "Battery check in progress"); break;
    case 0x14: mqttClient.publish(sourceTopic, "Auto-arm in progress",false, atoi(mqttQoSValue)); break;
    case 0x15: mqttClient.publish(sourceTopic, "Arming with bypassed zones",false, atoi(mqttQoSValue)); break;
    case 0x16: mqttClient.publish(sourceTopic, "Armed: No entry delay",false, atoi(mqttQoSValue)); break;
    case 0x17: mqttClient.publish(sourceTopic, "Power saving: Keypad blanked",false, atoi(mqttQoSValue)); break;
    case 0x19: mqttClient.publish(sourceTopic, "Disarmed: Alarm memory"); break;
    case 0x22: mqttClient.publish(sourceTopic, "Disarmed: Recent closing",false, atoi(mqttQoSValue)); break;
    case 0x2F: mqttClient.publish(sourceTopic, "Keypad LCD test"); break;
    case 0x33: mqttClient.publish(sourceTopic, "Command output in progress",false, atoi(mqttQoSValue)); break;
    case 0x3D: mqttClient.publish(sourceTopic, "Disarmed: Alarm memory",false, atoi(mqttQoSValue)); break;
    case 0x3E: mqttClient.publish(sourceTopic, "Partition disarmed",false, atoi(mqttQoSValue)); break;
    case 0x40: mqttClient.publish(sourceTopic, "Keypad blanked",false, atoi(mqttQoSValue)); break;
    case 0x8A: mqttClient.publish(sourceTopic, "Activate stay/away zones",false, atoi(mqttQoSValue)); break;
    case 0x8B: mqttClient.publish(sourceTopic, "Quick exit",false, atoi(mqttQoSValue)); break;
    case 0x8E: mqttClient.publish(sourceTopic, "Function not available",false, atoi(mqttQoSValue)); break;
    case 0x8F: mqttClient.publish(sourceTopic, "Invalid access code",false, atoi(mqttQoSValue)); break;
    case 0x9E: mqttClient.publish(sourceTopic, "Enter * function key",false, atoi(mqttQoSValue)); break;
    case 0x9F: mqttClient.publish(sourceTopic, "Enter access code",false, atoi(mqttQoSValue)); break;
    case 0xA0: mqttClient.publish(sourceTopic, "*1: Zone bypass",false, atoi(mqttQoSValue)); break;
    case 0xA1: mqttClient.publish(sourceTopic, "*2: Trouble menu",false, atoi(mqttQoSValue)); break;
    case 0xA2: mqttClient.publish(sourceTopic, "*3: Alarm memory",false, atoi(mqttQoSValue)); break;
    case 0xA3: mqttClient.publish(sourceTopic, "*4: Door chime enabled",false, atoi(mqttQoSValue)); break;
    case 0xA4: mqttClient.publish(sourceTopic, "*4: Door chime disabled",false, atoi(mqttQoSValue)); break;
    case 0xA5: mqttClient.publish(sourceTopic, "Enter master code",false, atoi(mqttQoSValue)); break;
    case 0xA6: mqttClient.publish(sourceTopic, "*5: Access codes",false, atoi(mqttQoSValue)); break;
    case 0xA7: mqttClient.publish(sourceTopic, "*5: Enter new 4-digit code",false, atoi(mqttQoSValue)); break;
    case 0xA9: mqttClient.publish(sourceTopic, "*6: User functions",false, atoi(mqttQoSValue)); break;
    case 0xAA: mqttClient.publish(sourceTopic, "*6: Time and date",false, atoi(mqttQoSValue)); break;
    case 0xAB: mqttClient.publish(sourceTopic, "*6: Auto-arm time",false, atoi(mqttQoSValue)); break;
    case 0xAC: mqttClient.publish(sourceTopic, "*6: Auto-arm enabled",false, atoi(mqttQoSValue)); break;
    case 0xAD: mqttClient.publish(sourceTopic, "*6: Auto-arm disabled",false, atoi(mqttQoSValue)); break;
    case 0xAF: mqttClient.publish(sourceTopic, "*6: System test",false, atoi(mqttQoSValue)); break;
    case 0xB0: mqttClient.publish(sourceTopic, "*6: Enable DLS",false, atoi(mqttQoSValue)); break;
    case 0xB2: mqttClient.publish(sourceTopic, "*7: Command output",false, atoi(mqttQoSValue)); break;
    case 0xB3: mqttClient.publish(sourceTopic, "*7: Command output",false, atoi(mqttQoSValue)); break;
    case 0xB7: mqttClient.publish(sourceTopic, "Enter installer code",false, atoi(mqttQoSValue)); break;
    case 0xB8: mqttClient.publish(sourceTopic, "Enter * function key while armed",false, atoi(mqttQoSValue)); break;
    case 0xB9: mqttClient.publish(sourceTopic, "*2: Zone tamper menu",false, atoi(mqttQoSValue)); break;
    case 0xBA: mqttClient.publish(sourceTopic, "*2: Zones with low batteries",false, atoi(mqttQoSValue)); break;
    case 0xBC: mqttClient.publish(sourceTopic, "*5: Enter new 6-digit code"); break;
    case 0xBF: mqttClient.publish(sourceTopic, "*6: Auto-arm select day"); break;
    case 0xC6: mqttClient.publish(sourceTopic, "*2: Zone fault menu",false, atoi(mqttQoSValue)); break;
    case 0xC8: mqttClient.publish(sourceTopic, "*2: Service required menu",false, atoi(mqttQoSValue)); break;
    case 0xCD: mqttClient.publish(sourceTopic, "Downloading in progress"); break;
    case 0xCE: mqttClient.publish(sourceTopic, "Active camera monitor selection"); break;
    case 0xD0: mqttClient.publish(sourceTopic, "*2: Keypads with low batteries",false, atoi(mqttQoSValue)); break;
    case 0xD1: mqttClient.publish(sourceTopic, "*2: Keyfobs with low batteries",false, atoi(mqttQoSValue)); break;
    case 0xD4: mqttClient.publish(sourceTopic, "*2: Sensors with RF delinquency",false, atoi(mqttQoSValue)); break;
    case 0xE4: mqttClient.publish(sourceTopic, "*8: Installer programming, 3 digits",false, atoi(mqttQoSValue)); break;
    case 0xE5: mqttClient.publish(sourceTopic, "Keypad slot assignment",false, atoi(mqttQoSValue)); break;
    case 0xE6: mqttClient.publish(sourceTopic, "Input: 2 digits",false, atoi(mqttQoSValue)); break;
    case 0xE7: mqttClient.publish(sourceTopic, "Input: 3 digits",false, atoi(mqttQoSValue)); break;
    case 0xE8: mqttClient.publish(sourceTopic, "Input: 4 digits",false, atoi(mqttQoSValue)); break;
    case 0xE9: mqttClient.publish(sourceTopic, "Input: 5 digits",false, atoi(mqttQoSValue)); break;
    case 0xEA: mqttClient.publish(sourceTopic, "Input HEX: 2 digits",false, atoi(mqttQoSValue)); break;
    case 0xEB: mqttClient.publish(sourceTopic, "Input HEX: 4 digits",false, atoi(mqttQoSValue)); break;
    case 0xEC: mqttClient.publish(sourceTopic, "Input HEX: 6 digits",false, atoi(mqttQoSValue)); break;
    case 0xED: mqttClient.publish(sourceTopic, "Input HEX: 32 digits",false, atoi(mqttQoSValue)); break;
    case 0xEE: mqttClient.publish(sourceTopic, "Input: 1 option per zone",false, atoi(mqttQoSValue)); break;
    case 0xEF: mqttClient.publish(sourceTopic, "Module supervision field",false, atoi(mqttQoSValue)); break;
    case 0xF0: mqttClient.publish(sourceTopic, "Function key 1",false, atoi(mqttQoSValue)); break;
    case 0xF1: mqttClient.publish(sourceTopic, "Function key 2",false, atoi(mqttQoSValue)); break;
    case 0xF2: mqttClient.publish(sourceTopic, "Function key 3",false, atoi(mqttQoSValue)); break;
    case 0xF3: mqttClient.publish(sourceTopic, "Function key 4",false, atoi(mqttQoSValue)); break;
    case 0xF4: mqttClient.publish(sourceTopic, "Function key 5",false, atoi(mqttQoSValue)); break;
    case 0xF5: mqttClient.publish(sourceTopic, "Wireless module placement test",false, atoi(mqttQoSValue)); break;
    case 0xF6: mqttClient.publish(sourceTopic, "Activate device for test"); break;
    case 0xF7: mqttClient.publish(sourceTopic, "*8: Installer programming, 2 digits",false, atoi(mqttQoSValue)); break;
    case 0xF8: mqttClient.publish(sourceTopic, "Keypad programming",false, atoi(mqttQoSValue)); break;
    case 0xFA: mqttClient.publish(sourceTopic, "Input: 6 digits"); break;
    default: return;
  }
}

void SendHaConfiguration(){

  if(mqttClient.connected()){
  Serial.println("HomeAssistant start configuration");
  //alarm-control-panel
  char HomeAssitanConfTopic[STRING_LEN];
  String deviceID=String(deviceIdFinalValue);
  String HomeAssitanValue= String(defaultHAPrefixValue)+"/alarm_control_panel/"+deviceID+"/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  Serial.println("HomeAssistant TOPICO: "+ String(HomeAssitanConfTopic));
  //Serial.println("HomeAssistant MENSAJE: " + String("{\"~\":\""+ deviceID + "\",\"name\":\"Security Partition 1\",\"cmd_t\":\"~/Set\",\"stat_t\":\"~/Partition1\",\"avty_t\":\"~/Status\",\"pl_disarm\":\"1D\",\"pl_arm_home\":\"1S\",\"pl_arm_away\":\"1A\",\"pl_arm_nite\":\"1N\"}"));
  
  //DynamicJsonDocument doc(2048);
  //JsonObject obj = doc.to<JsonObject>();
  //obj["~"] = deviceID;
  //obj["name"] = "Security Partition 1";
  //obj["cmd_t"] = "~/Set";
  //obj["stat_t"] = "~/Partition1";
  //obj["avty_t"] = "~/Status";
  //obj["pl_disarm"] = "1D";
  //obj["pl_arm_home"] = "1S";
  //obj["pl_arm_away"] = "1A";
  //obj["pl_arm_nite"] = "1N";

  //char out[2048];
  //int b =serializeJson(obj, out);

  DynamicJsonDocument doc(500);
  String RConfCommandResult;
  //doc["~"] = deviceID;
  doc["name"] = "Security_Partition_1";
  doc["cmd_t"] = deviceID+"/Set";
  doc["pl_disarm"] = "1D";
  //serializeJson(doc, RConfCommandResult);
  char buffer[256];
  serializeJson(doc, buffer);

  mqttClient.publish(HomeAssitanConfTopic, buffer, true, 0);
  Serial.println("ya lo envie BIEN!!!!!!");

  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Security Partition 1\",\"cmd_t\":\"~/Set\",\"stat_t\":\"~/Partition1\",\"avty_t\":\"~/Status\",\"pl_disarm\":\"1D\",\"pl_arm_home\":\"1S\",\"pl_arm_away\":\"1A\",\"pl_arm_nite\":\"1N\"}", true, atoi(mqttQoSValue)); 
  //Wifi-sensor
  //HomeAssitanValue=String(defaultHAPrefixValue)+"/sensor/"+deviceID+"/config";
  //memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  //strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Wifi Alarm\",\"dev_cla\":\"signal_strength\",\"stat_t\":\"~/keepAlive\",\"unit_of_meas\":\"dBm\",\"val_tpl\":\"{{value_json.dBm}}\",\"avty_t\":\"~/Status\",\"ic\":\"mdi:shield\"}", true, atoi(mqttQoSValue)); 
  //Trouble
  //HomeAssitanConfTopic=(String)defaultHAPrefixValue+"/binary_sensor/"+deviceID+"-Trouble/config";
  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Security Trouble\",\"dev_cla\":\"problem\",\"stat_t\":\"~/Trouble\",\"pl_on\":\"1\",\"pl_off\":\"2\"}", (bool) 1, atoi(mqttQoSValue));
  //Zone1
  //HomeAssitanConfTopic=(String)defaultHAPrefixValue+"/binary_sensor/"+deviceID+"-Z1/config";
  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Zone 1\",\"dev_cla\":\"door\",\"stat_t\":\"~/Zone1\",\"pl_on\":\"1\",\"pl_off\":\"2\"}", (bool) 1, atoi(mqttQoSValue));
  //Zone2
  //HomeAssitanConfTopic=(String)defaultHAPrefixValue+"/binary_sensor/"+deviceID+"-Z2/config";
  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Zone 2\",\"dev_cla\":\"window\",\"stat_t\":\"~/Zone2\",\"pl_on\":\"1\",\"pl_off\":\"2\"}", (bool) 1, atoi(mqttQoSValue));
  //Zone3
  //HomeAssitanConfTopic=(String)defaultHAPrefixValue+"/binary_sensor/"+deviceID+"-Z3/config";
  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Zone 3\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone3\",\"pl_on\":\"1\",\"pl_off\":\"2\"}", (bool) 1, atoi(mqttQoSValue));
  //Zone4
  //HomeAssitanConfTopic=(String)defaultHAPrefixValue+"/binary_sensor/"+deviceID+"-Z4/config";
  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Zone 4\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone4\",\"pl_on\":\"1\",\"pl_off\":\"2\"}", (bool) 1, atoi(mqttQoSValue));
  //Zone5
  //HomeAssitanConfTopic=(String)defaultHAPrefixValue+"/binary_sensor/"+deviceID+"-Z5/config";
  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Zone 5\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone5\",\"pl_on\":\"1\",\"pl_off\":\"2\"}", (bool) 1, atoi(mqttQoSValue));
  //Zone6
  //HomeAssitanConfTopic=(String)defaultHAPrefixValue+"/binary_sensor/"+deviceID+"-Z6/config";
  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Zone 6\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone6\",\"pl_on\":\"1\",\"pl_off\":\"2\"}", (bool) 1, atoi(mqttQoSValue));
  //Zone7
  //HomeAssitanConfTopic=(String)defaultHAPrefixValue+"/binary_sensor/"+deviceID+"-Z7/config";
  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Zone 7\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone7\",\"pl_on\":\"1\",\"pl_off\":\"2\"}", (bool) 1, atoi(mqttQoSValue));
  //Zone8
  //HomeAssitanConfTopic=(String)defaultHAPrefixValue+"/binary_sensor/"+deviceID+"-Z8/config";
  //mqttClient.publish(HomeAssitanConfTopic, "{\"~\":\""+ deviceID + "\",\"name\":\"Zone 8\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone8\",\"pl_on\":\"1\",\"pl_off\":\"2\"}", (bool) 1, atoi(mqttQoSValue));
  Serial.println("HomeAssistant configuration sent");
  }
  else{
    Serial.println("El cliente no esta conectado para publicar el mensaje!!!!!!!!!!!!!!!!");
  }
}