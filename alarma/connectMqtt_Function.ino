
boolean connectMqtt() {
  unsigned long now = millis();
  if ((lastMqttConnectionAttempt + 1000) > now)
  {
    // Do not repeat within 1 sec.
    return false;
  }
   /* --------------- SerialDebug: --------- */
  Serial.println("Connecting to MQTT server...");
  if (atoi(enableMqttDebugValue) == 1) {
    mqttClient.publish(MqttDebugTopicValue, deviceID + 
    " - Connecting to MQTT server...", (bool) atoi(mqttRetainValue),atoi(mqttQoSValue));
  }

  if (!connectMqttOptions()) {
    Serial.println("entro a connectMqttOptions VA A a salir con false");
    lastMqttConnectionAttempt = now;
    return false;
  }

  /* --------------- SerialDebug: --------- */
  Serial.println("Connected MQTT server DEBUG");
  if (atoi(enableMqttDebugValue) == 1) {mqttClient.publish(MqttDebugTopicValue, deviceID + " - Connected to MQTT server!", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));}
  /* --------------- mqttDebug: --------- */
  
  if (dsc.keybusConnected) {
    mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
    lastSentStatus = String(mqttBirthMessageValue);

    /* --------------- SerialDebug: --------- */
    Serial.println("Status message published: " + lastSentStatus);
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {mqttClient.publish(MqttDebugTopicValue, deviceID + " - Status message published: " + lastSentStatus, (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));}
  }else {
    mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
    lastSentStatus = String(mqttNoDSCValue);

    /* --------------- SerialDebug: --------- */
    Serial.println("Status message published: " + lastSentStatus);
    if (atoi(enableMqttDebugValue) == 1) {mqttClient.publish(MqttDebugTopicValue, deviceID + " - Status message published: " + lastSentStatus, (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));}
    /* --------------- mqttDebug: --------- */
  }
  // Para publicar estados iniciales
  publicaEstados();

  //Subcribe al topic de comando
  mqttClient.subscribe(mqttCommandTopicValue);
  /* --------------- SerialDebug: --------- */
  Serial.println((String) "Subcribed to: " + mqttCommandTopicValue);
  /* --------------- mqttDebug: --------- */
  if (atoi(enableMqttDebugValue) == 1) {
    mqttClient.publish(MqttDebugTopicValue, deviceID + " - Subcribed to: " + mqttCommandTopicValue), (bool) atoi(mqttRetainValue), atoi(mqttQoSValue);
  }
  
  return true;
}



boolean connectMqttOptions(){
  Serial.println("START OPTION to MQTT server...");
  boolean result;
  net2.setInsecure();
  mqttClient.setWill(mqttStatusTopicValue, mqttLwtMessageValue, true, 0);
  Serial.println((String)"mqttClientID: " + mqttClientIDValue);
  Serial.println((String)"mqttUserNameValue: " + mqttUserNameValue);
  Serial.println((String)"mqttUserPasswordValue: " + mqttUserPasswordValue);
  if (mqttUserPasswordValue[0] != '\0') {
    result = mqttClient.connect(mqttClientIDValue, mqttUserNameValue, mqttUserPasswordValue);
  }else{
    result =false;
  }
  Serial.println("RESULTADO DE LA CONEXION: " + result);
  return result;
}