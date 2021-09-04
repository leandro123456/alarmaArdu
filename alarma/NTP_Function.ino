

///NTP

String getTimeString() {
  String timeString = String(weekday()) + String(year());
  if (month()<10){timeString = timeString + "0" + month();} else {timeString = timeString + month();}
  if (day()<10){timeString = timeString + "0" + day();} else {timeString = timeString + day();}
  if (hour()<10){timeString = timeString + "0" + hour();} else {timeString = timeString + hour();}
  if (minute()<10){timeString = timeString + "0" + minute();} else {timeString = timeString + minute();}
  if (second()<10){timeString = timeString + "0" + second();} else {timeString = timeString + second();}
  return timeString;
}

String getReadableTime() {

  String ReadableTime;

  if (weekday() == 1) {ReadableTime = "Sunday, " + String(year());}
  if (weekday() == 2) {ReadableTime = "Monday, " + String(year());}
  if (weekday() == 3) {ReadableTime = "Tuesday, " + String(year());}
  if (weekday() == 4) {ReadableTime = "Wednesday, " + String(year());}
  if (weekday() == 5) {ReadableTime = "Thursday, " + String(year());}
  if (weekday() == 6) {ReadableTime = "Friday, " + String(year());}
  if (weekday() == 7) {ReadableTime = "Saturday, " + String(year());}
  
  if (month() == 1){ReadableTime = ReadableTime + " January ";}
  if (month() == 2){ReadableTime = ReadableTime + " February ";}
  if (month() == 3){ReadableTime = ReadableTime + " March ";}
  if (month() == 4){ReadableTime = ReadableTime + " April ";}
  if (month() == 5){ReadableTime = ReadableTime + " May ";}
  if (month() == 6){ReadableTime = ReadableTime + " June ";}
  if (month() == 7){ReadableTime = ReadableTime + " July ";}
  if (month() == 8){ReadableTime = ReadableTime + " August ";}
  if (month() == 9){ReadableTime = ReadableTime + " September ";}
  if (month() == 10){ReadableTime = ReadableTime + " October ";}
  if (month() == 11){ReadableTime = ReadableTime + " November ";}
  if (month() == 12){ReadableTime = ReadableTime + " December ";}
      
  ReadableTime = ReadableTime + day() + " - ";
  if (hour()<10){ReadableTime = ReadableTime + "0" + hour();} else {ReadableTime = ReadableTime + hour();}
  if (minute()<10){ReadableTime = ReadableTime + ":0" + minute();} else {ReadableTime = ReadableTime + ":" + minute();}
  if (second()<10){ReadableTime = ReadableTime + ":0" + second();} else {ReadableTime = ReadableTime + ":" + second();}
  
  return ReadableTime;
}
    
/*-------- NTP code ----------*/



time_t getNtpTime(){
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
    /* --------------- SerialDebug: --------- */
  //Serial.println((String)"Sending NTP request to " + ntpServerValue + " (" + ntpServerIP + ")");
  Serial.println((String)"Sending NTP request to " + ntpServerValue);
  /* --------------- SerialDebug: --------- */
  /* --------------- mqttDebug: --------- */
  //if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Sending NTP request to " + ntpServerValue + " (" + ntpServerIP + ")", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Sending NTP request to " + ntpServerValue, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  /* --------------- mqttDebug: --------- */
  // get a random server from the pool
  WiFi.hostByName(ntpServerValue, ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      /* --------------- SerialDebug: --------- */
      Serial.println("NTP response received");
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - NTP response received", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + atoi(timeZoneValue) * SECS_PER_HOUR;
    }
  }
  //Serial.println("No NTP Response :-(");
  /* --------------- SerialDebug: --------- */
  Serial.println("No response from NTP server");
  /* --------------- SerialDebug: --------- */
  /* --------------- mqttDebug: --------- */
  if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - No response from NTP server", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
  /* --------------- mqttDebug: --------- */
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address){
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
 ///// Fin NTP

//Timer
 void runTimer() {

String tmrStr = String(tiMerStringValue);

  if(tmrStr.length()%7 == 0) {
      int eventos = tmrStr.length()/7;
    
      if (timeStatus() != timeNotSet) {
      
            for (int i = 0; i < eventos; i++) {
                String timerEventString = tmrStr.substring(i*7,(i*7)+7);
            
                // Aca va la magia de cada evento
                
    
    /*
    Domingo = 1 = G, H, J, P
    Lunes = 2 = A, H, I, K, L, N, P
    Martes = 3 = B, H, I, K, M, N
    Miercoles = 4 = C, H, I, K, L, N, P
    Jueves = 5 = D, H, I, K, M, O
    Viernes = 6 = E, H, I, K, L, O, P
    Sabado = 7 = F, H, J, K, M, O
    
    getTimeString().substring(9,13) == timerEventString.substring(1,3) + timerEventString.substring(3,5)
    
    Accion: timerEventString.substring(5,6)
    switch: timerEventString.substring(6)
     */
    
    
                  if (
                      (
                      (String(weekday()) == "1" && (                      //Domingo = 1 = G, H, J, P
                      timerEventString.substring(0,1) == "G" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "J" ||
                      timerEventString.substring(0,1) == "P" ))
                      ||
                      (String(weekday()) == "2" && (                      //Lunes = 2 = A, H, I, K, L, N, P
                      timerEventString.substring(0,1) == "A" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "I" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "L" ||
                      timerEventString.substring(0,1) == "N" ||
                      timerEventString.substring(0,1) == "P" ))
                      ||
                      (String(weekday()) == "3" && (                      //Martes = 3 = B, H, I, K, M, N
                      timerEventString.substring(0,1) == "B" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "I" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "M" ||
                      timerEventString.substring(0,1) == "N" ))
                      ||
                      (String(weekday()) == "4" && (                      //Miercoles = 4 = C, H, I, K, L, N, P
                      timerEventString.substring(0,1) == "C" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "I" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "L" ||
                      timerEventString.substring(0,1) == "N" ||
                      timerEventString.substring(0,1) == "P" ))
                      ||
                      (String(weekday()) == "5" && (                      //Jueves = 5 = D, H, I, K, M, O
                      timerEventString.substring(0,1) == "D" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "I" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "M" ||
                      timerEventString.substring(0,1) == "O" ))
                      ||
                      (String(weekday()) == "6" && (                      //Viernes = 6 = E, H, I, K, L, O, P
                      timerEventString.substring(0,1) == "E" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "I" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "L" ||
                      timerEventString.substring(0,1) == "O" ||
                      timerEventString.substring(0,1) == "P" ))
                      ||
                      (String(weekday()) == "7" && (                      //Sabado = 7 = F, H, J, K, M, O
                      timerEventString.substring(0,1) == "F" ||
                      timerEventString.substring(0,1) == "H" ||
                      timerEventString.substring(0,1) == "J" ||
                      timerEventString.substring(0,1) == "K" ||
                      timerEventString.substring(0,1) == "M" ||
                      timerEventString.substring(0,1) == "O" ))
                      )
                      &&
                      (getTimeString().substring(9,13) == timerEventString.substring(1,3) + timerEventString.substring(3,5))
                      &&
                      timerEventDone[i] == 0
                    )
                    {
                      /* --------------- SerialDebug: --------- */
                      Serial.println("Executing Timer event: " + timerEventString);
                      /* --------------- SerialDebug: --------- */
                      /* --------------- mqttDebug: --------- */
                      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Executing Timer event: " + timerEventString, (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
                      /* --------------- mqttDebug: --------- */

                      /////////// Magic! 

                      // La particion esta en el timerEventString.substring(6)
                              
                      if (timerEventString.substring(5,6) == "1"){  //Action Armar Stay
                        //Armar Stay
                        controlDSC("arm_stay",timerEventString.substring(6).toInt()); 
                      }

                      if (timerEventString.substring(5,6) == "2"){  //Action Armar Away
                        //Armar Away
                        controlDSC("arm_away",timerEventString.substring(6).toInt()); 
                      }
                      
                      if (timerEventString.substring(5,6) == "0"){  //Action desarmar
                        //desarmar
                        controlDSC("disarm",timerEventString.substring(6).toInt());
                      }
                      
                      ////////////// End Magic!
                      timerEventDone[i] = 1;
                      return;
                    }
                    else if (getTimeString().substring(9,13) != timerEventString.substring(1,3) + timerEventString.substring(3,5) && timerEventDone[i] == 1) { 

                      timerEventDone[i] = 0; }
            
            
            }
            }
      
      else {
          /* --------------- SerialDebug: --------- */
          Serial.println("Timer: No DateTime info from NTP yet. Nothing can be done!");
          /* --------------- SerialDebug: --------- */
          /* --------------- mqttDebug: --------- */
          if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - Timer: No DateTime info from NTP yet. Nothing can be done!", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
          /* --------------- mqttDebug: --------- */
      }
    }
    if(tmrStr.length()%7 != 0 || tmrStr.length() == 0) {
      /* --------------- SerialDebug: --------- */
      Serial.println("ERROR: Wrong Timer String or not specified");
      /* --------------- SerialDebug: --------- */
      /* --------------- mqttDebug: --------- */
      if (atoi(enableMqttDebugValue) == 1) {mqttClientRConf.publish(MqttDebugTopicValue, (String) deviceIdValue + " - ERROR: Wrong Timer String or not specified", (bool) atoi(remoteConfigRetainValue), atoi(remoteConfigQoSValue));}
      /* --------------- mqttDebug: --------- */   
  }
  

}
