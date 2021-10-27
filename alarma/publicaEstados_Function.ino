void publicaEstados(){
  if(mqttClient.connected()){
  
    // PUBLISH TIMER STRING This needs to be improved being moved to Arduino Json
    if (String(tiMerStringValue).length() > 0 && atoi(publishTimerStringValue) == 1){
      String msgtext="{\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}" ;
      msgtext.toCharArray(textmessage, STRING_LEN);
      mqttClient.publish(mqttKeepAliveTopicValue, textmessage, (bool) atoi(mqttRetainValue));
      
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {
        String msgtext=(String) deviceIdValue + " - Data published on: " + mqttServerValue + " Topic: " + mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}";
        msgtext.toCharArray(textmessage, STRING_LEN);
        mqttClient.publish(MqttDebugTopicValue,textmessage , false);}
      /* --------------- mqttDebug: --------- */
      /* --------------- SerialDebug: --------- */
        Serial.println("Publishing data...");
        Serial.println((String) "Message sent. Topic: " + mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}" );
      /* --------------- SerialDebug: --------- */
    }
    // end PUBLISH TIMER STRING
    String msgtext=(String) "{\"deviceID\":\"" + deviceIdValue + "\",\"dateTime\":\"" + getTimeString().substring(1) + "\",\"DSC\":" + dsc.keybusConnected + ",\"MQTT\":" + mqttClient.connected() + ",\"MQTTRM\":" + mqttClient.connected() + ",\"dBm\":" + String(WiFi.RSSI()) + "}";
    msgtext.toCharArray(textmessage, STRING_LEN);
    mqttClient.publish(mqttKeepAliveTopicValue,textmessage , (bool) atoi(mqttRetainValue)); 

  } else {
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext= (String) deviceIdValue + " - FAIL: Data cound not be published because not connected to broker: " + mqttServerValue;
      msgtext.toCharArray(textmessage, STRING_LEN);
      mqttClient.publish(MqttDebugTopicValue,textmessage, false);
      }
    /* --------------- mqttDebug: --------- */
    /* --------------- SerialDebug: --------- */
      Serial.println("Publishing data...");
      Serial.println((String) "FAIL: Data cound not be published because not connected to broker" );
    /* --------------- SerialDebug: --------- */
  }
}
