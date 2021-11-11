boolean connectMqtt() {
  unsigned long now = millis();
  if ((lastMqttConnectionAttempt + 1000) > now)
  {
    // Do not repeat within 1 sec.
    return false;
  }
   //--------------- SerialDebug: --------- 
  Serial.println("Connecting to MQTT server...");
  if (atoi(enableMqttDebugValue) == 1) {
    String msgtext= deviceID + " - Connecting to MQTT server...";
    mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
  }

  if (!connectMqttOptions()) {
    Serial.println("entro a connectMqttOptions VA A a salir con false");
    lastMqttConnectionAttempt = now;
    return false;
  }

  //--------------- SerialDebug: --------- 
  Serial.println("Connected MQTT server DEBUG");
  // --------------- mqttDebug: --------- 
  if (atoi(enableMqttDebugValue) == 1) {
    String msgtext= deviceID + " - Connected to MQTT server!";
    mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
  }
  
  if (dsc.keybusConnected) {
    mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true);
    lastSentStatus = String(mqttBirthMessageValue);

    //--------------- SerialDebug: --------- 
    Serial.println("Status message published: " + lastSentStatus);
    // --------------- mqttDebug: --------- 
    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext=deviceID + " - Status message published: " + lastSentStatus;
      mqttClient.publish(MqttDebugTopicValue, msgtext.c_str(), (bool) atoi(mqttRetainValue));}
  }else {
    mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true);
    lastSentStatus = String(mqttNoDSCValue);

    //--------------- SerialDebug: --------- 
    Serial.println("Status message published: " + lastSentStatus);
    // --------------- mqttDebug: --------- 
    if (atoi(enableMqttDebugValue) == 1) {
      String msgtext= deviceID + " - Status message published: " + lastSentStatus;
      mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
    }
  }
  // Para publicar estados iniciales
  publicaEstados();

  //Subcribe al topic de comando
  mqttClient.subscribe(mqttCommandTopicValue);
  
  //--------------- SerialDebug: --------- 
  Serial.println((String) "Subcribed to: " + mqttCommandTopicValue);
  // --------------- mqttDebug: --------- 
  if (atoi(enableMqttDebugValue) == 1) {
    String msgtext= deviceID + " - Subcribed to: " + mqttCommandTopicValue;
    mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool)atoi(mqttRetainValue));
  }

  return true;
}



boolean connectMqttOptions(){
    Serial.println("START OPTION to MQTT server...");
    boolean result;
    net2.setInsecure();
    Serial.println((String)"mqttClientID: " + mqttClientIDValue);
    Serial.println((String)"mqttUserNameValue: " + mqttUserNameValue);
    Serial.println((String)"mqttUserPasswordValue: " + mqttUserPasswordValue);
    if (mqttUserPasswordValue[0] != '\0') {
      result = mqttClient.connect(mqttClientIDValue,mqttUserNameValue, mqttUserPasswordValue,mqttStatusTopicValue,0,true,mqttLwtMessageValue);
    }else{
      result =false;
    }
    Serial.println("RESULTADO DE LA CONEXION: " + (String)result);
    return result;
}
