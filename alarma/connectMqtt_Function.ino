boolean connectMqtt() {
  unsigned long now = millis();
  if ((lastMqttConnectionAttempt + 1000) > now)
  {
    return false;
  }

  Serial.println("Connecting to MQTT server...");

  if (!connectMqttOptions()) {
    lastMqttConnectionAttempt = now;
    return false;
  }

  publicaEstados();
  mqttClient.subscribe(mqttCommandTopicValue);
  Serial.println((String) "Subcribed to: " + mqttCommandTopicValue);
  return true;
}


boolean connectMqttOptions(){
  boolean result;
  net2.setInsecure();
  if(String(deviceIdFinalValue).equals("empty") || String(deviceIdFinalValue).equals("")){
    Serial.println((String)"mqtt Conection - UserName: " + mqttUserNameValue);
    result = mqttClient.connect(clientIDrnd,mqttUserNameValue, mqttUserPasswordValue);
    return result;
  }
  else{
    Serial.println((String)"mqtt USER: " + String(mqttUserNameValue));
    result = mqttClient.connect(mqttClientIDValue,mqttUserNameValue, mqttUserPasswordValue,mqttStatusTopicValue,0,true,mqttLwtMessageValue);
    return result;
  }
}