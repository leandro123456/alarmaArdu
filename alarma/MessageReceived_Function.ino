
void mqttMessageReceived(String &topic, String &payload){
  // --------------- SerialDebug: ----------
  Serial.println("llego el mensaje!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  Serial.println("Message received: " + String(topic) + " - ");
  Serial.println("Payload: "+ String(payload));
  // --------------- mqttDebug: --------- 
  if (atoi(enableMqttDebugValue) == 1) {
    mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " - Message received: Topic: " + topic + " Payload: " + payload, (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
  }

  if(topic==mqttDeviceConfigResponseValue){
    Serial.println("encontro el topico para finalizar el registro de un nuevo usuario!!");
    Serial.println(payload);
    StaticJsonDocument<500> jsonBuffer;  // estaba en 300 pero lo subo a 500 porque no entraban los comments

    deserializeJson(jsonBuffer, payload);
    String deviceIdObtenido = jsonBuffer["clt"];
    String mqttPasswordObtenido = jsonBuffer["mqttpdw"];

    Serial.println("deviceId obtenido: "+ deviceIdObtenido);
    Serial.println("Password obtenido: "+ mqttPasswordObtenido);
    
    //Configuraciones
    Serial.println("*******************  Configuracion FINAL del dispositivo");
    memset(deviceIdFinalValue, 0, sizeof deviceIdFinalValue);
    strncpy(deviceIdFinalValue, String(deviceIdObtenido).c_str(), String(deviceIdObtenido).length());
    Serial.println(">>>>>>>>>>>>>>>>>>>> El valor de la respuesta ID: -"+ String(deviceIdFinalValue)+"-fin");

    memset(mqttUserNameValue, 0, sizeof mqttUserNameValue);
    strncpy(mqttUserNameValue, String(deviceIdObtenido).c_str(), String(deviceIdObtenido).length());

    memset(mqttUserPasswordValue, 0, sizeof mqttUserPasswordValue);
    strncpy(mqttUserPasswordValue, String(mqttPasswordObtenido).c_str(), String(mqttPasswordObtenido).length());

    memset(mqttClientIDValue, 0, sizeof mqttClientIDValue);
    strncpy(mqttClientIDValue, String(deviceIdObtenido+"CID").c_str(), String(deviceIdObtenido+"CID").length());

    //Topicos
    memset(mqttStatusTopicValue, 0, sizeof mqttStatusTopicValue);
    strncpy(mqttStatusTopicValue, String(deviceIdObtenido+"/Status").c_str(), String(deviceIdObtenido+"/Status").length());

    memset(mqttPartitionTopicValue, 0, sizeof mqttPartitionTopicValue);
    strncpy(mqttPartitionTopicValue, String(deviceIdObtenido +"/Patition").c_str(), String(deviceIdObtenido +"/Patition").length());

    memset(mqttActivePartitionTopicValue, 0, sizeof mqttActivePartitionTopicValue);
    strncpy(mqttActivePartitionTopicValue, String(deviceIdObtenido +"/activePartition").c_str(), String(deviceIdObtenido +"/activePartition").length());
    
    memset(mqttZoneTopicValue, 0, sizeof mqttZoneTopicValue);
    strncpy(mqttZoneTopicValue, String(deviceIdObtenido +"/Zone").c_str(), String(deviceIdObtenido +"/Zone").length());

    memset(mqttFireTopicValue, 0, sizeof mqttFireTopicValue);
    strncpy(mqttFireTopicValue, String(deviceIdObtenido +"/Fire").c_str(), String(deviceIdObtenido +"/Fire").length());

    memset(mqttTroubleTopicValue, 0, sizeof mqttTroubleTopicValue);
    strncpy(mqttTroubleTopicValue, String(deviceIdObtenido +"/Trouble").c_str(), String(deviceIdObtenido +"/Trouble").length());

    memset(mqttCommandTopicValue, 0, sizeof mqttCommandTopicValue);
    strncpy(mqttCommandTopicValue, String(deviceIdObtenido +"/cmd").c_str(), String(deviceIdObtenido +"/cmd").length());

    memset(mqttKeepAliveTopicValue, 0, sizeof mqttKeepAliveTopicValue);
    strncpy(mqttKeepAliveTopicValue, String(deviceIdObtenido +"/keepAlive").c_str(), String(deviceIdObtenido +"/keepAlive").length());

    memset(monitoringTopicValue, 0, sizeof monitoringTopicValue);
    strncpy(monitoringTopicValue, String("MNTR/"+deviceIdObtenido).c_str(), String("MNTR/"+deviceIdObtenido).length());

    memset(MqttDebugTopicValue, 0, sizeof MqttDebugTopicValue);
    strncpy(MqttDebugTopicValue, String("RMgmt/"+deviceIdObtenido+"/debug").c_str(), String("RMgmt/"+deviceIdObtenido+"/debug").length());
   

    iotWebConf.setupUpdateServer(
            [](const char *updatePath)
            { httpUpdater.setup(&server, updatePath); },
            [](const char *userName, char *password)
            { httpUpdater.updateCredentials(userName, password); });

    iotWebConf.saveConfig();

    Serial.println(">>>>>>>>>>>>>>>>> El valor de la respuesta ID2: -"+ String(deviceIdFinalValue)+"-fin");


  }
  else{
    // Actualiza con el tiempo estado publicado
    if (topic == mqttStatusTopicValue) lastSentStatus = payload;

    if (topic == mqttCommandTopicValue){

      String partitionN = payload.substring(0,1);
      String acTion = payload.substring(1);    
      Serial.println("partitionN: " + partitionN + "; acTion: " + acTion);

      if (partitionN >= "0" && partitionN <= "8" && (acTion == "S" || acTion == "A" || acTion == "D" ) ){
        Serial.println("entro en el IF");
        // --------------- mqttDebug: --------- 
        if (atoi(enableMqttDebugValue) == 1) {
          mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " - Inside IF - Decode: Partition: " + partitionN + " Action: " + acTion, (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
        }
      
        // Arm stay
        if (acTion == "S" && !dsc.armed[partitionN.toInt()-1] && !dsc.exitDelay[partitionN.toInt()-1]) {
          controlDSC("arm_stay",partitionN.toInt());
        }
        // Arm away REEMPLAZAR ESTO POR 
        else if (acTion == "A" && !dsc.armed[partitionN.toInt()-1] && !dsc.exitDelay[partitionN.toInt()-1]) {
          controlDSC("arm_away",partitionN.toInt());
        }
        // Disarm  REEMPLAZAR ESTO POR 
        else if (acTion == "D" && (dsc.armed[partitionN.toInt()-1] || dsc.exitDelay[partitionN.toInt()-1])) {
          controlDSC("disarm",partitionN.toInt());
        }
      }else {//Trarammiento no documentado de PINES 1(14) y 2(13)
        if (payload == "P2H"){   //Pone el Pin2 en High
          digitalWrite(13,HIGH);
          // --------------- SerialDebug: ----------
          Serial.println("Pin2 in now HIGH");
          // --------------- mqttDebug: --------- 
          if (atoi(enableMqttDebugValue) == 1) {
            mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " Pin2 in now HIGH.", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
          }
        }
        if (payload == "P2H1s"){ //Pone el Pin2 en High y despus de un segundo lo pone en LOW
          digitalWrite(13,HIGH);
          delay(1000);
          digitalWrite(13,LOW);
          // --------------- SerialDebug: ----------
          Serial.println("Pin2 was put to HIGH and now is LOW");
          // --------------- mqttDebug: --------- 
          if (atoi(enableMqttDebugValue) == 1) {
            mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " Pin2 was put to HIGH and now is LOW.", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
          }
        }
        if (payload == "P2L"){ //Pone el Pin2 en LOW
          digitalWrite(13,LOW);
          // --------------- SerialDebug: ----------
          Serial.println("Pin2 in now LOW");
          // --------------- mqttDebug: --------- 
          if (atoi(enableMqttDebugValue) == 1) {
            mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " Pin2 in now LOW.", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
          }
        }
        if (payload == "P2L1s"){ //Pone el Pin2 en LOW y despus de un segundo lo pone en HIGH
          digitalWrite(13,LOW);
          delay(1000);
          digitalWrite(13,HIGH);
          // --------------- SerialDebug: ----------
          Serial.println("Pin2 was put to LOW and now is HIGH");
          // --------------- mqttDebug: --------- 
          if (atoi(enableMqttDebugValue) == 1) {
            mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " Pin2 was put to LOW and now is HIGH.", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
          }
        }
        if (payload == "P1H"){  //Pone el Pin1 en HIGH
          digitalWrite(14,HIGH);
          // --------------- SerialDebug: ----------
          Serial.println("Pin1 in now HIGH");
          // --------------- mqttDebug: --------- 
          if (atoi(enableMqttDebugValue) == 1) {
            mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " Pin1 in now HIGH.", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
          }
        }
        if (payload == "P1H1s"){ //Pone el Pin1 en HIGH y despus de un segundo lo pone en LOW
          digitalWrite(14,HIGH);
          delay(1000);
          digitalWrite(14,LOW);
          // --------------- SerialDebug: ----------
          Serial.println("Pin1 was put to HIGH and now is LOW");
          // --------------- mqttDebug: --------- 
          if (atoi(enableMqttDebugValue) == 1) {
            mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " Pin1 was put to HIGH and now is LOW", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
          }
        }
        if (payload == "P1L"){ //Pone el Pin1 en LOW
          digitalWrite(14,LOW);
          // --------------- SerialDebug: ----------
          Serial.println("Pin1 in now LOW");
          // --------------- mqttDebug: --------- 
          if (atoi(enableMqttDebugValue) == 1) {
            mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " Pin1 in now LOW.", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
          }
        }
        if (payload == "P1L1s"){ //Pone el Pin1 en LOW y despus de un segundo lo pone en HIGH
          digitalWrite(14,LOW);
          delay(1000);
          digitalWrite(14,HIGH);
          // --------------- SerialDebug: ----------
          Serial.println("Pin1 was put to LOW and now is HIGH");
          // --------------- mqttDebug: --------- 
          if (atoi(enableMqttDebugValue) == 1) {
            mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " Pin1 was put to LOW and now is HIGH", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
          }
        }      
        if (payload != "P2H" && payload != "P2L" && payload != "P1H" && payload != "P1L"){
          dsc.write(payload.c_str());  //Chequear por que no funciona con panic y con auxiliar
          // --------------- SerialDebug: ----------
          Serial.println(" Pin1 AND Pin2 was failed: "+ payload);
          // --------------- mqttDebug: --------- 
          if (atoi(enableMqttDebugValue) == 1) {
            mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " Pin1 AND Pin2 was failed: "+ payload.c_str(), (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
          }
        } 
      }      
    }
  }
}


void controlDSC(String coMMand, int targetPartition){
  // Arm stay
  if (coMMand == "arm_stay") {
    dsc.writePartition = targetPartition;         // Sets writes to the partition number
    dsc.write('s');                               // Virtual keypad arm stay
  }
  // Arm away
  else if (coMMand == "arm_away") {
    dsc.writePartition = targetPartition;         // Sets writes to the partition number
    dsc.write('w');                               // Virtual keypad arm away
  }
  // Disarm
  else if (coMMand == "disarm") {
    dsc.writePartition = targetPartition;         // Sets writes to the partition number
    dsc.write(accessCodeValue);                   // Virtual keypad Disarm
  }

  // --------------- mqttDebug: --------- 
  if (atoi(enableMqttDebugValue) == 1) {
    mqttClient.publish(MqttDebugTopicValue, String(deviceIdFinalValue) + " "+ coMMand +" called", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
  }
}