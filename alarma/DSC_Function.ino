void doDSC(){ 
  if (dsc.statusChanged) { // Checks if the security system status has changed
    dsc.statusChanged = false;  // Reset the status tracking flag

    // If the Keybus data buffer is exceeded, the sketch is too busy to process all Keybus commands.  Call
    // loop() more often, or increase dscBufferSize in the library: src/dscKeybusInterface.h
    if (dsc.bufferOverflow){
      dsc.bufferOverflow = false;
    }

    // Checks if the interface is connected to the Keybus
    if (dsc.keybusChanged) {
      dsc.keybusChanged = false;  // Resets the Keybus data status flag
      if (dsc.keybusConnected) {
        mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true); //Alarm Online
        lastSentStatus = String(mqttBirthMessageValue);
      }
      else {
        mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true); //Alarm Disconected
        lastSentStatus = String(mqttNoDSCValue);
      }
    }
      
    // Sends the access code when needed by the panel for arming
    if (dsc.accessCodePrompt) {
      dsc.accessCodePrompt = false;
      dsc.write(accessCodeValue);
    }

    if (dsc.troubleChanged) {
      dsc.troubleChanged = false;  // Resets the trouble status flag
      if (dsc.trouble) 
        mqttClient.publish(mqttTroubleTopicValue, "1", (bool) atoi(mqttRetainValue));
      else 
        mqttClient.publish(mqttTroubleTopicValue, "0", (bool) atoi(mqttRetainValue));
      
      if (false) {
        String troubletopic=(String) monitoringTopicValue + "/Trouble";
        if (dsc.trouble)
          mqttClient.publish(troubletopic.c_str(), "1", true);
        else
          mqttClient.publish(troubletopic.c_str(), "0", true);
      }
    }
  
    // Publishes status per partition
    for (byte partition = 0; partition < dscPartitions; partition++) {
      // Skips processing if the partition is disabled or in installer programming
      if (dsc.disabled[partition]) continue;

      char partitionNumber[2];
      itoa(partition + 1, partitionNumber, 10);
      //strcat(publishTopic, partitionNumber);
      char publishTopic[strlen(mqttPartitionTopicValue) + 2];
      appendPartition(mqttPartitionTopicValue, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

      // Publishes the partition status message USING MNTR
      publishMessage(((String)monitoringTopicValue +"/DetailPartition").c_str(), partition);

      // Publishes armed/disarmed status
      if (dsc.armedChanged[partition]) {
        dsc.armedChanged[partition] = false;  // Resets the partition armed status flag
        if (dsc.armedChanged[partition]) dsc.armedChanged[partition] = false; 
        char publishTopic[strlen(mqttPartitionTopicValue) + 2]; // Appends the mqttPartitionTopic with the partition number
        appendPartition(mqttPartitionTopicValue, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

        String msgtext1;
        if (dsc.armed[partition]) {
          if (dsc.armedAway[partition] && dsc.noEntryDelay[partition]) 
            mqttClient.publish(publishTopic, "armed_night", true);
          else if (dsc.armedAway[partition]) 
            mqttClient.publish(publishTopic, "armed_away", true);
          else if (dsc.armedStay[partition] && dsc.noEntryDelay[partition]) 
            mqttClient.publish(publishTopic, "armed_night", true);
          else if (dsc.armedStay[partition]) 
            mqttClient.publish(publishTopic, "armed_home", true);
        
          //Monitoring
          if (false) {
            msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
            if (dsc.armedAway[partition]){
              sendMonitoring(msgtext1,"armed_away",String(deviceIdFinalValue) +" MONITORING: ARMED_AWAY Status published for partition " + partitionNumber);
            }
            else if (dsc.armedStay[partition]){
              sendMonitoring(msgtext1,"armed_home",String(deviceIdFinalValue) + " MONITORING: ARMED_HOME status published for partition " + partitionNumber);
            }
          }
        }
        else {
          mqttClient.publish(publishTopic, "disarmed", true);
          msgtext1=(String)monitoringTopicValue + "/Partition" + partitionNumber;
          sendMonitoring(msgtext1,"disarmed",(String)"MONITORING: NORMAL Status published for partition " + partitionNumber);
        }
      }

      // Publishes exit delay status
      if (dsc.exitDelayChanged[partition]) {
        dsc.exitDelayChanged[partition] = false;  // Resets the exit delay status flag
        char publishTopic[strlen(mqttPartitionTopicValue) + 2];
        appendPartition(mqttPartitionTopicValue, partition, publishTopic);

        if (dsc.exitDelay[partition]) mqttClient.publish(publishTopic, "pending", (bool) atoi(mqttRetainValue)); // Publish as a retained message
        else if (!dsc.exitDelay[partition] && !dsc.armed[partition]) mqttClient.publish(publishTopic, "disarmed", (bool) atoi(mqttRetainValue));

        //Monitoring
        String topicoDeMensaje=(String)monitoringTopicValue + "/Partition" + partitionNumber;
        if (false) {
          if (dsc.exitDelay[partition]){
            sendMonitoring(topicoDeMensaje,"pending",String(deviceIdFinalValue) + " MONITORING: PENDING status published for partition " + partitionNumber);
          }
          else if (!dsc.exitDelay[partition] && !dsc.armed[partition]){
            sendMonitoring(topicoDeMensaje,"disarmed",String(deviceIdFinalValue) + " MONITORING: DISARMED status published for partition " + partitionNumber);
          }
        }
      }

      // Publishes alarm status
      if (dsc.alarmChanged[partition]) {
        dsc.alarmChanged[partition] = false;  // Resets the partition alarm status flag
        char publishTopic[strlen(mqttPartitionTopicValue) + 2];
        appendPartition(mqttPartitionTopicValue, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number
        String topicSendMonit=(String)monitoringTopicValue + "/Partition" + partitionNumber;
        if (dsc.alarm[partition]) {
          mqttClient.publish(publishTopic, "triggered", true); // Alarm tripped
          sendMonitoring(topicSendMonit,"triggered",String(deviceIdFinalValue) + " MONITORING: TRIGGERED status published for partition " + partitionNumber);
        }
        else if (!dsc.armedChanged[partition]) {
          mqttClient.publish(publishTopic, "disarmed", true);
          sendMonitoring(topicSendMonit,"disarmed",String(deviceIdFinalValue) + " MONITORING: DISARMED status published for partition " + partitionNumber);
        }
      }

      // Publishes fire alarm status
      if (dsc.fireChanged[partition]) {
        dsc.fireChanged[partition] = false;  // Resets the fire status flag
        char firePublishTopic[strlen(mqttFireTopicValue) + 2];
        appendPartition(mqttFireTopicValue, partition, firePublishTopic);  // Appends the mqttFireTopic with the partition number
        
        String topicSendMonit=(String) monitoringTopicValue + "/Fire" + partitionNumber;
        if (dsc.fire[partition]){
          mqttClient.publish(firePublishTopic, "1", (bool) atoi(mqttRetainValue));  // Fire alarm tripped
          sendMonitoring(topicSendMonit,"1",String(deviceIdFinalValue) + " MONITORING: FIRE status published for partition " + partitionNumber);
        }else{ 
          mqttClient.publish(firePublishTopic, "0", (bool) atoi(mqttRetainValue));                      // Fire alarm restored
          sendMonitoring(topicSendMonit,"0",String(deviceIdFinalValue) + " MONITORING: FIRE status published for partition " + partitionNumber);
        }
      }
    }
    // Publishes zones 1-64 status in a separate topic per zone
    // Zone status is stored in the openZones[] and openZonesChanged[] arrays using 1 bit per zone, up to 64 zones:
    //   openZones[0] and openZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
    //   openZones[1] and openZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
    //   openZones[7] and openZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64 
    if (dsc.openZonesStatusChanged) {
      dsc.openZonesStatusChanged = false;

      // Resets the open zones status flag
      for (byte zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
        for (byte zoneBit = 0; zoneBit < 8; zoneBit++) {
          if (bitRead(dsc.openZonesChanged[zoneGroup], zoneBit)) {  // Checks an individual open zone status flag
            bitWrite(dsc.openZonesChanged[zoneGroup], zoneBit, 0);  // Resets the individual open zone status flag
            // Appends the mqttZoneTopic with the zone number
            char zonePublishTopic[strlen(mqttZoneTopicValue) + 3];
            char zone[3];
            strcpy(zonePublishTopic, mqttZoneTopicValue);
            itoa(zoneBit + 1 + (zoneGroup * 8), zone, 10);
            strcat(zonePublishTopic, zone);
            String topicval=(String) monitoringTopicValue + "/Zone" + zone;
            if (bitRead(dsc.openZones[zoneGroup], zoneBit)) {
                mqttClient.publish(zonePublishTopic, "1", (bool) atoi(mqttRetainValue));            // Zone open
              //Monitoring
              if (false) { //este era en maximo nivel de monitoreo
                sendMonitoring(topicval,"1",String(deviceIdFinalValue) + " MONITORING: ACTIVE status published for zone " + zone);
              }                
            }
            else {
              mqttClient.publish(zonePublishTopic, "0", (bool) atoi(mqttRetainValue));         // Zone closed
              //Monitoring
              if (false) { // este era en maximo nivel de monitoreo
                sendMonitoring(topicval,"0",String(deviceIdFinalValue) + " MONITORING: INACTIVE status published for zone " + zone);
              }
            }
          }
        }
      }
    }
  }
}

void sendMonitoring(String topicoDeMensaje, String mensaje, String mensajeDebug){
  Serial.println("Topic "+ topicoDeMensaje+ " Message: "+ mensaje + " messageDebug: "+ mensajeDebug);
}

void appendPartition(const char* sourceTopic, byte sourceNumber, char* publishTopic) {
  char partitionNumber[2];
  strcpy(publishTopic, sourceTopic);
  itoa(sourceNumber + 1, partitionNumber, 10);
  strcat(publishTopic, partitionNumber);
}
