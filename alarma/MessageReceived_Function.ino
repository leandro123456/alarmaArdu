void CompleteRegister(String payload){

  Serial.println("PAYLOAD: "+ payload);
  StaticJsonDocument<500> jsonBuffer;
  deserializeJson(jsonBuffer, payload);

  String deviceIdObtenido = jsonBuffer["devid"];
  if(deviceIdObtenido.equals("password-validation-invalid") ||
      deviceIdObtenido.equals("password-validation-wrong")||
      deviceIdObtenido.equals("wrong-password") ||
      deviceIdObtenido.equals("invalid-deviceid")){
    correctPassword = false;
    return;
  }
  
  String mqttUserObtenido = jsonBuffer["mqttusr"];
  String mqttPasswordObtenido = jsonBuffer["mqttpwd"];
  String rootId = jsonBuffer["rootinfo"];

  //Configuraciones
  memset(deviceIdFinalValue, 0, sizeof deviceIdFinalValue);
  strncpy(deviceIdFinalValue, String(deviceIdObtenido).c_str(), String(deviceIdObtenido).length());

  memset(mqttUserNameValue, 0, sizeof mqttUserNameValue);
  strncpy(mqttUserNameValue, String(mqttUserObtenido).c_str(), String(mqttUserObtenido).length());

  memset(mqttUserPasswordValue, 0, sizeof mqttUserPasswordValue);
  strncpy(mqttUserPasswordValue, String(mqttPasswordObtenido).c_str(), String(mqttPasswordObtenido).length());

  memset(mqttClientIDValue, 0, sizeof mqttClientIDValue);
  strncpy(mqttClientIDValue, String(deviceIdObtenido+"CID").c_str(), String(deviceIdObtenido+"CID").length());

  //Topicos
  memset(mqttStatusTopicValue, 0, sizeof mqttStatusTopicValue);
  strncpy(mqttStatusTopicValue, String(rootId+"/"+deviceIdObtenido+"/Status").c_str(), String(rootId+"/"+deviceIdObtenido+"/Status").length());

  memset(mqttPartitionTopicValue, 0, sizeof mqttPartitionTopicValue);
  strncpy(mqttPartitionTopicValue, String(rootId+"/"+deviceIdObtenido +"/Partition").c_str(), String(rootId+"/"+deviceIdObtenido +"/Partition").length());

  memset(mqttActivePartitionTopicValue, 0, sizeof mqttActivePartitionTopicValue);
  strncpy(mqttActivePartitionTopicValue, String(rootId+"/"+deviceIdObtenido +"/activePartition").c_str(), String(rootId+"/"+deviceIdObtenido +"/activePartition").length());

  memset(mqttZoneTopicValue, 0, sizeof mqttZoneTopicValue);
  strncpy(mqttZoneTopicValue, String(rootId+"/"+deviceIdObtenido +"/Zone").c_str(), String(rootId+"/"+deviceIdObtenido +"/Zone").length());

  memset(mqttFireTopicValue, 0, sizeof mqttFireTopicValue);
  strncpy(mqttFireTopicValue, String(rootId+"/"+deviceIdObtenido +"/Fire").c_str(), String(rootId+"/"+deviceIdObtenido +"/Fire").length());

  memset(mqttTroubleTopicValue, 0, sizeof mqttTroubleTopicValue);
  strncpy(mqttTroubleTopicValue, String(rootId+"/"+deviceIdObtenido +"/Trouble").c_str(), String(rootId+"/"+deviceIdObtenido +"/Trouble").length());

  memset(mqttCommandTopicValue, 0, sizeof mqttCommandTopicValue);
  strncpy(mqttCommandTopicValue, String(rootId+"/"+deviceIdObtenido +"/cmd").c_str(), String(rootId+"/"+deviceIdObtenido +"/cmd").length());

  memset(mqttKeepAliveTopicValue, 0, sizeof mqttKeepAliveTopicValue);
  strncpy(mqttKeepAliveTopicValue, String(rootId+"/"+deviceIdObtenido +"/keepAlive").c_str(), String(rootId+"/"+deviceIdObtenido +"/keepAlive").length());

  memset(monitoringTopicValue, 0, sizeof monitoringTopicValue);
  strncpy(monitoringTopicValue, String(rootId+"/"+deviceIdObtenido+"/MNTR").c_str(), String(rootId+"/"+deviceIdObtenido+"/MNTR").length());

  iotWebConf.setupUpdateServer(
    [](const char *updatePath)
    { httpUpdater.setup(&server, updatePath); },
    [](const char *userName, char *password)
    { httpUpdater.updateCredentials(userName, password); });
  iotWebConf.saveConfig();
}

void mqttMessageReceived(char* topic, byte* payload, unsigned int length) {
  String topico=String(topic);
  Serial.println("Message received: " +topico+ " **-** " + String((char *)payload));

    if (topico == mqttStatusTopicValue) lastSentStatus = String((char *)payload);
    String valMes="Message received: " +topico+ " **-** " + String((char *)payload)+" **";
    Serial.println(valMes);

    char HomeAssitanConfTopic[STRING_LEN];
    String HomeAssitanValue= String(mqttKeepAliveTopicValue)+"/debug1";
    memset(HomeAssitanConfTopic, 0, sizeof HomeAssitanConfTopic);
    strncpy(HomeAssitanConfTopic, HomeAssitanValue.c_str(), HomeAssitanValue.length());
    mqttClient.publish(HomeAssitanConfTopic,valMes.c_str(), (bool) atoi(mqttRetainValue));
      

    String valor= String((char *)payload);
    String partitionN = valor.substring(0,1);
    String acTion = valor.substring(1,2);
    Serial.println("partitionN: " + partitionN + " - acTion: " + acTion);
    byte partitionrr = 0;
    byte payloadIndexrr = 0;

    if (partitionN >= "0" && partitionN <= "8" && (acTion == "S" || acTion == "A" || acTion == "D" ) ){
      Serial.println("Entro en acciones tipo de accion: "+ acTion);

      // Arm stay
      if (acTion == "S" && !dsc.armed[partitionN.toInt()-1] && !dsc.exitDelay[partitionN.toInt()-1]) {
        controlDSC("arm_stay",partitionN.toInt());
      }
    
      // Arm away 
      else if (acTion == "A" && !dsc.armed[partitionN.toInt()-1] && !dsc.exitDelay[partitionN.toInt()-1]) {
        controlDSC("arm_away",partitionN.toInt());
      }
          
      // Disarm
      else if (acTion == "D" && (dsc.armed[partitionN.toInt()-1] || dsc.exitDelay[partitionN.toInt()-1])) {
        controlDSC("disarm",partitionN.toInt());
      }     
    }
    else {//Trarammiento no documentado de PINES 1(14) y 2(13)
      acTion = valor.substring(1,5);
      boolean NoEntro=true;
      if(NoEntro){
        if (acTion == "P2H1s"){
          digitalWrite(13,HIGH);
          delay(1000);
          digitalWrite(13,LOW);
          NoEntro=false;
        }
        if (acTion == "P2L1s"){
          digitalWrite(13,LOW);
          delay(1000);
          digitalWrite(13,HIGH);
          NoEntro=false;
        }

        if (acTion == "P1H1s"){
          digitalWrite(14,HIGH);
          delay(1000);
          digitalWrite(14,LOW);
          NoEntro=false;
        }

        if (acTion == "P1L1s"){
          digitalWrite(14,LOW);
          delay(1000);
          digitalWrite(14,HIGH);
          NoEntro=false;
        }
      }
      if(NoEntro){
        acTion = valor.substring(1,4);
        if (acTion == "P2H"){ 
          digitalWrite(13,HIGH);
        }
        if (acTion == "P2L"){ 
          digitalWrite(13,LOW);
        }      
        if (acTion == "P1H"){
          digitalWrite(14,HIGH);
        }       
        if (acTion == "P1L"){ //Pone el Pin1 en LOW
          digitalWrite(14,LOW);
        }
        if (acTion != "P2H" && acTion != "P2L" && acTion != "P1H" && acTion != "P1L"){
          dsc.write(partitionN.c_str());  //TODO Chequear por que no funciona con panic y con auxiliar 
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
}
