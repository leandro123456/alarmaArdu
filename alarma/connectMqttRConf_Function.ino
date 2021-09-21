
boolean connectMqttRConf() {
  if (atoi(enableRConfigValue) == 1) {
  
        unsigned long aHora = millis();
        if ((lastRConfMqttConnectionAttempt + 1000) > aHora)
        {
          // Do not repeat within 1 sec.
          return false;
        }
        /* --------------- SerialDebug: --------- */
        Serial.println("Connecting to RConf MQTT server...");
        /* --------------- SerialDebug: --------- */                  

                 
        boolean result;
        mqttClientRConf.setWill(mqttStatusTopicValue, mqttLwtMessageValue, true, 0);
        result = mqttClientRConf.connect(mqttRCClientIDValue, remoteConfigMqttUserValue, remoteConfigMqttPwdValue);
        Serial.println((String)"mqttClientID-Rconf: " + mqttRCClientIDValue);
        Serial.println((String)"mqttUserNameValue-Rconf: " + remoteConfigMqttUserValue);
        Serial.println((String)"mqttUserPasswordValue-Rconf: " + remoteConfigMqttPwdValue);
        Serial.println("RESULTADO DE LA CONEXION-Rconf: " + (String)result);
        if (!result) {
          lastRConfMqttConnectionAttempt = aHora;
          return false;
        }

        /* --------------- SerialDebug: --------- */
        Serial.println("Connected RConf MQTT server");
        /* --------------- SerialDebug: --------- */

        if (atoi(forceSecureAllTrafficValue) != 1){
          if (dsc.keybusConnected) {
            mqttClient.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
            lastSentStatus = String(mqttBirthMessageValue);

            /* --------------- SerialDebug: --------- */
            Serial.println("Status message published: " + lastSentStatus);
            /* --------------- SerialDebug: --------- */
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
          }
          if (!dsc.keybusConnected) {
            mqttClient.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
            lastSentStatus = String(mqttNoDSCValue);

            /* --------------- SerialDebug: --------- */
            Serial.println("Status message published: " + lastSentStatus);
            /* --------------- SerialDebug: --------- */
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
          }
        }

        
        if (atoi(forceSecureAllTrafficValue) == 1 || atoi(enableRConfigValue) == 1){
          if (dsc.keybusConnected) {
            mqttClientRConf.publish(mqttStatusTopicValue, mqttBirthMessageValue, true, atoi(mqttQoSValue));
            lastSentStatus = String(mqttBirthMessageValue);

            /* --------------- SerialDebug: --------- */
            Serial.println("Status message published: " + lastSentStatus);
            /* --------------- SerialDebug: --------- */
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
          }
          if (!dsc.keybusConnected) {
            mqttClientRConf.publish(mqttStatusTopicValue, mqttNoDSCValue, true, atoi(mqttQoSValue));
            lastSentStatus = String(mqttNoDSCValue);

            /* --------------- SerialDebug: --------- */
            Serial.println("Status message published: " + lastSentStatus);
            /* --------------- SerialDebug: --------- */
            /* --------------- mqttDebug: --------- */
            if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Status message published: " + lastSentStatus, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
            /* --------------- mqttDebug: --------- */
          }
        }
        
        
      //Subcribe al topic de configuracion remota
        mqttClientRConf.subscribe(remoteConfigTopicValue);

        /* --------------- SerialDebug: --------- */
        Serial.println((String) "Subcribed to: " + remoteConfigTopicValue);
        /* --------------- SerialDebug: --------- */
        /* --------------- mqttDebug: --------- */
        if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Subcribed to: " + remoteConfigTopicValue), (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue);}
        /* --------------- mqttDebug: --------- */                      

      //Enva mensaje de HELLO a RConfig
      //String helloMsg = "{\"deviceId\":\"" + String(deviceIdValue) + "\", \"fwVer\":\"" + String(CONFIG_VERSION) + "\", \"RCTopic\":\"" + String(remoteConfigTopicValue) + "\", \"RCRTopic\":\"" + String(remoteConfigResultTopicValue) + "\", \"notice\":\"HELLO\"}";
      // La linea de arriba esta comentada porque el mensaje al tener los topicos era muy largo y pinchaba... La reemplac por la de abajo sin los topicos de RM.
      String helloMsg = "{\"deviceId\":\"" + String(deviceIdValue) + "\", \"fwVer\":\"" + String(CONFIG_VERSION) + "\", \"notice\":\"HELLO\"}";
       
      mqttClientRConf.publish(remoteConfigResultTopicValue, helloMsg, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Message sent. Topic: " + remoteConfigResultTopicValue + " Payload: " + helloMsg );
      /* --------------- SerialDebug: --------- */    
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Message sent: Topic: " + remoteConfigResultTopicValue + " Payload: " + helloMsg, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */


      //Subcribe al topic de commandos y de Status
      if(atoi(forceSecureAllTrafficValue) == 1){
            mqttClientRConf.subscribe(mqttCommandTopicValue);
              /* --------------- SerialDebug: --------- */
              Serial.println((String) "Subcribed to: " + mqttCommandTopicValue);
              /* --------------- SerialDebug: --------- */
              /* --------------- mqttDebug: --------- */
              if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Subcribed to: " + mqttCommandTopicValue, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
              /* --------------- mqttDebug: --------- */

      }  
       
       return true;
  }
}