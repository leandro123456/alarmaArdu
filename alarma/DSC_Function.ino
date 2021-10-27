void doDSC(){     
  char* topicpartition;
            if (dsc.handlePanel() && dsc.statusChanged) {  // Processes data only when a valid Keybus command has been read
                dsc.statusChanged = false;                   // Reset the status tracking flag
            
                // If the Keybus data buffer is exceeded, the sketch is too busy to process all Keybus commands.  Call
                // handlePanel() more often, or increase dscBufferSize in the library: src/dscKeybusInterface.h
                if (dsc.bufferOverflow) Serial.println(F("Keybus buffer overflow"));
                dsc.bufferOverflow = false;


                // Checks if the interface is connected to the Keybus
                if (dsc.keybusChanged) {
                  dsc.keybusChanged = false;  // Resets the Keybus data status flag

                  if (atoi(forceSecureAllTrafficValue) != 1){
                    if (dsc.keybusConnected) {
                      mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true);
                      lastSentStatus = String(mqttBirthMessageValue);
                    }
                    else {
                      mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true);
                      lastSentStatus = String(mqttNoDSCValue);
                    }
                  }
                  
                  if (atoi(forceSecureAllTrafficValue) == 1){
                    if (dsc.keybusConnected) {
                      mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true);
                      lastSentStatus = String(mqttBirthMessageValue);
                    }
                    else {
                      mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true);
                      lastSentStatus = String(mqttNoDSCValue);
                    }
                  }
                  
                }
                
            
                // Sends the access code when needed by the panel for arming
                if (dsc.accessCodePrompt && dsc.writeReady) {
                  dsc.accessCodePrompt = false;
                  dsc.write(accessCodeValue);
                }
            
                if (dsc.troubleChanged) {
                  dsc.troubleChanged = false;  // Resets the trouble status flag
            
                  if (atoi(forceSecureAllTrafficValue) != 1){
                    if (dsc.trouble) mqttClient.publish(mqttTroubleTopicValue, "1", (bool) atoi(mqttRetainValue));
                    else mqttClient.publish(mqttTroubleTopicValue, "0", (bool) atoi(mqttRetainValue));
                  }
                  if (atoi(forceSecureAllTrafficValue) == 1){
                    if (dsc.trouble) mqttClient.publish(mqttTroubleTopicValue, "1", (bool) atoi(mqttRetainValue));
                    else mqttClient.publish(mqttTroubleTopicValue, "0", (bool) atoi(mqttRetainValue));
                  }
                  
                  //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
                        String troubletopic=(String) monitoringTopicValue + "/Trouble";
                        troubletopic.toCharArray(textmessage, STRING_LEN);
                        if (dsc.trouble){ mqttClient.publish(textmessage, "1", true);}
                        else{ mqttClient.publish(textmessage, "0", true);}
      
      
                        /* --------------- SerialDebug: --------- */
                        Serial.println("MONITORING: online Status published");
                        /* --------------- SerialDebug: --------- */
                        /* --------------- mqttDebug: --------- */
                        if (atoi(enableMqttDebugValue) == 1) {
                          String msgtext= (String) deviceIdValue + " MONITORING: Trouble status published";
                          msgtext.toCharArray(textmessage, STRING_LEN);
                          mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                        /* --------------- mqttDebug: --------- */
                        }
                  // END Monitoring
                
                }
            
                // Publishes status per partition
                for (byte partition = 0; partition < dscPartitions; partition++) {
            
                  // Publishes exit delay status
                  if (dsc.exitDelayChanged[partition]) {
                    dsc.exitDelayChanged[partition] = false;  // Resets the exit delay status flag
            
                    // Appends the mqttPartitionTopic with the partition number
                    char publishTopic[strlen(mqttPartitionTopicValue) + 1];
                    char partitionNumber[2];
                    strcpy(publishTopic, mqttPartitionTopicValue);
                    itoa(partition + 1, partitionNumber, 10);
                    strcat(publishTopic, partitionNumber);
            
                    if (atoi(forceSecureAllTrafficValue) != 1){
                      if (dsc.exitDelay[partition]) mqttClient.publish(publishTopic, "pending", (bool) atoi(mqttRetainValue)); // Publish as a retained message
                      else if (!dsc.exitDelay[partition] && !dsc.armed[partition]) mqttClient.publish(publishTopic, "disarmed", (bool) atoi(mqttRetainValue));
                    }
                    if (atoi(forceSecureAllTrafficValue) == 1){
                      if (dsc.exitDelay[partition]) mqttClient.publish(publishTopic, "pending", (bool) atoi(mqttRetainValue)); // Publish as a retained message
                      else if (!dsc.exitDelay[partition] && !dsc.armed[partition]) mqttClient.publish(publishTopic, "disarmed", (bool) atoi(mqttRetainValue));
                   }

                   //Monitoring
                        if (atoi(enableMonitoringValue) >1) {
                        if (dsc.exitDelay[partition]){ 
                          String msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
                          msgtext1.toCharArray(topicpartition, STRING_LEN);
                          mqttClient.publish(topicpartition, "pending", true); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: PENDING Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {
                              String msgtext= (String) deviceIdValue + " MONITORING: PENDING status published for partition " + partitionNumber;
                              msgtext.toCharArray(textmessage, STRING_LEN);
                              mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                            /* --------------- mqttDebug: --------- */
                        }
                          else if (!dsc.exitDelay[partition] && !dsc.armed[partition]){ 
                            String msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
                          msgtext1.toCharArray(topicpartition, STRING_LEN);
                            mqttClient.publish(topicpartition, "disarmed", true);
                              /* --------------- SerialDebug: --------- */
                              Serial.println((String)"MONITORING: DISARMED Status published for partition " + partitionNumber);
                              /* --------------- SerialDebug: --------- */
                              /* --------------- mqttDebug: --------- */
                              if (atoi(enableMqttDebugValue) == 1) {
                                String msgtext= (String) deviceIdValue + " MONITORING: DISARMED status published for partition " + partitionNumber;
                                msgtext.toCharArray(textmessage, STRING_LEN);
                                mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                              /* --------------- mqttDebug: --------- */ 
                          }
                        }
                        if (atoi(enableMonitoringValue) == 1) {
                          String msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
                          msgtext1.toCharArray(topicpartition, STRING_LEN);
                          mqttClient.publish(topicpartition, "normal", true); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {
                              String msgtext=(String) deviceIdValue + " MONITORING: NORMAL status published for partition " + partitionNumber;
                              msgtext.toCharArray(textmessage, STRING_LEN);
                              mqttClient.publish(MqttDebugTopicValue,textmessage , (bool) atoi(remoteConfigRetainValue));}
                            /* --------------- mqttDebug: --------- */
                        }
                  // END Monitoring
                  }
            
                  // Publishes armed/disarmed status
                  if (dsc.armedChanged[partition]) {
                    dsc.armedChanged[partition] = false;  // Resets the partition armed status flag
            
                    // Appends the mqttPartitionTopic with the partition number
                    char publishTopic[strlen(mqttPartitionTopicValue) + 1];
                    char partitionNumber[2];
                    strcpy(publishTopic, mqttPartitionTopicValue);
                    itoa(partition + 1, partitionNumber, 10);
                    strcat(publishTopic, partitionNumber);
            
                    if (dsc.armed[partition]) {
            
                      if (atoi(forceSecureAllTrafficValue) != 1) {
                        if (dsc.armedAway[partition]) mqttClient.publish(publishTopic, "armed_away", true);
                        else if (dsc.armedStay[partition]) mqttClient.publish(publishTopic, "armed_home", true);
                      }
                      if (atoi(forceSecureAllTrafficValue) == 1) {
                        if (dsc.armedAway[partition]) mqttClient.publish(publishTopic, "armed_away", true);
                        else if (dsc.armedStay[partition]) mqttClient.publish(publishTopic, "armed_home", true);
                      }
                    
                    
                      //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
                        if (dsc.armedAway[partition]){ 
                          String msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
                          msgtext1.toCharArray(topicpartition, STRING_LEN);
                          mqttClient.publish(topicpartition, "armed_away", true); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: ARMED_AWAY Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {
                              String msgtext= (String) deviceIdValue + " MONITORING: ARMED_AWAY status published for partition " + partitionNumber;
                              msgtext.toCharArray(textmessage, STRING_LEN);
                              mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                            /* --------------- mqttDebug: --------- */
                        }
                          else if (dsc.armedStay[partition]){ 
                            String msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
                            msgtext1.toCharArray(topicpartition, STRING_LEN);
                            mqttClient.publish(topicpartition, "armed_home", true);
                              /* --------------- SerialDebug: --------- */
                              Serial.println((String)"MONITORING: ARMED_HOME Status published for partition " + partitionNumber);
                              /* --------------- SerialDebug: --------- */
                              /* --------------- mqttDebug: --------- */
                              if (atoi(enableMqttDebugValue) == 1) {
                                String msgtext= (String) deviceIdValue + " MONITORING: ARMED_HOME status published for partition " + partitionNumber;
                                msgtext.toCharArray(textmessage, STRING_LEN);
                                mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                              /* --------------- mqttDebug: --------- */ 
                          }
                        }

                        if (atoi(enableMonitoringValue) == 1) {
                          String msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
                            msgtext1.toCharArray(topicpartition, STRING_LEN);
                          mqttClient.publish(topicpartition, "normal", true); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {
                              String msgtext=(String) deviceIdValue + " MONITORING: NORMAL status published for partition " + partitionNumber;
                              msgtext.toCharArray(textmessage, STRING_LEN);
                              mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
                            /* --------------- mqttDebug: --------- */
                        }

                        
                    }
                    else {
                          if (atoi(forceSecureAllTrafficValue) != 1) {
                              mqttClient.publish(publishTopic, "disarmed", true);
                          }
                          if (atoi(forceSecureAllTrafficValue) == 1) {
                              mqttClient.publish(publishTopic, "disarmed", true);
                          }
                        //Monitoring
                        
                        
                        if (atoi(enableMonitoringValue) > 1) {
                          String msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
                            msgtext1.toCharArray(topicpartition, STRING_LEN);
                          mqttClient.publish(topicpartition, "disarmed", true); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: DISARMED Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {
                              String msgtext=(String) deviceIdValue + " MONITORING: DISARMED status published for partition " + partitionNumber;
                              msgtext.toCharArray(textmessage, STRING_LEN);  
                              mqttClient.publish(MqttDebugTopicValue,textmessage , (bool) atoi(remoteConfigRetainValue));
                            }
                            /* --------------- mqttDebug: --------- */
                        }

                        if (atoi(enableMonitoringValue) == 1) {
                          mqttClient.publish(textmessage, "normal", true); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {
                              String msgtext= (String) deviceIdValue + " MONITORING: NORMAL status published for partition " + partitionNumber;
                              msgtext.toCharArray(textmessage, STRING_LEN);
                              mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));
                              }
                            /* --------------- mqttDebug: --------- */
                        }
                    // END Monitoring
                    }
                  }
            
                  // Publishes alarm status
                  if (dsc.alarmChanged[partition]) {
                    dsc.alarmChanged[partition] = false;  // Resets the partition alarm status flag
                    if (dsc.alarm[partition]) {
            
                      // Appends the mqttPartitionTopic with the partition number
                      char publishTopic[strlen(mqttPartitionTopicValue) + 1];
                      char partitionNumber[2];
                      strcpy(publishTopic, mqttPartitionTopicValue);
                      itoa(partition + 1, partitionNumber, 10);
                      strcat(publishTopic, partitionNumber);
            
                      if (atoi(forceSecureAllTrafficValue) != 1){
                        mqttClient.publish(publishTopic, "triggered", true); // Alarm tripped
                      }
                      if (atoi(forceSecureAllTrafficValue) == 1){
                        mqttClient.publish(publishTopic, "triggered", true); // Alarm tripped
                      }


                      //Monitoring
                        if (atoi(enableMonitoringValue) > 0) {
                          String msgtext=(String)monitoringTopicValue + "/Partition" + partitionNumber;
                          msgtext.toCharArray(textmessage, STRING_LEN);
                          mqttClient.publish(textmessage, "triggered", true); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: TRIGGERED Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {
                              String msgtext=(String) deviceIdValue + " MONITORING: TRIGGERED status published for partition " + partitionNumber;
                              msgtext.toCharArray(textmessage, STRING_LEN);
                              mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));
                            }
                            /* --------------- mqttDebug: --------- */
                        }
                      // END Monitoring

                      
                    }
                  }
            
                  // Publishes fire alarm status
                  if (dsc.fireChanged[partition]) {
                    dsc.fireChanged[partition] = false;  // Resets the fire status flag
            
                    // Appends the mqttFireTopic with the partition number
                    char firePublishTopic[strlen(mqttFireTopicValue) + 1];
                    char partitionNumber[2];
                    strcpy(firePublishTopic, mqttFireTopicValue);
                    itoa(partition + 1, partitionNumber, 10);
                    strcat(firePublishTopic, partitionNumber);
            
            
                    if (atoi(forceSecureAllTrafficValue) != 1){
                        if (dsc.fire[partition]) mqttClient.publish(firePublishTopic, "1", (bool) atoi(mqttRetainValue));  // Fire alarm tripped
                        else mqttClient.publish(firePublishTopic, "0", (bool) atoi(mqttRetainValue));                      // Fire alarm restored
                    }
                    if (atoi(forceSecureAllTrafficValue) == 1){
                        if (dsc.fire[partition]) mqttClient.publish(firePublishTopic, "1", (bool) atoi(mqttRetainValue));  // Fire alarm tripped
                        else mqttClient.publish(firePublishTopic, "0", (bool) atoi(mqttRetainValue));                      // Fire alarm restored
                    }


                    //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
                        String msgtext=(String) monitoringTopicValue + "/Fire" + partitionNumber;
                        msgtext.toCharArray(textmessage, STRING_LEN);
                        if (dsc.fire[partition]){
                           mqttClient.publish(textmessage, "1", true);
                        }else{
                          mqttClient.publish(textmessage, "0", true);
                        }
                        /* --------------- SerialDebug: --------- */
                        Serial.println((String)"MONITORING: FIRE Status published for partition " + partitionNumber);
                        /* --------------- mqttDebug: --------- */
                        if (atoi(enableMqttDebugValue) == 1) {
                          String msgtext= (String) deviceIdValue + " MONITORING: FIRE status published for partition " + partitionNumber;
                          msgtext.toCharArray(textmessage, STRING_LEN);
                          mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));
                          }
                        /* --------------- mqttDebug: --------- */
                        }
                  // END Monitoring


                    
                  }
                }
            
                // Publishes zones 1-64 status in a separate topic per zone
                // Zone status is stored in the openZones[] and openZonesChanged[] arrays using 1 bit per zone, up to 64 zones:
                //   openZones[0] and openZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
                //   openZones[1] and openZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
                //   ...
                //   openZones[7] and openZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
                if (dsc.openZonesStatusChanged) {
                  dsc.openZonesStatusChanged = false;                           // Resets the open zones status flag
                  for (byte zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
                    for (byte zoneBit = 0; zoneBit < 8; zoneBit++) {
                      if (bitRead(dsc.openZonesChanged[zoneGroup], zoneBit)) {  // Checks an individual open zone status flag
                        bitWrite(dsc.openZonesChanged[zoneGroup], zoneBit, 0);  // Resets the individual open zone status flag
            
                        // Appends the mqttZoneTopic with the zone number
                        char zonePublishTopic[strlen(mqttZoneTopicValue) + 2];
                        char zone[3];
                        strcpy(zonePublishTopic, mqttZoneTopicValue);
                        itoa(zoneBit + 1 + (zoneGroup * 8), zone, 10);
                        strcat(zonePublishTopic, zone);
            
                        if (bitRead(dsc.openZones[zoneGroup], zoneBit)) {
                          if (atoi(forceSecureAllTrafficValue) != 1){
                            mqttClient.publish(zonePublishTopic, "1", (bool) atoi(mqttRetainValue));            // Zone open
                          }
                          if (atoi(forceSecureAllTrafficValue) == 1){
                            mqttClient.publish(zonePublishTopic, "1", (bool) atoi(mqttRetainValue));            // Zone open
                          }

                          //Monitoring
                                if (atoi(enableMonitoringValue) == 3) {
                                  String msgtext=(String) monitoringTopicValue + "/Zone" + zone;
                                  msgtext.toCharArray(textmessage, STRING_LEN);
                                  mqttClient.publish(textmessage, "1", true);
                                  /* --------------- SerialDebug: --------- */
                                  Serial.println((String)"MONITORING: ACTIVE status published for zone " + zone);
                                  /* --------------- SerialDebug: --------- */
                                  /* --------------- mqttDebug: --------- */
                                  if (atoi(enableMqttDebugValue) == 1) {
                                    String msgtext= (String) deviceIdValue + " MONITORING: ACTIVE status published for zone " + zone;
                                    msgtext.toCharArray(textmessage, STRING_LEN);
                                    mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));
                                    }
                                  /* --------------- mqttDebug: --------- */
                                }
                          // END Monitoring
                          
                        }
                        else {
                          if (atoi(forceSecureAllTrafficValue) != 1){
                            mqttClient.publish(zonePublishTopic, "0", (bool) atoi(mqttRetainValue));         // Zone closed
                          }
                          if (atoi(forceSecureAllTrafficValue) == 1){
                            mqttClient.publish(zonePublishTopic, "0", (bool) atoi(mqttRetainValue));         // Zone closed
                          }

                          //Monitoring
                                if (atoi(enableMonitoringValue) == 3) {
                                  String topicval=(String) monitoringTopicValue + "/Zone" + zone;
                                  topicval.toCharArray(textmessage, STRING_LEN);
                                  mqttClient.publish(textmessage, "0", true);
                                  /* --------------- SerialDebug: --------- */
                                  Serial.println((String)"MONITORING: INACTIVE status published for zone " + zone);
                                  /* --------------- SerialDebug: --------- */
                                  /* --------------- mqttDebug: --------- */
                                  if (atoi(enableMqttDebugValue) == 1) {
                                    String msgtext=(String) deviceIdValue + " MONITORING: INACTIVE status published for zone " + zone;
                                    msgtext.toCharArray(textmessage, STRING_LEN);
                                    mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));
                                    }
                                  /* --------------- mqttDebug: --------- */
                                }
                          // END Monitoring
                          
                        }
                      }
                    }
                  }
                }
            
            //    mqtt.subscribe(mqttCommandTopicValue);
              }


}


void controlDSC(String coMMand, int targetPartition){
            
            // Arm stay
            if (coMMand == "arm_stay" && !dsc.armed[targetPartition-1] && !dsc.exitDelay[targetPartition-1]) {
             // while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              dsc.writePartition = targetPartition;         // Sets writes to the partition number
              dsc.write('s');                             // Virtual keypad arm stay
            
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {
              String msgtext= (String) deviceIdValue + " ARM_STAY called";
              msgtext.toCharArray(textmessage, STRING_LEN);
              mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));
            }
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Arm away
            else if (coMMand == "arm_away" && !dsc.armed[targetPartition-1] && !dsc.exitDelay[targetPartition-1]) {
             // while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              dsc.writePartition = targetPartition;         // Sets writes to the partition number
              dsc.write('w');                             // Virtual keypad arm away

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {
              String msgtext= (String) deviceIdValue + " ARM_AWAY called";
              msgtext.toCharArray(textmessage, STRING_LEN);
              mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
            
            }
          
            // Disarm
            else if (coMMand == "disarm" && (dsc.armed[targetPartition-1] || dsc.exitDelay[targetPartition-1])) {
             // while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              dsc.writePartition = targetPartition;         // Sets writes to the partition number
              dsc.write(accessCodeValue);

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {
              String msgtext= (String) deviceIdValue + " Access Code sent to DISARM";
              msgtext.toCharArray(textmessage, STRING_LEN);
              mqttClient.publish(MqttDebugTopicValue,textmessage, (bool) atoi(remoteConfigRetainValue));}
            }
}
