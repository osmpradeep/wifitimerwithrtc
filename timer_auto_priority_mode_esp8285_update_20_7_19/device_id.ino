bool change_devcie_id()
{
  
  String data = server.arg("ID");//Set Config
  Serial.println(data);
  delay(10);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data);
  if (!json.success()) {
    Serial.println("Failed to parse");
    server.send(270, "text/plane", "ID_CHANGE_FAILED");
    delay(10);
    return false;
  } 
  String ret="DEVICE ID CHANGED TO " + data + "DEVICE RESTARTING NOW";
  server.send(271, "text/plane", ret);
  delay(2000);
  String t_device_id            =    String  (json["DEVICE_ID"].as<String>());
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {  Serial.println("Failed to open config file");return false;}
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer_write;
  JsonObject& json_write = jsonBuffer_write.parseObject(buf.get());
  if (!json_write .success()) {Serial.println("Failed to parse config file");}
  json_write  ["DEVICE_ID"]   =       t_device_id;
  String output;
  json_write .printTo(output);
  File myFile = SPIFFS.open("/config.json", "w");
  if (myFile) {
    myFile.print(output);
    myFile.close();
    Serial.println("done.");
  } else {Serial.println("error opening config.json");}
  Serial.println("SET done.");
  _restart.enableDelayed(TASK_SECOND * 1);
  return true;  
  }


 
