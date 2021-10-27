void mqttMessageReceived(char* topic, byte* payload, unsigned int length) {
/* --------------- SerialDebug: --------- */
Serial.println("########################  Message received: " +String(topic) + " - " + String((char *)payload));
/* --------------- SerialDebug: --------- */
/* --------------- mqttDebug: --------- */
if (atoi(enableMqttDebugValue) == 1) {
  String msgtext= (String) deviceIdValue + " - Message received: Topic: " + topic + " Payload: " + String((char *)payload);
  msgtext.toCharArray(textmessage, STRING_LEN);
  mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
/* --------------- mqttDebug: --------- */
   
// Actualiza con el ltimo estado publicado
if (topic == mqttStatusTopicValue) lastSentStatus = String((char *)payload);

if (topic == mqttCommandTopicValue){

String partitionN = String((char *)payload).substring(0,1);
String acTion = String((char *)payload).substring(1);    

Serial.println("partitionN: " + partitionN);
Serial.println("acTion: " + acTion);


  if (partitionN >= "0" && partitionN <= "8" && (acTion == "S" || acTion == "A" || acTion == "D" ) ){
  /* --------------- mqttDebug: --------- */
  //if (atoi(enableMqttDebugValue) == 1) {mqttClient.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Inside IF - Decode: Partition: " + partitionN + " Action: " + acTion, (bool) atoi(remoteConfigRetainValue));}
  /* --------------- mqttDebug: --------- */
  
Serial.println("entro en el IF");
 
            // Arm stay  REEMPLAZAR ESTO POR controlDSC("arm_stay",partitionN.toInt());
            if (acTion == "S" && !dsc.armed[partitionN.toInt()-1] && !dsc.exitDelay[partitionN.toInt()-1]) {
              //while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              //dsc.writePartition = partitionN.toInt() + 1;         // Sets writes to the partition number
              dsc.writePartition = partitionN.toInt();         // Sets writes to the partition number
              dsc.write('s');                             // Virtual keypad arm stay
            
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {
              String msgtext=(String) deviceIdValue + " ARM_STAY called";
              msgtext.toCharArray(textmessage, STRING_LEN);
              mqttClient.publish(MqttDebugTopicValue, textmessage, (bool) atoi(remoteConfigRetainValue));}
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Arm away REEMPLAZAR ESTO POR controlDSC("arm_away",partitionN.toInt());
            else if (acTion == "A" && !dsc.armed[partitionN.toInt()-1] && !dsc.exitDelay[partitionN.toInt()-1]) {
              //while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              //dsc.writePartition = partitionN.toInt() + 1;         // Sets writes to the partition number
              dsc.writePartition = partitionN.toInt();         // Sets writes to the partition number
              dsc.write('w');                             // Virtual keypad arm away

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {
              String msgtext=(String) deviceIdValue + " ARM_AWAY called";
              msgtext.toCharArray(textmessage, STRING_LEN);
              mqttClient.publish(MqttDebugTopicValue, textmessage, (bool) atoi(remoteConfigRetainValue));}
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Disarm  REEMPLAZAR ESTO POR controlDSC("disarm",partitionN.toInt());
            else if (acTion == "D" && (dsc.armed[partitionN.toInt()-1] || dsc.exitDelay[partitionN.toInt()-1])) {
              //while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              //dsc.writePartition = partitionN.toInt() + 1;         // Sets writes to the partition number
              dsc.writePartition = partitionN.toInt();         // Sets writes to the partition number
              dsc.write(accessCodeValue);

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {
              String msgtext= (String) deviceIdValue + " Access code sent to DISARM";
              msgtext.toCharArray(textmessage, STRING_LEN);
              mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
            /* --------------- mqttDebug: --------- */
            }
            
          }else {    //Trarammiento no documentado de PINES 1(14) y 2(13)
                
                if (String((char *)payload) == "P2H"){   //Pone el Pin2 en High
                  digitalWrite(13,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 in now HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {
                    String msgtext= (String) deviceIdValue + " Pin2 in now HIGH.";
                    msgtext.toCharArray(textmessage, STRING_LEN);
                    mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (String((char *)payload) == "P2H1s"){ //Pone el Pin2 en High y despus de un segundo lo pone en LOW
                  digitalWrite(13,HIGH);
                  delay(1000);
                  digitalWrite(13,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 was put to HIGH and now is LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {
                    String msgtext= (String) deviceIdValue + " Pin2 was put to HIGH and now is LOW.";
                    msgtext.toCharArray(textmessage, STRING_LEN);
                    mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                  /* --------------- mqttDebug: --------- */
                }
                
                if (String((char *)payload) == "P2L"){ //Pone el Pin2 en LOW
                  digitalWrite(13,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 in now LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {
                    String msgtext=(String) deviceIdValue + " Pin2 in now LOW.";
                    msgtext.toCharArray(textmessage, STRING_LEN);
                    mqttClient.publish(MqttDebugTopicValue, textmessage, (bool) atoi(remoteConfigRetainValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (String((char *)payload) == "P2L1s"){ //Pone el Pin2 en LOW y despus de un segundo lo pone en HIGH
                  digitalWrite(13,LOW);
                  delay(1000);
                  digitalWrite(13,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin2 was put to LOW and now is HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {
                    String msgtext= (String) deviceIdValue + " Pin2 was put to LOW and now is HIGH.";
                    msgtext.toCharArray(textmessage, STRING_LEN);
                    mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                  /* --------------- mqttDebug: --------- */
                }
                
                if (String((char *)payload) == "P1H"){  //Pone el Pin1 en HIGH
                  digitalWrite(14,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 in now HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {
                    String msgtext= (String) deviceIdValue + " Pin1 in now HIGH.";
                    msgtext.toCharArray(textmessage, STRING_LEN);
                    mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (String((char *)payload) == "P1H1s"){ //Pone el Pin1 en HIGH y despus de un segundo lo pone en LOW
                  digitalWrite(14,HIGH);
                  delay(1000);
                  digitalWrite(14,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 was put to HIGH and now is LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {
                    String msgtext=(String) deviceIdValue + " Pin1 was put to HIGH and now is LOW";
                    msgtext.toCharArray(textmessage, STRING_LEN);
                    mqttClient.publish(MqttDebugTopicValue,textmessage , (bool) atoi(remoteConfigRetainValue));}
                  /* --------------- mqttDebug: --------- */
                }
                
                if (String((char *)payload) == "P1L"){ //Pone el Pin1 en LOW
                  digitalWrite(14,LOW);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 in now LOW");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {
                    String msgtext= (String) deviceIdValue + " Pin1 in now LOW.";
                    msgtext.toCharArray(textmessage, STRING_LEN);
                    mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                  /* --------------- mqttDebug: --------- */
                }

                if (String((char *)payload) == "P1L1s"){ //Pone el Pin1 en LOW y despus de un segundo lo pone en HIGH
                  digitalWrite(14,LOW);
                  delay(1000);
                  digitalWrite(14,HIGH);
                  
                  /* --------------- SerialDebug: --------- */
                  Serial.println("Pin1 was put to LOW and now is HIGH");
                  /* --------------- SerialDebug: --------- */
                  /* --------------- mqttDebug: --------- */
                  if (atoi(enableMqttDebugValue) == 1) {
                    String msgtext=(String) deviceIdValue + " Pin1 was put to LOW and now is HIGH";
                    msgtext.toCharArray(textmessage, STRING_LEN);
                    mqttClient.publish(MqttDebugTopicValue,textmessage , (bool) atoi(remoteConfigRetainValue));}
                  /* --------------- mqttDebug: --------- */
                }

            
            if (String((char *)payload) != "P2H" && String((char *)payload) != "P2L" && String((char *)payload) != "P1H" && String((char *)payload) != "P1L")
              dsc.write((char *)payload);  //Chequear por que no funciona con panic y con auxiliar
            
            
          }
          
   }
}
