void mqttRConfMessageReceived(String &topic, String &payload)
{

/* --------------- SerialDebug: --------- */
Serial.println("================================ Message received REMOTE MANAGEMENT: " + topic + " - " + payload);
/* --------------- SerialDebug: --------- */
/* --------------- mqttDebug: --------- */
if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Message received: Topic: " + topic + " Payload: " + payload, false, atoi(remoteConfigQoSValue));}
/* --------------- mqttDebug: --------- */


if (atoi(forceSecureAllTrafficValue) == 1 && topic != remoteConfigTopicValue) {
        /* --------------- SerialDebug: --------- */
        Serial.println("Non Remote Management MQTT message received on Remote Management connection. Forwarding...");
        /* --------------- SerialDebug: --------- */
        /* --------------- mqttDebug: --------- */
        if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + "Non Remote Management MQTT message received on Remote Management connection. Forwarding...", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
        /* --------------- mqttDebug: --------- */
  String topIco = topic;
  String payLoad = payload;
  mqttMessageReceived(topIco, payLoad);
  return;
}

 
// Interpreta comandos de configuracin remota
 if (topic == remoteConfigTopicValue){

    //Configuracin remota
    /* --------------- SerialDebug: --------- */
    Serial.println("Mensaje de configiracion remota recibido: " + payload);
    /* --------------- SerialDebug: --------- */
    /* --------------- mqttDebug: --------- */
    if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote config message received: " + payload, false, atoi(remoteConfigQoSValue));}
    /* --------------- mqttDebug: --------- */

    String RConfCommandResult;
    int commandResponses = 0;
    
    StaticJsonDocument<500> jsonBuffer;  // estaba en 300 pero lo sub a 500 porque no entraban los comments

    deserializeJson(jsonBuffer, payload);
        String RCpaSSwd = jsonBuffer["pwd"];
        String coMMand = jsonBuffer["command"];
        String paRam1 = jsonBuffer["param1"];
        String paRam2 = jsonBuffer["param2"];
    
/* --------------- mqttDebug: --------- */
if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - JSON payload pharsed: " + payload + " Command: " + coMMand, false, atoi(remoteConfigQoSValue));}
/* --------------- mqttDebug: --------- */    
    //JsonObject& rootOut = jsonBuffer.createObject();
    
    /*
     * Comandos para administracin remota.
     * El comando getAllParams deberia devolver un json con todos los parametros de configuracion. (pero no pude armar un solo json porque por el largo no entra en un mensahe mqtt)
     * 
     * 
     * Faltan comandos para actualizar los comando basicos: SSID del AP, Password del AP, WifiSsid, WifiPassword y el timeout del AP
     * 
     * 
     * 
     * 
     */


// Verifica Pwd para la administracion remota
 if (RCpaSSwd != String(RConfigPwdValue)) {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "FAIL: Wrong Password";
      jsonBuffer["comments"] = "Wrong Password";
      // jsonBuffer["comments"] = RConfigPwdValue;
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
  
  return;  // con esto debera salir sin hacer nada cuando el pwd no coincide
 }                       
    /*    
     *   El comando getConfigVersion devuelve el deviceID y la version del firmware del dispositivo
     */
    if (coMMand == "getConfigVersion") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(CONFIG_VERSION);

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Remote Config result message sent. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */                    
    }


  /*    
     *   El comando getIP devuelve el deviceID y la ip privada asignda en la red local, la mascara de la subnet y el default gateway
     */
    if (coMMand == "getIP") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["localIP"] = WiFi.localIP().toString();
      jsonBuffer["subNetMask"] = WiFi.subnetMask().toString();
      jsonBuffer["gateWay"] = WiFi.gatewayIP().toString();

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Remote Config result message sent. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }

     /*    
     *   El comando getEnableMqttDebug devuelve el deviceID y estado del debug por mqtt
     */
   
    if (coMMand == "getEnableMqttDebug") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(enableMqttDebugValue);
      /*
      JsonArray& data = rootOut.createNestedArray("data");
      data.add(48.756080);
      data.add(2.302038);
      */

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }
    
    /*    
     *   El comando updateEnableMqttDebug modifica el estado del debug por mqtt
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateEnableMqttDebug") {

      memset(enableMqttDebugValue, 0, sizeof enableMqttDebugValue);
      strncpy(enableMqttDebugValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Enable MQTT debug state is now: " + String(enableMqttDebugValue);

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
                      
    }


     /*    
     *   El comando getMqttDebugTopic devuelve el deviceID y topic donde se publican los mensajes de debug
     */
   
    if (coMMand == "getMqttDebugTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(MqttDebugTopicValue);

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }
    
    /*    
     *   El comando updateMqttDebugTopic modifica el topic donde se publican los mensajes de debug
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttDebugTopic") {

      memset(MqttDebugTopicValue, 0, sizeof MqttDebugTopicValue);
      strncpy(MqttDebugTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT debug topic is now: " + String(MqttDebugTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      
      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


      

    /*    
     *   El comando triggerSendStatus devuelve el deviceID y hace que se publiquen los estados en el topic de stado del/los switches
     */
     
    if (coMMand == "triggerSendStatus") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";

      publicaEstados();
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }



/*    
     *   El comando getMqttConnStatus Devuelve el deviceID y el estado de la conexion al servidor MQTT principal
     */
     
    if (coMMand == "getMqttConnStatus") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      

      if(mqttClient.connected()){
        jsonBuffer["response"] = "1";
      }
      else {
        jsonBuffer["response"] = "0";
      }
      
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
    }



    /*    
     *   El comando getMqttServer devuelve el deviceID y la url del servidor MQTT configurado
     */
   
    if (coMMand == "getMqttServer") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
    }


 
    /*    
     *   El comando updateMqttServer actualiza la url del servidor MQTT por la proporcionada en campo parametro.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttServer") {

      //char paRam1C[sizeof(paRam1)];
      //paRam1.toCharArray(paRam1C, sizeof(paRam1));
      //strncpy(mqttServerValue, paRam1C, sizeof(paRam1C));
      memset(mqttServerValue, 0, sizeof mqttServerValue);
      strncpy(mqttServerValue, paRam1.c_str(), String(paRam1).length());  

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT server is now: " + String(mqttServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
                      
    }


/*    
     *   El comando getUpdateInterval devuelve el deviceID y el intervalo el segundos entre publicacion de info
     */
     
    if (coMMand == "getUpdateInterval") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(updateIntervalValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }

     if (coMMand == "updateUpdateInterval") {

      memset(updateIntervalValue, 0, sizeof updateIntervalValue);
      strncpy(updateIntervalValue, paRam1.c_str(), String(paRam1).length());


      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Update Interval is now: " + String(updateIntervalValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
                      
    }


/*    
     *   El comando getMqttPort devuelve el deviceID y el puertopara la conexion al servidor MQTT de la funcionalidad principal
     */
   
    if (coMMand == "getMqttPort") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttPortValue);

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttPort actualiza el puerto para la conexion al servidor MQTT de la funcionalidad principal por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttPort") {

      memset(mqttPortValue, 0, sizeof mqttPortValue);
      strncpy(mqttPortValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT Port is now: " + String(mqttPortValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
                
    }

  /*    
     *   El comando getMqttClientID devuelve el deviceID y el clientID con el que esta confgigurado el cliente MQTT tanto para la funcionalidad principal como para RConfig
     */
   
    if (coMMand == "getMqttClientID") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttClientIDValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttClientID actualiza el el clienteID del cliente MQTT tabto de la funcionalidad principal como de RCOnfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttClientID") {

      memset(mqttClientIDValue, 0, sizeof mqttClientIDValue);
      strncpy(mqttClientIDValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT ClientID is now: " + String(mqttClientIDValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
                      
    }


   /*    
     *   El comando getMqttUserName devuelve el nombre de usuario utilizado para conectarse al servidor MQTT de la funcionalidad principal
     */
   
    if (coMMand == "getMqttUserName") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttUserNameValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttUserName actualiza el nombre de usuario con el que se conecta al server MQTT de la funcionalidad principal por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttUserName") {

      memset(mqttUserNameValue, 0, sizeof mqttUserNameValue);
      strncpy(mqttUserNameValue, paRam1.c_str(), String(paRam1).length());


      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT User Name is now: " + String(mqttUserNameValue);
      

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


   /*    
     *   El comando getMqttUserPassword devuelve la contrasea utilizada para conectarse al servidor MQTT de la funcionalidad principal
     */
   
    if (coMMand == "getMqttUserPassword") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttUserPasswordValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttUserPassword actualiza la contrasea con la que se conecta al server MQTT de la funcionalidad principal por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttUserPassword") {

      memset(mqttUserPasswordValue, 0, sizeof mqttUserPasswordValue);
      strncpy(mqttUserPasswordValue, paRam1.c_str(), String(paRam1).length());


      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT User Password updated";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


    /*    
     *   El comando updateDeviceID actualiza el deviceID del dispositivo por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateDeviceID") {

      memset(deviceIdValue, 0, sizeof deviceIdValue);
      strncpy(deviceIdValue, paRam1.c_str(), String(paRam1).length());


      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Device ID is now: " + String(deviceIdValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }







    /*    
     *   El comando disableRConfig deshabilita el Remote Management. Ojo porque luego solo se puede habilitar con acceso fsico al dispositivo.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "disableRConfig") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;

      if (paRam1 == "DISABLE" && paRam2 == "YES"){
        memset(enableRConfigValue, 0, sizeof enableRConfigValue);
        strncpy(enableRConfigValue, String("0").c_str(), String("0").length());
        jsonBuffer["response"] = "OK";
        jsonBuffer["comments"] = "Remote Management feature has beed disabled.";
      }
      else {
        jsonBuffer["response"] = "FAIL";
        jsonBuffer["comments"] = "Forbidden - Wrong parameters";
     }
  
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }



   /*    
     *   El comando getMqttRetain devuelve el valor para el parametro RETAIN de MQTT para la publicacin de mensajes de de la funcionalidad principal
     */
   
    if (coMMand == "getMqttRetain") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttRetainValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttRetain actualiza el valor para el parametro RETAIN de MQTT para la publicacin de mensajes de de la funcionalidad principal por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttRetain") {

      memset(mqttRetainValue, 0, sizeof mqttRetainValue);
      strncpy(mqttRetainValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT Retain parameter is now: " + String(mqttRetainValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


   /*    
     *   El comando getMqttQoS devuelve el valor para el parametro QoS de MQTT para la publicacin de mensajes de de la funcionalidad principal
     */
   
    if (coMMand == "getMqttQoS") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttQoSValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttQoS actualiza el valor para el parametro QoS de MQTT para la publicacin de mensajes de de la funcionalidad principal por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttQoS") {

      memset(mqttQoSValue, 0, sizeof mqttQoSValue);
      strncpy(mqttQoSValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT QoS parameter is now: " + String(mqttQoSValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

   /*    
     *   El comando getRemoteConfigMqttServer devuelve el servidor al que se conectara para RConfig 
     */
   
    if (coMMand == "getRemoteConfigMqttServer") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigMqttServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigMqttServer actualiza el servidor al que se conecta para RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigMqttServer") {

      memset(remoteConfigMqttServerValue, 0, sizeof remoteConfigMqttServerValue);
      strncpy(remoteConfigMqttServerValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config MQTT server is now: " + String(remoteConfigMqttServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

   /*    
     *   El comando getRemoteConfigMqttPort devuelve el puerto de conexion para RConfig 
     */
   
    if (coMMand == "getRemoteConfigMqttPort") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigMqttPortValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigMqttPort actualiza el puerto de conexion para RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigMqttPort") {

      memset(remoteConfigMqttPortValue, 0, sizeof remoteConfigMqttPortValue);
      strncpy(remoteConfigMqttPortValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config MQTT Port is now: " + String(remoteConfigMqttPortValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

   /*    
     *   El comando getRemoteConfigMqttUser devuelve el nombre de usuario para la conexion para RConfig 
     */
   
    if (coMMand == "getRemoteConfigMqttUser") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigMqttUserValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigMqttUser actualiza el nombre de usuario para la conexion para RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigMqttUser") {

      memset(remoteConfigMqttUserValue, 0, sizeof remoteConfigMqttUserValue);
      strncpy(remoteConfigMqttUserValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config MQTT Username is now: " + String(remoteConfigMqttUserValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


   /*    
     *   El comando getRemoteConfigMqttPwd devuelve la contrasea para la conexion para RConfig 
     */
   
    if (coMMand == "getRemoteConfigMqttPwd") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigMqttPwdValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigMqttPwd actualiza la contrasea para la conexion para RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigMqttPwd") {

      memset(remoteConfigMqttPwdValue, 0, sizeof remoteConfigMqttPwdValue);
      strncpy(remoteConfigMqttPwdValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config MQTT Password updated";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


   /*    
     *   El comando getForceSecureAllTraffic devuelve el estado del flag forceSecureAllTraffic 
     */
   
    if (coMMand == "getForceSecureAllTraffic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(forceSecureAllTrafficValue);

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateForceSecureAllTraffic actualiza el flag ForceSecureAllTraffic por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateForceSecureAllTraffic") {

      memset(forceSecureAllTrafficValue, 0, sizeof forceSecureAllTrafficValue);
      strncpy(forceSecureAllTrafficValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Force Secure All Traffic flag is now: " + String(forceSecureAllTrafficValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }




    /*    
     *   El comando updateRemoteConfigPwd actualiza la contrasea para enviar comandos de RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigPwd") {

      memset(RConfigPwdValue, 0, sizeof RConfigPwdValue);
      strncpy(RConfigPwdValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config Password updated";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


   /*    
     *   El comando getRemoteConfigTopic devuelve el topic en el que el dispositivo espera los comandos de RConfig 
     */
   
    if (coMMand == "getRemoteConfigTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigTopic actualiza el topico en el que el dispositivo espera comandos de RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigTopic") {

      memset(remoteConfigTopicValue, 0, sizeof remoteConfigTopicValue);
      strncpy(remoteConfigTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config Topic is now: " + String(remoteConfigTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

   /*    
     *   El comando getRemoteConfigResultTopic devuelve el topic en el que el dispositivo publica los resultados de los comandos de RConfig 
     */
   
    if (coMMand == "getRemoteConfigResultTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigResultTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigResultTopic actualiza el topico en el que el dispositivo publica los resultados comandos de RConfig por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigResultTopic") {

      memset(remoteConfigResultTopicValue, 0, sizeof remoteConfigResultTopicValue);
      strncpy(remoteConfigResultTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config Result Topic is now: " + String(remoteConfigResultTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


      /*    
     *   El comando getRemoteConfigRetain devuelve el estado del parametro retain para los mensajes MQTT publicados. (0 = false, 1 = true)  
     */
   
    if (coMMand == "getRemoteConfigRetain") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigRetainValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigRetain actualiza el parametro retain por el proporcionado en campo parametro pram1. (0 = false, 1 = true)
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigRetain") {

      memset(remoteConfigRetainValue, 0, sizeof remoteConfigRetainValue);
      strncpy(remoteConfigRetainValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config Retain value in now: " + String(remoteConfigRetainValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

/*    
     *   El comando getRemoteConfigQoS devuelve el estado del parametro QoS para los mensajes MQTT de Remote Management publicados. (0, 1 o 2)  
     */
   
    if (coMMand == "getRemoteConfigQoS") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(remoteConfigQoSValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateRemoteConfigQoS actualiza el parametro QoS por el proporcionado en campo parametro pram1. (0, 1 o 2)
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateRemoteConfigQoS") {

      memset(remoteConfigQoSValue, 0, sizeof remoteConfigQoSValue);
      strncpy(remoteConfigQoSValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Remote Config QoS parameter is now: " + String(remoteConfigQoSValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


/*    
     *   El comando updateAccessCode actualiza el codigo para armar y desarmar la alarma por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateAccessCode") {

      memset(accessCodeValue, 0, sizeof accessCodeValue);
      strncpy(accessCodeValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Access Code updated";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getMqttStatusTopic devuelve el topic en el que el dispositivo publicar el Status
     */
   
    if (coMMand == "getMqttStatusTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttStatusTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttStatusTopic actualiza el topico en el que el dispositivo publica el Status por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttStatusTopic") {

      memset(mqttStatusTopicValue, 0, sizeof mqttStatusTopicValue);
      strncpy(mqttStatusTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "MQTT Status Topic is now: " + String(mqttStatusTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getMqttBirthMessage devuelve el payload del mensaje de Status cuando establece conexin
     */
   
    if (coMMand == "getMqttBirthMessage") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttBirthMessageValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttBirthMessage actualiza el payload que el dispositivo publica en el topcico de Status cuando se conecta por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttBirthMessage") {

      memset(mqttBirthMessageValue, 0, sizeof mqttBirthMessageValue);
      strncpy(mqttBirthMessageValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt Birth Message is now: " + String(mqttBirthMessageValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getMqttLwtMessage devuelve el payload del mensaje de Last Will para cuando se pierde la conexin
     */
   
    if (coMMand == "getMqttLwtMessage") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttLwtMessageValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttLwtMessage actualiza el payload que el broker publica en el topcico de Status cuando se pierde la conexin por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttLwtMessage") {

      memset(mqttLwtMessageValue, 0, sizeof mqttLwtMessageValue);
      strncpy(mqttLwtMessageValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt LWT message is now: " + String(mqttLwtMessageValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getMqttNoDSC devuelve el payload del mensaje que el dipositivo publica cuando se pierde la conexin con el sistema de alarma
     */
   
    if (coMMand == "getMqttNoDSC") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttNoDSCValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttNoDSC actualiza el payload que el dispositivo publica en el topcico de Status cuando se pierde la conexin con el sistema de alarma por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttNoDSC") {

      memset(mqttNoDSCValue, 0, sizeof mqttNoDSCValue);
      strncpy(mqttNoDSCValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt NoDSC message is now: " + String(mqttNoDSCValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getMqttPartitionTopic devuelve el prefijo con el que el dispositivo crea los topicos de estado de las particiones
     */
   
    if (coMMand == "getMqttPartitionTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttPartitionTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttPartitionTopic actualiza el prefijo con el que el dispositivo crea los topicos de estado de las particiones por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttPartitionTopic") {

      memset(mqttPartitionTopicValue, 0, sizeof mqttPartitionTopicValue);
      strncpy(mqttPartitionTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt partition topic prefix is now: " + String(mqttPartitionTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }



/*    
     *   El comando getMqttPartitionTopic devuelve el prefijo con el que el dispositivo crea los topicos de estado de las particiones
     */
   
    if (coMMand == "getMqttActivePartitionTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttActivePartitionTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttPartitionTopic actualiza el prefijo con el que el dispositivo crea los topicos de estado de las particiones por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttActivePartitionTopic") {

      memset(mqttActivePartitionTopicValue, 0, sizeof mqttActivePartitionTopicValue);
      strncpy(mqttActivePartitionTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt Active Partition topic is now: " + String(mqttPartitionTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }



  /*    
     *   El comando getMqttZoneTopic devuelve el prefijo con el que el dispositivo crea los topicos de estado de las zonas
     */
   
    if (coMMand == "getMqttZoneTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttZoneTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttZoneTopic actualiza el prefijo con el que el dispositivo crea los topicos de estado de las zonas por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttZoneTopic") {

      memset(mqttZoneTopicValue, 0, sizeof mqttZoneTopicValue);
      strncpy(mqttZoneTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt zone topic prefix is now: " + String(mqttZoneTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getMqttFireTopic devuelve el prefijo con el que el dispositivo crea los topicos de Fire
     */
   
    if (coMMand == "getMqttFireTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttFireTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttFireTopic actualiza el prefijo con el que el dispositivo crea los topicos de Fire por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttFireTopic") {

      memset(mqttFireTopicValue, 0, sizeof mqttFireTopicValue);
      strncpy(mqttFireTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt Fire topic prefix is now: " + String(mqttFireTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getMqttTroubleTopic devuelve el topico en el que el dispositivo publica el estado de Trouble
     */
   
    if (coMMand == "getMqttTroubleTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttTroubleTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttTroubleTopic actualiza el topico en el que el dispositivo publica el estado de Trouble por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttTroubleTopic") {

      memset(mqttTroubleTopicValue, 0, sizeof mqttTroubleTopicValue);
      strncpy(mqttTroubleTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt Trouble topic is now: " + String(mqttTroubleTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getMqttCommandTopic devuelve el topico en el que el dispositivo escucha commandos.
     */
   
    if (coMMand == "getMqttCommandTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttCommandTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttCommandTopic actualiza el topico en el que el dispositivo escucha comandos por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttCommandTopic") {

      memset(mqttCommandTopicValue, 0, sizeof mqttCommandTopicValue);
      strncpy(mqttCommandTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt Commands topic is now: " + String(mqttCommandTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getMqttKeepAliveTopic devuelve topico en el que el dispositivo pubica el mensaje de keepalive.
     */
   
    if (coMMand == "getMqttKeepAliveTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(mqttKeepAliveTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMqttKeepAliveTopic actualiza el topico en el que el dispositivo publica el mensaje de keepalive por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMqttKeepAliveTopic") {

      memset(mqttKeepAliveTopicValue, 0, sizeof mqttKeepAliveTopicValue);
      strncpy(mqttKeepAliveTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt keepAlive topic is now: " + String(mqttKeepAliveTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getEnableMonitoring devuelve valor de inicialilzacin de la funcionalidad de monitoreo.
     */
   
    if (coMMand == "getEnableMonitoring") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(enableMonitoringValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateEnableMonitoring actualiza el valor de inicializacin de la funcionalidad de monitoreo por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateEnableMonitoring") {

      memset(enableMonitoringValue, 0, sizeof enableMonitoringValue);
      strncpy(enableMonitoringValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt monitoring is now: " + String(enableMonitoringValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getMonitoringTopic devuelve prefijo con el que el dispositivo crea los topicos para publicar la informacin de monitoreo.
     */
   
    if (coMMand == "getMonitoringTopic") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(monitoringTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateMonitoringTopic actualiza el prefijo con el que el dispositivo crea los topicos para publicar la informacion de monitoreo por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateMonitoringTopic") {

      memset(monitoringTopicValue, 0, sizeof monitoringTopicValue);
      strncpy(monitoringTopicValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Mqtt monitoring topic prefix is now: " + String(monitoringTopicValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


/////////// TIMER

 /*    
     *   El comando getTimerStatus devuelve el valor correspondiente al estado del timer.
     */
   
    if (coMMand == "getTimerStatus") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(tiMerStatusValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateTimerStatus actualiza el valor correspondiente al estado del timer por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateTimerStatus") {

      memset(tiMerStatusValue, 0, sizeof monitoringTopicValue);
      strncpy(tiMerStatusValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Timer status is now: " + String(tiMerStatusValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getTimerString devuelve el timer string de la funcionalidad Timer.
     */
   
    if (coMMand == "getTimerString") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(tiMerStringValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateTimerString actualiza el timer string de la funcionalidad Timer por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateTimerString") {

      memset(tiMerStringValue, 0, sizeof tiMerStringValue);
      strncpy(tiMerStringValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Timer string updated";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getPublishTimerString devuelve el valor para la opcion de publicacion de timer string.
     */
   
    if (coMMand == "getPublishTimerString") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(publishTimerStringValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updatePublishTimerString actualiza el valor para la opcione de publicacin del timer string por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updatePublishTimerString") {

      memset(publishTimerStringValue, 0, sizeof publishTimerStringValue);
      strncpy(publishTimerStringValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Publish Timer String value is now: " + String(publishTimerStringValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

////////// END TIMER

///////// NTP

  /*    
     *   El comando getNtpServer devuelve el ntp server configurado.
     */
   
    if (coMMand == "getNtpServer") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(ntpServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateNtpServer actualiza el valor del NTP server por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateNtpServer") {

      memset(ntpServerValue, 0, sizeof ntpServerValue);
      strncpy(ntpServerValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "NTP server is now: " + String(ntpServerValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getTimeZone devuelve el valor de la time zone configurada.
     */
   
    if (coMMand == "getTimeZone") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(timeZoneValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateTimeZone actualiza el valor de la time zone configurada por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateTimeZone") {

      memset(timeZoneValue, 0, sizeof timeZoneValue);
      strncpy(timeZoneValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Time Zone is now: " + String(timeZoneValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }

  /*    
     *   El comando getNtpUpdateInterval devuelve el valor en segundos del intervalo de consulta al NTP server.
     */
   
    if (coMMand == "getNtpUpdateInterval") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(ntpUpdateIntervalValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateNtpUpdateInterval actualiza el valor en segundos del intervalo de consulta al NTP server por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateNtpUpdateInterval") {

      memset(ntpUpdateIntervalValue, 0, sizeof ntpUpdateIntervalValue);
      strncpy(ntpUpdateIntervalValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "NTP update interval is now: " + String(ntpUpdateIntervalValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }


  /*    
     *   El comando getTimeDST devuelve el valor del parametro DST.
     */
   
    if (coMMand == "getTimeDST") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = String(timeDSTValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */

    }
    
    /*    
     *   El comando updateTimeDST actualiza el valor del parametro DST por el proporcionado en campo parametro pram1.
     *   Para que los cambios persistan en el dispositivo luefo de que este se reinicie los cambios deberan ser guardados enviando el comando saveConfig
     */
    if (coMMand == "updateTimeDST") {

      memset(timeDSTValue, 0, sizeof timeDSTValue);
      strncpy(timeDSTValue, paRam1.c_str(), String(paRam1).length());

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "DST value is now: " + String(timeDSTValue);
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;

      /* --------------- SerialDebug: --------- */
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Remote Config result message sent: " + RConfCommandResult, false, atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
    }



////////// END NTP



     /*    COMANDOS QUE FALTA IMPLEMENTAR
      * 
      * getTiMerStatus  tiMerStatusValue
      * updateTiMerStatus: param1 = new timer status
      * getTiMerString   tiMerStringValue
      * updateTiMerString: param1 = add/del, param2 = target timer string (ademas este comenado tiene que verificar que entren los strings en 98 caracteres)
      * getPublishTimerString publisgTimerStringValue
      * updatePublishTimerString param1 = new publisgTimerString value
      * 
      * getNtpServer   ntpServerValue
      * updateNtpServer: param1 =  new ntp server
      * getTimeZone   timeZoneValue
      * updateTimeZone: param1 = new timezone
      * 
      * get y update el intervalo de update del server
      * get (readable y timestring) y set la hora  
      * force update del NTP server
      * 
      * getTimeDST   timeDSTValue
      * updateTimeDST: param1 = new DST state
      * 
      * 
      * getHideSSID   hideSSIDValue
      * updateHideSSID: param1 = new hide SSID value (no implementar)
      * 
      */







    /*
     * El comando saveConfig guarda la configuracin para que persista luego de un reinicio del dispositivo y lo resetea
     */

    if (coMMand == "saveConfig") {

      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Config saved. Rebooting...";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      
      iotWebConf.saveConfig();
     }

     /*
     * El comando reset reinicia el dispositivo
     */

    if (coMMand == "reset") {
      
      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "OK";
      jsonBuffer["comments"] = "Rebooting in 5 seconds...";
      
      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      ++commandResponses;
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
      
      needReset = true;
    }


    
   /*
    * Error comando desconocido
    */
    if (commandResponses == 0) {
      
      jsonBuffer["deviceId"] = String(deviceIdValue);
      jsonBuffer["command"] = coMMand;
      jsonBuffer["response"] = "FAIL";
      jsonBuffer["comments"] = "Unknown command";
      /*
      JsonArray& data = rootOut.createNestedArray("data");
      data.add(48.756080);
      data.add(2.302038);
      */

      serializeJson(jsonBuffer, RConfCommandResult);
      
      mqttClientRConf.publish(remoteConfigResultTopicValue, RConfCommandResult, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));
      Serial.println((String) "Mensaje enviado. Topic: " + remoteConfigResultTopicValue + " Payload: " + RConfCommandResult );
    }

    Serial.println("Se proceso mensaje de configuracion remota y se enviaron " +  String(commandResponses) + " mensajes de respuesta.");
    
  }
}
