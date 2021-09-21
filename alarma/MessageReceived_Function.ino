void mqttMessageReceived(String &topic, String &payload){
/* --------------- SerialDebug: --------- */
Serial.println("########################  Message received: " + topic + " - " + payload);
/* --------------- SerialDebug: --------- */
/* --------------- mqttDebug: --------- */
if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Message received: Topic: " + topic + " Payload: " + payload, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
/* --------------- mqttDebug: --------- */
   
// Actualiza con el ltimo estado publicado
if (topic == mqttStatusTopicValue) lastSentStatus = payload;

if (topic == mqttCommandTopicValue){

String partitionN = payload.substring(0,1);
String acTion = payload.substring(1);    

Serial.println("partitionN: " + partitionN);
Serial.println("acTion: " + acTion);


  if (partitionN >= "0" && partitionN <= "8" && (acTion == "S" || acTion == "A" || acTion == "D" ) ){
  /* --------------- mqttDebug: --------- */
  //if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Inside IF - Decode: Partition: " + partitionN + " Action: " + acTion, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  /* --------------- mqttDebug: --------- */
  
Serial.println("entro en el IF");
 
            // Arm stay  REEMPLAZAR ESTO POR controlDSC("arm_stay",partitionN.toInt());
            if (acTion == "S" && !dsc.armed[partitionN.toInt()-1] && !dsc.exitDelay[partitionN.toInt()-1]) {
              //while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              //dsc.writePartition = partitionN.toInt() + 1;         // Sets writes to the partition number
              dsc.writePartition = partitionN.toInt();         // Sets writes to the partition number
              dsc.write('s');                             // Virtual keypad arm stay
            
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " ARM_STAY called", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Arm away REEMPLAZAR ESTO POR controlDSC("arm_away",partitionN.toInt());
            else if (acTion == "A" && !dsc.armed[partitionN.toInt()-1] && !dsc.exitDelay[partitionN.toInt()-1]) {
              //while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              //dsc.writePartition = partitionN.toInt() + 1;         // Sets writes to the partition number
              dsc.writePartition = partitionN.toInt();         // Sets writes to the partition number
              dsc.write('w');                             // Virtual keypad arm away

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " ARM_AWAY called", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Disarm  REEMPLAZAR ESTO POR controlDSC("disarm",partitionN.toInt());
            else if (acTion == "D" && (dsc.armed[partitionN.toInt()-1] || dsc.exitDelay[partitionN.toInt()-1])) {
              //while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              //dsc.writePartition = partitionN.toInt() + 1;         // Sets writes to the partition number
              dsc.writePartition = partitionN.toInt();         // Sets writes to the partition number
              dsc.write(accessCodeValue);

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Access code sent to DISARM", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            }
            
          }else {    //Trarammiento no documentado de PINES 1(14) y 2(13)
                
                if (payload == "P2H"){   //Pone el Pin2 en High
                  digitalWrite(13,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 in now HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin2 in now HIGH.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (payload == "P2H1s"){ //Pone el Pin2 en High y despus de un segundo lo pone en LOW
                  digitalWrite(13,HIGH);
                  delay(1000);
                  digitalWrite(13,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 was put to HIGH and now is LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin2 was put to HIGH and now is LOW.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }
                
                if (payload == "P2L"){ //Pone el Pin2 en LOW
                  digitalWrite(13,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 in now LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin2 in now LOW.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (payload == "P2L1s"){ //Pone el Pin2 en LOW y despus de un segundo lo pone en HIGH
                  digitalWrite(13,LOW);
                  delay(1000);
                  digitalWrite(13,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 was put to LOW and now is HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin2 was put to LOW and now is HIGH.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }
                
                if (payload == "P1H"){  //Pone el Pin1 en HIGH
                  digitalWrite(14,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 in now HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin1 in now HIGH.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (payload == "P1H1s"){ //Pone el Pin1 en HIGH y despus de un segundo lo pone en LOW
                  digitalWrite(14,HIGH);
                  delay(1000);
                  digitalWrite(14,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 was put to HIGH and now is LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin1 was put to HIGH and now is LOW", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }
                
                if (payload == "P1L"){ //Pone el Pin1 en LOW
                  digitalWrite(14,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 in now LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin1 in now LOW.", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (payload == "P1L1s"){ //Pone el Pin1 en LOW y despus de un segundo lo pone en HIGH
                  digitalWrite(14,LOW);
                  delay(1000);
                  digitalWrite(14,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 was put to LOW and now is HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Pin1 was put to LOW and now is HIGH", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                  /* --------------- mqttDebug: --------- */
                }

            
            if (payload != "P2H" && payload != "P2L" && payload != "P1H" && payload != "P1L") dsc.write(payload.c_str());  //Chequear por que no funciona con panic y con auxiliar
            
            
          }
          
   }
}
