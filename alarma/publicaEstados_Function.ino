void publicaEstados(){
  if(mqttClient.connected()){
  
    // PUBLISH TIMER STRING This needs to be improved being moved to Arduino Json
    if (String(tiMerStringValue).length() > 0 && atoi(publishTimerStringValue) == 1){
      mqttClient.publish(mqttKeepAliveTopicValue, "{\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}" , (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
      
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClient.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Data published on: " + mqttServerValue + " Topic: " + mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}", false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
      /* --------------- SerialDebug: --------- */
        Serial.println("Publishing data...");
        Serial.println((String) "Message sent. Topic: " + mqttKeepAliveTopicValue + " Payload: {\"deviceId\":\"" + String(deviceIdValue) + "\",\"timerString\":\"" + String(tiMerStringValue) + "\"}" );
      /* --------------- SerialDebug: --------- */
    }
    // end PUBLISH TIMER STRING

    mqttClient.publish(mqttKeepAliveTopicValue, (String) "{\"deviceID\":\"" + deviceIdValue + "\",\"dateTime\":\"" + getTimeString().substring(1) + "\",\"DSC\":" + dsc.keybusConnected + ",\"MQTT\":" + mqttClient.connected() + ",\"MQTTRM\":" + mqttClient.connected() + ",\"dBm\":" + String(WiFi.RSSI()) + "}", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); 

  } else {
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {mqttClient.publish(MqttDebugTopicValue, (String) deviceIdValue + " - FAIL: Data cound not be published because not connected to broker: " + mqttServerValue, false, atoi(remoteConfigQoSValue));}
    /* --------------- mqttDebug: --------- */
    /* --------------- SerialDebug: --------- */
      Serial.println("Publishing data...");
      Serial.println((String) "FAIL: Data cound not be published because not connected to broker" );
    /* --------------- SerialDebug: --------- */
  }
}