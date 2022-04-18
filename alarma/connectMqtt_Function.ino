
boolean connectMqtt() {
  unsigned long now = millis();
  if ((lastMqttConnectionAttempt + 1000) > now)
  {
    return false;
  }

  Serial.println("Connecting to MQTT server...");
  if (atoi(enableMqttDebugValue) == 1) {
    mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + 
    " - Connecting to MQTT server...", (bool) atoi(mqttRetainValue),atoi(mqttQoSValue));
  }

  if (!connectMqttOptions()) {
    lastMqttConnectionAttempt = now;
    return false;
  }

  /* --------------- SerialDebug: --------- */
  if (atoi(enableMqttDebugValue) == 1) {
    mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " - Connected to MQTT server!", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));}
  /* --------------- mqttDebug: --------- */
  
  //Serial.println("DeviceID: "+ String(deviceIdFinalValue));
  if(contPrimerINgreso==1 &&(String(deviceIdFinalValue).startsWith("empty") || String(deviceIdFinalValue).equals(""))){
  //if(String(deviceIdFinalValue).startsWith("empty") || String(deviceIdFinalValue).equals("")){
    PublicarConfiguracionInicial();
    mqttClient.subscribe(mqttDeviceConfigResponseValue);
    Serial.println("First Initial Subcribed to: " + (String)mqttDeviceConfigResponseValue);
    contPrimerINgreso=2;
  }
  else{
    if (dsc.keybusConnected) {
      mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
      lastSentStatus = String(mqttBirthMessageValue);

      Serial.println("Status message published: " + lastSentStatus);
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " - Status message published: " + lastSentStatus, (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));}
    }else {
      mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
      lastSentStatus = String(mqttNoDSCValue);

      /* --------------- SerialDebug: --------- */
      Serial.println("Status message published: " + lastSentStatus);
      if (atoi(enableMqttDebugValue) == 1) {mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " - Status message published: " + lastSentStatus, (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

    publicaEstados();
    mqttClient.subscribe(mqttCommandTopicValue);
    /* --------------- SerialDebug: --------- */
    Serial.println((String) "Subcribed to: " + mqttCommandTopicValue);
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {
      mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " - Subcribed to: " + mqttCommandTopicValue), (bool) atoi(mqttRetainValue), atoi(mqttQoSValue);
    }
  }
  return true;
}



boolean connectMqttOptions(){
  boolean result;
  net2.setInsecure();
  mqttClient.setWill(mqttStatusTopicValue, mqttLwtMessageValue, true, 0);
  if(String(deviceIdFinalValue).equals("empty") || String(deviceIdFinalValue).equals("")){
    //Serial.println((String)"mqttClientID INITIAL: " + String(clientIDrnd));
    result = mqttClient.connect(clientIDrnd, "mqttusr", "mqttpwd");
    Serial.println("resulttado1 coneccion: "+ (String)result);
    return result;
  }
  else{
    Serial.println((String)"mqttClientID: " + String(mqttClientIDValue));
    Serial.println((String)"mqtt USER: " + String(mqttUserNameValue));
    Serial.println((String)"mqtt PASS: " + String(mqttUserPasswordValue));
    result = mqttClient.connect(mqttClientIDValue, mqttUserNameValue, mqttUserPasswordValue);
    return result;
  }
}