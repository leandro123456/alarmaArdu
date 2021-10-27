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
    String msgtext= (String) deviceIdValue + " - Connecting to MQTT server...";
    msgtext.toCharArray(textmessage, STRING_LEN);
    mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));
     }

  if (!connectMqttOptions()) {
    Serial.println("entro a connectMqttOptions VA A a salir con false");
    lastMqttConnectionAttempt = now;
    return false;
  }

  /* --------------- SerialDebug: --------- */
  Serial.println("Connected MQTT server DEBUG");
  if (atoi(enableMqttDebugValue) == 1) {
    String msgtext= (String) deviceIdValue + " - Connected to MQTT server!";
    msgtext.toCharArray(textmessage, STRING_LEN);
    mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
  /* --------------- mqttDebug: --------- */
  
  if (dsc.keybusConnected) {
    mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true);
    lastSentStatus = String(mqttBirthMessageValue);

    /* --------------- SerialDebug: --------- */
    Serial.println("Status message published: " + lastSentStatus);
    /* --------------- SerialDebug: --------- */
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext=(String) deviceIdValue + " - Status message published: " + lastSentStatus;
      msgtext.toCharArray(textmessage, STRING_LEN);
      mqttClient.publish(MqttDebugTopicValue, textmessage, (bool) atoi(remoteConfigRetainValue));}
    /* --------------- mqttDebug: --------- */
  }else {
    mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true);
    lastSentStatus = String(mqttNoDSCValue);

    /* --------------- SerialDebug: --------- */
    Serial.println("Status message published: " + lastSentStatus);
    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext= (String) deviceIdValue + " - Status message published: " + lastSentStatus;
      msgtext.toCharArray(textmessage, STRING_LEN);
      mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
    /* --------------- mqttDebug: --------- */
  }
  // Para publicar estados iniciales
  publicaEstados();

  //Subcribe al topic de comando
  mqttClient.subscribe(mqttCommandTopicValue);
  
  /* --------------- SerialDebug: --------- */
  Serial.println((String) "Subcribed to: " + mqttCommandTopicValue);
  if (atoi(enableMqttDebugValue) == 1) {
    String msgtext= (String) deviceIdValue + " - Subcribed to: " + mqttCommandTopicValue;
    msgtext.toCharArray(textmessage, STRING_LEN);
    mqttClient.publish(MqttDebugTopicValue,textmessage, (bool)atoi(remoteConfigRetainValue));}
  /* --------------- mqttDebug: --------- */
 

  return true;
}



boolean connectMqttOptions(){
    Serial.println("START OPTION to MQTT server...");
    boolean result;
//    if (atoi(isSecurePort) == 1)
      net2.setInsecure();
//    mqttClient.setWill(mqttStatusTopicValue, mqttLwtMessageValue, true, 0);
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
