boolean connectMqtt() {
  unsigned long now = millis();
  if ((lastMqttConnectionAttempt + 1000) > now)
  {
    return false;
  }

  Serial.println("Connecting to MQTT server...");
  if (atoi(enableMqttDebugValue) == 1) {
    String msgtext= String(deviceIdFinalValue) + " - Connecting to MQTT server...";
    mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
  }

  if (!connectMqttOptions()) {
    lastMqttConnectionAttempt = now;
    return false;
  }

  //--------------- SerialDebug: --------- 
  if (atoi(enableMqttDebugValue) == 1) {
    String msgtext= String(deviceIdFinalValue) + " - Connected to MQTT server!";
    mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
  }
  
  if (dsc.keybusConnected) {
    mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true);
    lastSentStatus = String(mqttBirthMessageValue);

    Serial.println("Status message published: " + lastSentStatus);
    // --------------- mqttDebug: --------- 
    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext=String(deviceIdFinalValue) + " - Status message published: " + lastSentStatus;
      mqttClient.publish(MqttDebugTopicValue, msgtext.c_str(), (bool) atoi(mqttRetainValue));}
  }else {
    mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true);
    lastSentStatus = String(mqttNoDSCValue);

    //--------------- SerialDebug: --------- 
    Serial.println("Status message published: " + lastSentStatus);

    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext= String(deviceIdFinalValue) + " - Status message published: " + lastSentStatus;
      mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
    }
  }

  publicaEstados();
  mqttClient.subscribe(mqttCommandTopicValue);
  /* --------------- SerialDebug: --------- */
  Serial.println((String) "Subcribed to: " + mqttCommandTopicValue);
  /* --------------- mqttDebug: --------- */
  if (atoi(enableMqttDebugValue) == 1) {
    String msgtext= String(deviceIdFinalValue) + " - Subcribed to: " + mqttCommandTopicValue;
    mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool)atoi(mqttRetainValue));
  }
  return true;
}


boolean connectMqttOptions(){
  boolean result;
  net2.setInsecure();
  if(String(deviceIdFinalValue).equals("empty") || String(deviceIdFinalValue).equals("")){
    Serial.println((String)"mqttClientID: " + clientIDrnd);
    Serial.println((String)"mqttUserNameValue: " + mqttUserNameValue);
    Serial.println((String)"mqttUserPasswordValue: " + mqttUserPasswordValue);
    result = mqttClient.connect(clientIDrnd,mqttUserNameValue, mqttUserPasswordValue);
    Serial.println("resulttado1 coneccion: "+ (String)result);
    return result;
  }
  else{
    Serial.println((String)"mqtt USER: " + String(mqttUserNameValue));
    Serial.println((String)"mqtt PASS: " + String(mqttUserPasswordValue));
    String clientId = "DSC01Clt";
    clientId += String(random(0xffff), HEX);
    Serial.println((String)"mqttClientID: " + clientId.c_str());
    result = mqttClient.connect(clientId.c_str(),mqttUserNameValue, mqttUserPasswordValue,mqttStatusTopicValue,0,true,mqttLwtMessageValue);
    return result;
  }
}