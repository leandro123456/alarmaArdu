void doDSC(){
  
            
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
                      mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
                      lastSentStatus = String(mqttBirthMessageValue);
                    }
                    else {
                      mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
                      lastSentStatus = String(mqttNoDSCValue);
                    }
                  }
                  
                  if (atoi(forceSecureAllTrafficValue) == 1){
                    if (dsc.keybusConnected) {
                      mqttClientRConf.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
                      lastSentStatus = String(mqttBirthMessageValue);
                    }
                    else {
                      mqttClientRConf.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
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
                    if (dsc.trouble) mqttClient.publish(mqttTroubleTopicValue, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                    else mqttClient.publish(mqttTroubleTopicValue, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                  }
                  if (atoi(forceSecureAllTrafficValue) == 1){
                    if (dsc.trouble) mqttClientRConf.publish(mqttTroubleTopicValue, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                    else mqttClientRConf.publish(mqttTroubleTopicValue, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                  }
                  
                  //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
      
                        if (dsc.trouble) mqttClientRConf.publish((String) monitoringTopicValue + "/Trouble", "1", true, atoi(mqttQoSValue));
                          else mqttClientRConf.publish((String) monitoringTopicValue + "/Trouble", "0", true, atoi(mqttQoSValue));
      
      
                        /* --------------- SerialDebug: --------- */
                        Serial.println("MONITORING: online Status published");
                        /* --------------- SerialDebug: --------- */
                        /* --------------- mqttDebug: --------- */
                        if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: Trouble status published", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
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
                      if (dsc.exitDelay[partition]) mqttClient.publish(publishTopic, "pending", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); // Publish as a retained message
                      else if (!dsc.exitDelay[partition] && !dsc.armed[partition]) mqttClient.publish(publishTopic, "disarmed", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                    }
                    if (atoi(forceSecureAllTrafficValue) == 1){
                      if (dsc.exitDelay[partition]) mqttClientRConf.publish(publishTopic, "pending", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); // Publish as a retained message
                      else if (!dsc.exitDelay[partition] && !dsc.armed[partition]) mqttClientRConf.publish(publishTopic, "disarmed", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
                   }

                   //Monitoring
                        if (atoi(enableMonitoringValue) >1) {
                        if (dsc.exitDelay[partition]){ mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "pending", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: PENDING Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: PENDING status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }
                          else if (!dsc.exitDelay[partition] && !dsc.armed[partition]){ mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "disarmed", true, atoi(mqttQoSValue));
                              /* --------------- SerialDebug: --------- */
                              Serial.println((String)"MONITORING: DISARMED Status published for partition " + partitionNumber);
                              /* --------------- SerialDebug: --------- */
                              /* --------------- mqttDebug: --------- */
                              if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: DISARMED status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                              /* --------------- mqttDebug: --------- */ 
                          }
                        }
                        if (atoi(enableMonitoringValue) == 1) {mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "normal", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: NORMAL status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
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
                        if (dsc.armedAway[partition]) mqttClient.publish(publishTopic, "armed_away", true, atoi(mqttQoSValue));
                        else if (dsc.armedStay[partition]) mqttClient.publish(publishTopic, "armed_home", true, atoi(mqttQoSValue));
                      }
                      if (atoi(forceSecureAllTrafficValue) == 1) {
                        if (dsc.armedAway[partition]) mqttClientRConf.publish(publishTopic, "armed_away", true, atoi(mqttQoSValue));
                        else if (dsc.armedStay[partition]) mqttClientRConf.publish(publishTopic, "armed_home", true, atoi(mqttQoSValue));
                      }
                    
                    
                    //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
                        if (dsc.armedAway[partition]){ mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "armed_away", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: ARMED_AWAY Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: ARMED_AWAY status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }
                          else if (dsc.armedStay[partition]){ mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "armed_home", true, atoi(mqttQoSValue));
                              /* --------------- SerialDebug: --------- */
                              Serial.println((String)"MONITORING: ARMED_HOME Status published for partition " + partitionNumber);
                              /* --------------- SerialDebug: --------- */
                              /* --------------- mqttDebug: --------- */
                              if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: ARMED_HOME status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                              /* --------------- mqttDebug: --------- */ 
                          }
                        }

                        if (atoi(enableMonitoringValue) == 1) {mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "normal", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: NORMAL status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }

                        
                    }
                    else {
                                if (atoi(forceSecureAllTrafficValue) != 1) {
                                    mqttClient.publish(publishTopic, "disarmed", true, atoi(mqttQoSValue));
                                }
                                if (atoi(forceSecureAllTrafficValue) == 1) {
                                    mqttClientRConf.publish(publishTopic, "disarmed", true, atoi(mqttQoSValue));
                                }
                    
                    
                    
                    //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
                          mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "disarmed", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: DISARMED Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: DISARMED status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                            /* --------------- mqttDebug: --------- */
                        }

                        if (atoi(enableMonitoringValue) == 1) {mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "normal", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: NORMAL status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
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
                        mqttClient.publish(publishTopic, "triggered", true, atoi(mqttQoSValue)); // Alarm tripped
                      }
                      if (atoi(forceSecureAllTrafficValue) == 1){
                        mqttClientRConf.publish(publishTopic, "triggered", true, atoi(mqttQoSValue)); // Alarm tripped
                      }


                      //Monitoring
                        if (atoi(enableMonitoringValue) > 0) {
                          mqttClientRConf.publish((String)monitoringTopicValue + "/Partition" + partitionNumber, "triggered", true, atoi(mqttQoSValue)); // Publish as a retained message
                            /* --------------- SerialDebug: --------- */
                            Serial.println((String)"MONITORING: TRIGGERED Status published for partition " + partitionNumber);
                            /* --------------- SerialDebug: --------- */
                            /* --------------- mqttDebug: --------- */
                            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: TRIGGERED status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
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
                        if (dsc.fire[partition]) mqttClient.publish(firePublishTopic, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));  // Fire alarm tripped
                        else mqttClient.publish(firePublishTopic, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));                      // Fire alarm restored
                    }
                    if (atoi(forceSecureAllTrafficValue) == 1){
                        if (dsc.fire[partition]) mqttClientRConf.publish(firePublishTopic, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));  // Fire alarm tripped
                        else mqttClientRConf.publish(firePublishTopic, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));                      // Fire alarm restored
                    }


                    //Monitoring
                        if (atoi(enableMonitoringValue) > 1) {
      
                        if (dsc.fire[partition]) mqttClientRConf.publish((String) monitoringTopicValue + "/Fire" + partitionNumber, "1", true, atoi(mqttQoSValue));
                          else mqttClientRConf.publish((String) monitoringTopicValue + "/Fire" + partitionNumber, "0", true, atoi(mqttQoSValue));
      
      
                        /* --------------- SerialDebug: --------- */
                        Serial.println((String)"MONITORING: FIRE Status published for partition " + partitionNumber);
                        /* --------------- SerialDebug: --------- */
                        /* --------------- mqttDebug: --------- */
                        if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: FIRE status published for partition " + partitionNumber, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
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
                            mqttClient.publish(zonePublishTopic, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));            // Zone open
                          }
                          if (atoi(forceSecureAllTrafficValue) == 1){
                            mqttClientRConf.publish(zonePublishTopic, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));            // Zone open
                          }

                          //Monitoring
                                if (atoi(enableMonitoringValue) == 3) {
                                  mqttClientRConf.publish((String) monitoringTopicValue + "/Zone" + zone, "1", true, atoi(mqttQoSValue));
                                  /* --------------- SerialDebug: --------- */
                                  Serial.println((String)"MONITORING: ACTIVE status published for zone " + zone);
                                  /* --------------- SerialDebug: --------- */
                                  /* --------------- mqttDebug: --------- */
                                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: ACTIVE status published for zone " + zone, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                                  /* --------------- mqttDebug: --------- */
                                }
                          // END Monitoring
                          
                        }
                        else {
                          if (atoi(forceSecureAllTrafficValue) != 1){
                            mqttClient.publish(zonePublishTopic, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));         // Zone closed
                          }
                          if (atoi(forceSecureAllTrafficValue) == 1){
                            mqttClientRConf.publish(zonePublishTopic, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));         // Zone closed
                          }

                          //Monitoring
                                if (atoi(enableMonitoringValue) == 3) {
                                  mqttClientRConf.publish((String) monitoringTopicValue + "/Zone" + zone, "0", true, atoi(mqttQoSValue));
                                  /* --------------- SerialDebug: --------- */
                                  Serial.println((String)"MONITORING: INACTIVE status published for zone " + zone);
                                  /* --------------- SerialDebug: --------- */
                                  /* --------------- mqttDebug: --------- */
                                  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " MONITORING: INACTIVE status published for zone " + zone, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
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
              while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              dsc.writePartition = targetPartition;         // Sets writes to the partition number
              dsc.write('s');                             // Virtual keypad arm stay
            
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " ARM_STAY called", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Arm away
            else if (coMMand == "arm_away" && !dsc.armed[targetPartition-1] && !dsc.exitDelay[targetPartition-1]) {
              while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              dsc.writePartition = targetPartition;         // Sets writes to the partition number
              dsc.write('w');                             // Virtual keypad arm away

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " ARM_AWAY called", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            
            }
          
            // Disarm
            else if (coMMand == "disarm" && (dsc.armed[targetPartition-1] || dsc.exitDelay[targetPartition-1])) {
              while (!dsc.writeReady) dsc.handlePanel();  // Continues processing Keybus data until ready to write
              dsc.writePartition = targetPartition;         // Sets writes to the partition number
              dsc.write(accessCodeValue);

            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " Access Code sent to DISARM", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
            }
}
