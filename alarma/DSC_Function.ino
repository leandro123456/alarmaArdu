void doDSC(){           
  if (dsc.statusChanged) {  // Processes data only when a valid Keybus command has been read
    dsc.statusChanged = false;                   // Reset the status tracking flag

    // If the Keybus data buffer is exceeded, the sketch is too busy to process all Keybus commands.  Call
    // handlePanel() more often, or increase dscBufferSize in the library: src/dscKeybusInterface.h
    if (dsc.bufferOverflow){
      Serial.println("Keybus buffer overflow");
      dsc.bufferOverflow = false;
    }

    // Checks if the interface is connected to the Keybus
    if (dsc.keybusChanged) {
      dsc.keybusChanged = false;  // Resets the Keybus data status flag
      if (dsc.keybusConnected) {
        mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
        lastSentStatus = String(mqttBirthMessageValue);
      }
      else {
        mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
        lastSentStatus = String(mqttNoDSCValue);
      }
    }
      
    // Sends the access code when needed by the panel for arming
    if (dsc.accessCodePrompt) { //&& dsc.writeReady) {
      dsc.accessCodePrompt = false;
      dsc.write(accessCodeValue);
    }
  
    if (dsc.troubleChanged) {
      dsc.troubleChanged = false;  // Resets the trouble status flag
      if (dsc.trouble){ 
        mqttClient.publish(mqttTroubleTopicValue, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
        //Monitoring
        if (atoi(enableMonitoringValue) > 1)
          publishMonitoring((String) monitoringTopicValue + "/Trouble","1",String(deviceIdFinalValue) + " MONITORING: Trouble status published"); 
      }
      else{ 
        mqttClient.publish(mqttTroubleTopicValue, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
        //Monitoring
        if (atoi(enableMonitoringValue) > 1)
          publishMonitoring((String) monitoringTopicValue + "/Trouble","0",String(deviceIdFinalValue) + " MONITORING: Trouble status published");
      }
    }
  
    // Publishes status per partition
    for (byte partition = 0; partition < dscPartitions; partition++) {
      // Skips processing if the partition is disabled or in installer programming [nuevo]
      if (dsc.disabled[partition]) continue;

      char partitionNumber[2];
      itoa(partition + 1, partitionNumber, 10);
      char publishTopic[strlen(mqttPartitionTopicValue) + 2];
      appendPartition(mqttPartitionTopicValue, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

      // Publishes the partition status message [nuevo]
      //if (atoi(enableMonitoringValue) != 0)
      publishMessage((String)monitoringTopicValue +"/DetailPartition" + partitionNumber, partition);

      
      // Publishes armed/disarmed status
      if (dsc.armedChanged[partition]) {
        dsc.armedChanged[partition] = false;  // Resets the partition armed status flag
        
        if (dsc.armed[partition]) {
          if (dsc.armedAway[partition] && dsc.noEntryDelay[partition]) 
            mqttClient.publish(publishTopic, "armed_night", true, atoi(mqttQoSValue));
          else if (dsc.armedAway[partition]) 
            mqttClient.publish(publishTopic, "armed_away", true, atoi(mqttQoSValue));
          else if (dsc.armedStay[partition] && dsc.noEntryDelay[partition]) 
            mqttClient.publish(publishTopic, "armed_night", true, atoi(mqttQoSValue));
          else if (dsc.armedStay[partition]) 
            mqttClient.publish(publishTopic, "armed_home", true, atoi(mqttQoSValue));
          //Monitoring
          if (atoi(enableMonitoringValue) > 1) {
            if (dsc.armedAway[partition])
              publishMonitoring((String)monitoringTopicValue + "/Partition" + partitionNumber,"armed_away",String(deviceIdFinalValue) + " MONITORING: ARMED_AWAY status published for partition " + partitionNumber);
            else if (dsc.armedStay[partition])
              publishMonitoring((String)monitoringTopicValue + "/Partition" + partitionNumber,"armed_home",String(deviceIdFinalValue) + " MONITORING: ARMED_HOME status published for partition " + partitionNumber);
          }
          if (atoi(enableMonitoringValue) == 1)
            publishMonitoring((String)monitoringTopicValue + "/Partition" + partitionNumber,"normal", String(deviceIdFinalValue) + " MONITORING: NORMAL status published for partition " + partitionNumber); 
        }
        else {
          mqttClient.publish(publishTopic, "disarmed", true, atoi(mqttQoSValue));
          //Monitoring
          if (atoi(enableMonitoringValue) > 1)
            publishMonitoring((String)monitoringTopicValue + "/Partition" + partitionNumber,"disarmed",String(deviceIdFinalValue) + " MONITORING: DISARMED status published for partition " + partitionNumber);
          if (atoi(enableMonitoringValue) == 1) 
            publishMonitoring((String)monitoringTopicValue + "/Partition" + partitionNumber,"normal",String(deviceIdFinalValue) + " MONITORING: NORMAL status published for partition " + partitionNumber);
        }
      }

      // Publishes exit delay status
      if (dsc.exitDelayChanged[partition]) {
        dsc.exitDelayChanged[partition] = false;  // Resets the exit delay status flag
        char publishTopic[strlen(mqttPartitionTopicValue) + 2];
        appendPartition(mqttPartitionTopicValue, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

        if (dsc.exitDelay[partition]){
          mqttClient.publish(publishTopic, "pending", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); // Publish as a retained message
          //Monitoring
          if (atoi(enableMonitoringValue) >1) 
            publishMonitoring((String)monitoringTopicValue + "/Partition" + partitionNumber,"pending",String(deviceIdFinalValue) + " MONITORING: PENDING status published for partition " + partitionNumber);
        }
        else if (!dsc.exitDelay[partition] && !dsc.armed[partition]) {
          mqttClient.publish(publishTopic, "disarmed", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
          //Monitoring
          if (atoi(enableMonitoringValue) >1) 
            publishMonitoring((String)monitoringTopicValue + "/Partition" + partitionNumber,"disarmed",String(deviceIdFinalValue) + " MONITORING: DISARMED status published for partition " + partitionNumber);
        }
        //Monitoring
        if (atoi(enableMonitoringValue) == 1) 
          publishMonitoring((String)monitoringTopicValue + "/Partition" + partitionNumber,"normal",String(deviceIdFinalValue) + " MONITORING: NORMAL status published for partition " + partitionNumber);
      }

      // Publishes alarm status
      if (dsc.alarmChanged[partition]) {
        dsc.alarmChanged[partition] = false;  // Resets the partition alarm status flag
        char publishTopic[strlen(mqttPartitionTopicValue) + 2];
        appendPartition(mqttPartitionTopicValue, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

        if (dsc.alarm[partition]) {
          mqttClient.publish(publishTopic, "triggered", true, atoi(mqttQoSValue)); // Alarm tripped
          //Monitoring
          if (atoi(enableMonitoringValue) > 0) 
            publishMonitoring((String)monitoringTopicValue + "/Partition" + partitionNumber,"triggered",String(deviceIdFinalValue) + " MONITORING: TRIGGERED status published for partition " + partitionNumber);
        }
        else if (!dsc.armedChanged[partition]){ //[nuevo]
          mqttClient.publish(publishTopic, "disarmed", true,atoi(mqttQoSValue));
          //Monitoring
          if (atoi(enableMonitoringValue) > 0) 
            publishMonitoring((String)monitoringTopicValue + "/Partition" + partitionNumber,"disarmed",String(deviceIdFinalValue) + " MONITORING: DISARMED status published for partition " + partitionNumber);
        }
      }

      // Publishes fire alarm status
      if (dsc.fireChanged[partition]) {
        dsc.fireChanged[partition] = false;  // Resets the fire status flag
        char publishTopic[strlen(mqttFireTopicValue) + 2];
        appendPartition(mqttFireTopicValue, partition, publishTopic);  // Appends the mqttFireTopic with the partition number

        if (dsc.fire[partition]){
          mqttClient.publish(publishTopic, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));  
          //Monitoring
          if (atoi(enableMonitoringValue) > 1)
            publishMonitoring((String) monitoringTopicValue + "/Fire" + partitionNumber, "1",String(deviceIdFinalValue) + " MONITORING: FIRE status published for partition " + partitionNumber);
        }else{
          mqttClient.publish(publishTopic, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));                      
          //Monitoring
          if (atoi(enableMonitoringValue) > 1)
            publishMonitoring((String) monitoringTopicValue + "/Fire" + partitionNumber, "0",String(deviceIdFinalValue) + " MONITORING: FIRE status published for partition " + partitionNumber);
        }
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
              mqttClient.publish(zonePublishTopic, "1", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue)); // Zone open
              //Monitoring
              if (atoi(enableMonitoringValue) == 3) {
                publishMonitoring((String) monitoringTopicValue + "/Zone" + zone,"1",String(deviceIdFinalValue) + " MONITORING: ACTIVE status published for zone " + zone); 
              }
            }
            else {
              mqttClient.publish(zonePublishTopic, "0", (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));         // Zone closed
              //Monitoring
              if (atoi(enableMonitoringValue) == 3) {
                publishMonitoring((String) monitoringTopicValue + "/Zone" + zone,"0",String(deviceIdFinalValue) + " MONITORING: INACTIVE status published for zone " + zone);
              }
            }
          }
        }
      }
    }
  }
}


void publishMonitoring(String topicoDeMonitoreo, String messaje, String messageToDebug){
  mqttClient.publish(topicoDeMonitoreo, messaje, true, atoi(mqttQoSValue));
  /* --------------- SerialDebug: --------- */
  Serial.println(messageToDebug);
  /* --------------- mqttDebug: --------- */
  if (atoi(enableMqttDebugValue) == 1) {
    mqttClient.publish(MqttDebugTopicValue, messageToDebug, (bool) atoi(mqttRetainValue), atoi(mqttQoSValue));
  }
}

void appendPartition(const char* sourceTopic, byte sourceNumber, char* publishTopic) {
  char partitionNumber[2];
  strcpy(publishTopic, sourceTopic);
  itoa(sourceNumber + 1, partitionNumber, 10);
  strcat(publishTopic, partitionNumber);
}