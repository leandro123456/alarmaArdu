void mqttMessageReceived(char* topic, byte* payload, unsigned int length) {
  String topico=String(topic);
  //--------------- SerialDebug: --------- 
  Serial.println("Message received: " +topico+ " **-** " + String((char *)payload));
  // --------------- mqttDebug: --------- 
  if (atoi(enableMqttDebugValue) == 1) {
    String msgtext= deviceID +  " - Message received: Topic: " + topico + " Payload: " + String((char *)payload);
    mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
  }
    
  // Actualiza con el ltimo estado publicado
  if (topico == mqttStatusTopicValue) lastSentStatus = String((char *)payload);

  Serial.println("va a comparar si son guales el tipico recibido: "+ String(topic) +": y el parametro del topico: "+ String(mqttCommandTopicValue));
  if (topico  == mqttCommandTopicValue){

    String partitionN = String((char *)payload).substring(0,1);
    String acTion = String((char *)payload).substring(1);    
    Serial.println("partitionN: " + partitionN + " - acTion: " + acTion);

    if (partitionN >= "0" && partitionN <= "8" && (acTion == "S" || acTion == "A" || acTion == "D" ) ){
      Serial.println("entro en el IF. tipo de accion: "+ acTion);
 
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
    }else {//Trarammiento no documentado de PINES 1(14) y 2(13)
      if (String((char *)payload) == "P2H"){   //Pone el Pin2 en High
        digitalWrite(13,HIGH);
        //--------------- SerialDebug: --------- 
        Serial.println("Pin2 in now HIGH");
        // --------------- mqttDebug: --------- 
        if (atoi(enableMqttDebugValue) == 1) {
          String msgtext= deviceID +  " Pin2 in now HIGH.";
          mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
        }
      }

      if (String((char *)payload) == "P2H1s"){ //Pone el Pin2 en High y despus de un segundo lo pone en LOW
        digitalWrite(13,HIGH);
        delay(1000);
        digitalWrite(13,LOW);
        //--------------- SerialDebug: --------- 
        Serial.println("Pin2 was put to HIGH and now is LOW");
        // --------------- mqttDebug: --------- 
        if (atoi(enableMqttDebugValue) == 1) {
          String msgtext= deviceID +  " Pin2 was put to HIGH and now is LOW.";
          mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));}
      } 

      if (String((char *)payload) == "P2L"){ //Pone el Pin2 en LOW
        digitalWrite(13,LOW);
        //--------------- SerialDebug: --------- 
        Serial.println("Pin2 in now LOW");
        // --------------- mqttDebug: --------- 
        if (atoi(enableMqttDebugValue) == 1) {
          String msgtext=deviceID +  " Pin2 in now LOW.";
          mqttClient.publish(MqttDebugTopicValue, msgtext.c_str(), (bool) atoi(mqttRetainValue));
        }
      }

      if (String((char *)payload) == "P2L1s"){ //Pone el Pin2 en LOW y despus de un segundo lo pone en HIGH
        digitalWrite(13,LOW);
        delay(1000);
        digitalWrite(13,HIGH);
        //--------------- SerialDebug: --------- 
        Serial.println("Pin2 was put to LOW and now is HIGH");
        // --------------- mqttDebug: --------- 
        if (atoi(enableMqttDebugValue) == 1) {
          String msgtext= deviceID +  " Pin2 was put to LOW and now is HIGH.";
          mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));}
      }
                
      if (String((char *)payload) == "P1H"){  //Pone el Pin1 en HIGH
        digitalWrite(14,HIGH);
        //--------------- SerialDebug: --------- 
        Serial.println("Pin1 in now HIGH");
        // --------------- mqttDebug: --------- 
        if (atoi(enableMqttDebugValue) == 1) {
          String msgtext= deviceID +  " Pin1 in now HIGH.";
          mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
        }
      }

      if (String((char *)payload) == "P1H1s"){ //Pone el Pin1 en HIGH y despus de un segundo lo pone en LOW
        digitalWrite(14,HIGH);
        delay(1000);
        digitalWrite(14,LOW);
        //--------------- SerialDebug: --------- 
        Serial.println("Pin1 was put to HIGH and now is LOW");
        // --------------- mqttDebug: --------- 
        if (atoi(enableMqttDebugValue) == 1) {
          String msgtext=deviceID +  " Pin1 was put to HIGH and now is LOW";
          mqttClient.publish(MqttDebugTopicValue,msgtext.c_str() , (bool) atoi(mqttRetainValue));}
      }
                
      if (String((char *)payload) == "P1L"){ //Pone el Pin1 en LOW
        digitalWrite(14,LOW);
        //--------------- SerialDebug: --------- 
        Serial.println("Pin1 in now LOW");
        // --------------- mqttDebug: --------- 
        if (atoi(enableMqttDebugValue) == 1) {
          String msgtext= deviceID +  " Pin1 in now LOW.";
          mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
        }
      }

      if (String((char *)payload) == "P1L1s"){ //Pone el Pin1 en LOW y despus de un segundo lo pone en HIGH
        digitalWrite(14,LOW);
        delay(1000);
        digitalWrite(14,HIGH);
        //--------------- SerialDebug: --------- 
        Serial.println("Pin1 was put to LOW and now is HIGH");
        // --------------- mqttDebug: --------- 
        if (atoi(enableMqttDebugValue) == 1) {
          String msgtext=deviceID +  " Pin1 was put to LOW and now is HIGH";
          mqttClient.publish(MqttDebugTopicValue,msgtext.c_str() , (bool) atoi(mqttRetainValue));}
      }

      if (String((char *)payload) != "P2H" && String((char *)payload) != "P2L" && String((char *)payload) != "P1H" && String((char *)payload) != "P1L")
        dsc.write((char *)payload);  //Chequear por que no funciona con panic y con auxiliar    
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
    String msgtext=deviceID + " "+ coMMand +" called";
    mqttClient.publish(MqttDebugTopicValue,msgtext.c_str(), (bool) atoi(mqttRetainValue));
  }
}