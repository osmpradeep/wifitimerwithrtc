bool change_name_password()
{
  
   String data = server.arg("AP_NP");//Set Config
  Serial.println(data);
  delay(10);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data);
  if (!json.success()) {
    Serial.println("Failed to parse");
    server.send(272, "text/plane", "NAME_CHANGE_FAILED");
    delay(10);
    return false;
  } 
  String ret="DEVICE NAME CHANGED.DEVICE RESTARTING NOW";
  server.send(273, "text/plane", ret);
  delay(2000);
  String ap_name          =    String  (json["AP_NAME"].as<String>());
  String ap_pwd           =    String  (json["AP_PWD"].as<String>());
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {  Serial.println("Failed to open config file");return false;}
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer_write;
  JsonObject& json_write = jsonBuffer_write.parseObject(buf.get());
  if (!json_write .success()) {Serial.println("Failed to parse config file");}
  json_write  ["AP_NAME"]   =       ap_name ;
  json_write  ["AP_PWD"]    =       ap_pwd  ;
  String output;
  json_write .printTo(output);
  File myFile = SPIFFS.open("/config.json", "w");
  if (myFile) {
    myFile.print(output);
    myFile.close();
    Serial.println("done.");
  } else {Serial.println("error opening config.json");}
  Serial.println("SET done.");
  //delay(10);
  _restart.enableDelayed(TASK_SECOND * 1);
  //ESP.restart();
  return true;  
  
  }
