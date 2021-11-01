void publicaEstados(){
  if(mqttClient.connected()){
    // PUBLISH TIMER STRING This needs to be improved being moved to Arduino Json
    if (String(tiMerStringValue).length() > 0 && atoi(publishTimerStringValue) == 1){
      String msgtext="{\"deviceId\":\"" +deviceID + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}" ;
      mqttClient.publish(mqttKeepAliveTopicValue, msgtext.c_str(), (bool) atoi(mqttRetainValue));
      
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {
        String msgtext=deviceID + " - Data published on: " + mqttServerValue + " Topic: " + mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}";
        mqttClient.publish(MqttDebugTopicValue,msgtext.c_str() , false);}
      /* --------------- mqttDebug: --------- */
      /* --------------- SerialDebug: --------- */
        Serial.println("Publishing data...");
        Serial.println((String) "Message sent. Topic: " + mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}" );
      /* --------------- SerialDebug: --------- */
    }
    // end PUBLISH TIMER STRING
    String msgtext="{\"deviceID\":\"" + deviceID + "\",\"dateTime\":\"" + getTimeString().substring(1) + "\",\"DSC\":" + dsc.keybusConnected + ",\"MQTT\":" + mqttClient.connected() + ",\"dBm\":" + String(WiFi.RSSI()) + "}";
    mqttClient.publish(mqttKeepAliveTopicValue,msgtext.c_str() , (bool) atoi(mqttRetainValue)); 
    Serial.println("Ya termino de publicar!!!");
  } else {
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext= deviceID + " - FAIL: Data cound not be published because not connected to broker: " + mqttServerValue;
      mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), false);
      }
    /* --------------- SerialDebug: --------- */
      Serial.println("Publishing data...");
      Serial.println((String) "FAIL: Data cound not be published because not connected to broker" );
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