void publicaEstados(){
  if(mqttClient.connected()){
    String msgtext="{\"deviceID\":\"" + String(deviceIdFinalValue) + ",\"MQTT\":" + mqttClient.connected() + ",\"dBm\":" + String(WiFi.RSSI()) + "}";
    mqttClient.publish(mqttKeepAliveTopicValue,msgtext.c_str() , (bool) atoi(mqttRetainValue));  
    /// --------------- SerialDebug: ----------
    Serial.println("Message sent. Topic: " + (String)mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdFinalValue) + "\"}" );
    // --------------- mqttDebug: ---------
    if (atoi(enableMqttDebugValue) == 1) {
      msgtext= String(deviceIdFinalValue) + " - Data published on: " + (String)mqttServerValue + " Topic: " + (String)mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdFinalValue) +"\"}";
      mqttClient.publish(MqttDebugTopicValue,msgtext.c_str() , false);
    }

    if(hanosended){
      mqttClient.publish(mqttActivePartitionTopicValue, String(activePartition).c_str(), true);             
      Serial.println("Active partition sended: "+String(activePartition)+ " to: "+ String(mqttActivePartitionTopicValue));
      SendHaConfiguration();
      hanosended=false;

      if (dsc.keybusConnected) {
        for (byte partition = 0; partition < dscPartitions; partition++) {
          char partitionNumber[2];
          itoa(partition + 1, partitionNumber, 10);
          char publishTopic[strlen(mqttPartitionTopicValue) + 2];
          appendPartition(mqttPartitionTopicValue, partition, publishTopic);
          String msgtext1;
          if (dsc.armed[partition]) {
            if (dsc.armedAway[partition] && dsc.noEntryDelay[partition]) 
              mqttClient.publish(publishTopic, "armed_night", true);
            else if (dsc.armedAway[partition]) 
              mqttClient.publish(publishTopic, "armed_away", true);
            else if (dsc.armedStay[partition] && dsc.noEntryDelay[partition]) 
              mqttClient.publish(publishTopic, "armed_night", true);
            else if (dsc.armedStay[partition]) 
              mqttClient.publish(publishTopic, "armed_home", true);
            //Monitoring
            if (atoi(enableMonitoringValue) > 1) {
              msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
              if (dsc.armedAway[partition]){
                sendMonitoring(msgtext1,"armed_away",String(deviceIdFinalValue) +" MONITORING: ARMED_AWAY Status published for partition " + partitionNumber);
              }
              else if (dsc.armedStay[partition]){
                sendMonitoring(msgtext1,"armed_home",String(deviceIdFinalValue) + " MONITORING: ARMED_HOME status published for partition " + partitionNumber);
              }
            }
          }
          else {
            mqttClient.publish(publishTopic, "disarmed", true);
            msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
            sendMonitoring(msgtext1,"disarmed",(String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
          }
        }
      }
    }
  } else {
    Serial.println("FAIL: Data cound not be published because not connected to broker" );
    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext= String(deviceIdFinalValue) + " - FAIL: Data cound not be published because not connected to broker: " + mqttServerValue;
      mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), false);
    }
  }
}


// Publishes the partition status message
void publishMessage(const char* sourceTopic, byte partition) {
  //char publishTopic[strlen(sourceTopic) + strlen(mqttPartitionMessageSuffix) + 2];
  char publishTopic[strlen(sourceTopic) + 2];
  char partitionNumber[2];

  // Appends the sourceTopic with the partition number and message topic
  itoa(partition + 1, partitionNumber, 10);
  strcpy(publishTopic, sourceTopic);
  strcat(publishTopic, partitionNumber);
  //strcat(publishTopic, mqttPartitionMessageSuffix);

  // Publishes the current partition message
  switch (dsc.status[partition]) {
    case 0x01: mqttClient.publish(publishTopic, "Partition ready", true); break;
    case 0x02: mqttClient.publish(publishTopic, "Stay zones open", true); break;
    case 0x03: mqttClient.publish(publishTopic, "Zones open", true); break;
    case 0x04: mqttClient.publish(publishTopic, "Armed: Stay", true); break;
    case 0x05: mqttClient.publish(publishTopic, "Armed: Away", true); break;
    case 0x06: mqttClient.publish(publishTopic, "Armed: No entry delay", true); break;
    case 0x07: mqttClient.publish(publishTopic, "Failed to arm", true); break;
    case 0x08: mqttClient.publish(publishTopic, "Exit delay in progress", true); break;
    case 0x09: mqttClient.publish(publishTopic, "Arming with no entry delay", true); break;
    case 0x0B: mqttClient.publish(publishTopic, "Quick exit in progress", true); break;
    case 0x0C: mqttClient.publish(publishTopic, "Entry delay in progress", true); break;
    case 0x0D: mqttClient.publish(publishTopic, "Entry delay after alarm", true); break;
    case 0x0E: mqttClient.publish(publishTopic, "Function not available"); break;
    case 0x10: mqttClient.publish(publishTopic, "Keypad lockout", true); break;
    case 0x11: mqttClient.publish(publishTopic, "Partition in alarm", true); break;
    case 0x12: mqttClient.publish(publishTopic, "Battery check in progress"); break;
    case 0x14: mqttClient.publish(publishTopic, "Auto-arm in progress", true); break;
    case 0x15: mqttClient.publish(publishTopic, "Arming with bypassed zones", true); break;
    case 0x16: mqttClient.publish(publishTopic, "Armed: No entry delay", true); break;
    case 0x17: mqttClient.publish(publishTopic, "Power saving: Keypad blanked", true); break;
    case 0x19: mqttClient.publish(publishTopic, "Disarmed: Alarm memory"); break;
    case 0x22: mqttClient.publish(publishTopic, "Disarmed: Recent closing", true); break;
    case 0x2F: mqttClient.publish(publishTopic, "Keypad LCD test"); break;
    case 0x33: mqttClient.publish(publishTopic, "Command output in progress", true); break;
    case 0x3D: mqttClient.publish(publishTopic, "Disarmed: Alarm memory", true); break;
    case 0x3E: mqttClient.publish(publishTopic, "Partition disarmed", true); break;
    case 0x40: mqttClient.publish(publishTopic, "Keypad blanked", true); break;
    case 0x8A: mqttClient.publish(publishTopic, "Activate stay/away zones", true); break;
    case 0x8B: mqttClient.publish(publishTopic, "Quick exit", true); break;
    case 0x8E: mqttClient.publish(publishTopic, "Function not available", true); break;
    case 0x8F: mqttClient.publish(publishTopic, "Invalid access code", true); break;
    case 0x9E: mqttClient.publish(publishTopic, "Enter * function key", true); break;
    case 0x9F: mqttClient.publish(publishTopic, "Enter access code", true); break;
    case 0xA0: mqttClient.publish(publishTopic, "*1: Zone bypass", true); break;
    case 0xA1: mqttClient.publish(publishTopic, "*2: Trouble menu", true); break;
    case 0xA2: mqttClient.publish(publishTopic, "*3: Alarm memory", true); break;
    case 0xA3: mqttClient.publish(publishTopic, "*4: Door chime enabled", true); break;
    case 0xA4: mqttClient.publish(publishTopic, "*4: Door chime disabled", true); break;
    case 0xA5: mqttClient.publish(publishTopic, "Enter master code", true); break;
    case 0xA6: mqttClient.publish(publishTopic, "*5: Access codes", true); break;
    case 0xA7: mqttClient.publish(publishTopic, "*5: Enter new 4-digit code", true); break;
    case 0xA9: mqttClient.publish(publishTopic, "*6: User functions", true); break;
    case 0xAA: mqttClient.publish(publishTopic, "*6: Time and date", true); break;
    case 0xAB: mqttClient.publish(publishTopic, "*6: Auto-arm time", true); break;
    case 0xAC: mqttClient.publish(publishTopic, "*6: Auto-arm enabled", true); break;
    case 0xAD: mqttClient.publish(publishTopic, "*6: Auto-arm disabled", true); break;
    case 0xAF: mqttClient.publish(publishTopic, "*6: System test", true); break;
    case 0xB0: mqttClient.publish(publishTopic, "*6: Enable DLS", true); break;
    case 0xB2: mqttClient.publish(publishTopic, "*7: Command output", true); break;
    case 0xB3: mqttClient.publish(publishTopic, "*7: Command output", true); break;
    case 0xB7: mqttClient.publish(publishTopic, "Enter installer code", true); break;
    case 0xB8: mqttClient.publish(publishTopic, "Enter * function key while armed", true); break;
    case 0xB9: mqttClient.publish(publishTopic, "*2: Zone tamper menu", true); break;
    case 0xBA: mqttClient.publish(publishTopic, "*2: Zones with low batteries", true); break;
    case 0xBC: mqttClient.publish(publishTopic, "*5: Enter new 6-digit code"); break;
    case 0xBF: mqttClient.publish(publishTopic, "*6: Auto-arm select day"); break;
    case 0xC6: mqttClient.publish(publishTopic, "*2: Zone fault menu", true); break;
    case 0xC8: mqttClient.publish(publishTopic, "*2: Service required menu", true); break;
    case 0xCD: mqttClient.publish(publishTopic, "Downloading in progress"); break;
    case 0xCE: mqttClient.publish(publishTopic, "Active camera monitor selection"); break;
    case 0xD0: mqttClient.publish(publishTopic, "*2: Keypads with low batteries", true); break;
    case 0xD1: mqttClient.publish(publishTopic, "*2: Keyfobs with low batteries", true); break;
    case 0xD4: mqttClient.publish(publishTopic, "*2: Sensors with RF delinquency", true); break;
    case 0xE4: mqttClient.publish(publishTopic, "*8: Installer programming, 3 digits", true); break;
    case 0xE5: mqttClient.publish(publishTopic, "Keypad slot assignment", true); break;
    case 0xE6: mqttClient.publish(publishTopic, "Input: 2 digits", true); break;
    case 0xE7: mqttClient.publish(publishTopic, "Input: 3 digits", true); break;
    case 0xE8: mqttClient.publish(publishTopic, "Input: 4 digits", true); break;
    case 0xE9: mqttClient.publish(publishTopic, "Input: 5 digits", true); break;
    case 0xEA: mqttClient.publish(publishTopic, "Input HEX: 2 digits", true); break;
    case 0xEB: mqttClient.publish(publishTopic, "Input HEX: 4 digits", true); break;
    case 0xEC: mqttClient.publish(publishTopic, "Input HEX: 6 digits", true); break;
    case 0xED: mqttClient.publish(publishTopic, "Input HEX: 32 digits", true); break;
    case 0xEE: mqttClient.publish(publishTopic, "Input: 1 option per zone", true); break;
    case 0xEF: mqttClient.publish(publishTopic, "Module supervision field", true); break;
    case 0xF0: mqttClient.publish(publishTopic, "Function key 1", true); break;
    case 0xF1: mqttClient.publish(publishTopic, "Function key 2", true); break;
    case 0xF2: mqttClient.publish(publishTopic, "Function key 3", true); break;
    case 0xF3: mqttClient.publish(publishTopic, "Function key 4", true); break;
    case 0xF4: mqttClient.publish(publishTopic, "Function key 5", true); break;
    case 0xF5: mqttClient.publish(publishTopic, "Wireless module placement test", true); break;
    case 0xF6: mqttClient.publish(publishTopic, "Activate device for test"); break;
    case 0xF7: mqttClient.publish(publishTopic, "*8: Installer programming, 2 digits", true); break;
    case 0xF8: mqttClient.publish(publishTopic, "Keypad programming", true); break;
    case 0xFA: mqttClient.publish(publishTopic, "Input: 6 digits"); break;
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
  //general configuration alarm
  DynamicJsonDocument obj(500);
  char buffer[500];
  obj["~"] = deviceID;
  obj["name"] = "Security Partition 1";
  obj["cmd_t"] = "~/cmd";
  obj["stat_t"] = "~/Partition1";
  obj["avty_t"] = "~/Status";
  obj["pl_disarm"] = "1D";
  obj["pl_arm_home"] = "1S";
  obj["pl_arm_away"] = "1A";
  obj["pl_arm_nite"] = "1N";
  serializeJson(obj, buffer);
  mqttClient.publish(HomeAssitanConfTopic, buffer, true);
  //Wifi-sensor
  DynamicJsonDocument obj1(500);
  char buffer1[500];
  obj1["~"] = deviceID;
  obj1["name"] = "Wifi Alarm";
  obj1["stat_t"] = "~/keepAlive";
  obj1["avty_t"] = "~/Status";
  obj1["dev_cla"] = "signal_strength";
  obj1["unit_of_meas"] = "dBm";
  obj1["val_tpl"] = "{{value_json.dBm}}";
  obj1["ic"] = "mdi:shield";
  serializeJson(obj1, buffer1);
  HomeAssitanValue= String(defaultHAPrefixValue)+"/sensor/"+deviceID+"/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());  
  mqttClient.publish(HomeAssitanConfTopic, buffer1, true); 
  //Status mensajes
  DynamicJsonDocument obj2(500);
  char buffer2[500];
  obj2["~"] = deviceID;
  obj2["name"] = "Security Partition 1";
  obj2["stat_t"] = "~/MNTR/DetailPartition1";
  obj2["avty_t"] = "~/Status";
  obj2["ic"] = "mdi:shield";
  serializeJson(obj2, buffer2);
  HomeAssitanValue= String(defaultHAPrefixValue)+"/sensor/"+deviceID+"-s/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());  
  mqttClient.publish(HomeAssitanConfTopic, buffer2, true); 
  //Trouble
  HomeAssitanValue= String(defaultHAPrefixValue)+"/binary_sensor/"+deviceID+"-Trouble/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  mqttClient.publish(HomeAssitanConfTopic, ("{\"~\":\""+ deviceID + "\",\"name\":\"Security Trouble\",\"dev_cla\":\"problem\",\"stat_t\":\"~/Trouble\",\"pl_on\":\"1\",\"pl_off\":\"0\"}").c_str(), true);
  //Zone1
  HomeAssitanValue= String(defaultHAPrefixValue)+"/binary_sensor/"+deviceID+"-Z1/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  mqttClient.publish(HomeAssitanConfTopic, ("{\"~\":\""+ deviceID + "\",\"name\":\"Zone 1\",\"dev_cla\":\"door\",\"stat_t\":\"~/Zone1\",\"pl_on\":\"1\",\"pl_off\":\"0\"}").c_str(), true);
  //Zone2
    HomeAssitanValue= String(defaultHAPrefixValue)+"/binary_sensor/"+deviceID+"-Z2/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  mqttClient.publish(HomeAssitanConfTopic, ("{\"~\":\""+ deviceID + "\",\"name\":\"Zone 2\",\"dev_cla\":\"window\",\"stat_t\":\"~/Zone2\",\"pl_on\":\"1\",\"pl_off\":\"0\"}").c_str(), true);
  //Zone3
  HomeAssitanValue= String(defaultHAPrefixValue)+"/binary_sensor/"+deviceID+"-Z3/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  mqttClient.publish(HomeAssitanConfTopic, ("{\"~\":\""+ deviceID + "\",\"name\":\"Zone 3\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone3\",\"pl_on\":\"1\",\"pl_off\":\"0\"}").c_str(), true);
  //Zone4
  HomeAssitanValue= String(defaultHAPrefixValue)+"/binary_sensor/"+deviceID+"-Z4/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  mqttClient.publish(HomeAssitanConfTopic, ("{\"~\":\""+ deviceID + "\",\"name\":\"Zone 4\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone4\",\"pl_on\":\"1\",\"pl_off\":\"0\"}").c_str(), true);
  //Zone5
    HomeAssitanValue= String(defaultHAPrefixValue)+"/binary_sensor/"+deviceID+"-Z5/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  mqttClient.publish(HomeAssitanConfTopic, ("{\"~\":\""+ deviceID + "\",\"name\":\"Zone 5\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone5\",\"pl_on\":\"1\",\"pl_off\":\"0\"}").c_str(), true);
  //Zone6
  HomeAssitanValue= String(defaultHAPrefixValue)+"/binary_sensor/"+deviceID+"-Z6/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  mqttClient.publish(HomeAssitanConfTopic, ("{\"~\":\""+ deviceID + "\",\"name\":\"Zone 6\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone6\",\"pl_on\":\"1\",\"pl_off\":\"0\"}").c_str(), true);
  //Zone7
  HomeAssitanValue= String(defaultHAPrefixValue)+"/binary_sensor/"+deviceID+"-Z7/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  mqttClient.publish(HomeAssitanConfTopic, ("{\"~\":\""+ deviceID + "\",\"name\":\"Zone 7\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone7\",\"pl_on\":\"1\",\"pl_off\":\"0\"}").c_str(), true);
  //Zone8
  HomeAssitanValue= String(defaultHAPrefixValue)+"/binary_sensor/"+deviceID+"-Z8/config";
  memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
  strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
  mqttClient.publish(HomeAssitanConfTopic, ("{\"~\":\""+ deviceID + "\",\"name\":\"Zone 8\",\"dev_cla\":\"motion\",\"stat_t\":\"~/Zone8\",\"pl_on\":\"1\",\"pl_off\":\"0\"}").c_str(), true);
  Serial.println("HomeAssistant configuration sent");
  }
  else{
    Serial.println("El cliente no esta conectado para publicar el mensaje!!!!!!!!!!!!!!!!");
  }
}