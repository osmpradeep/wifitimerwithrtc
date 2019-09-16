void device_info()
{
  String w_status;
  String input = "{\"CURRENT\":\"ON\",\"WIFI_STATUS\":\"3\"}";
    if (WiFi.status() != WL_CONNECTED) { 
    Serial.println("NOT CONNECTED");
     w_status= "NOT CONNECTED"  ; 
  }
  else if (WiFi.status() == WL_CONNECTED) {
    Serial.println("NOT CONNECTED");
    w_status= "CONNECTED"  ; 
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(input);
  root["CURRENT"]      =   String (current);
  root["WIFI_STATUS"]  =   String (w_status);
  String output;
  root.printTo(output);
  server.send(503, "text/plane",output); 
  }
