bool Set_Config() {
  String data = server.arg("SCF");//Set Config
  Serial.println(data);
  delay(10);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data);
  if (!json.success()) {
    Serial.println("Failed to parse");
    server.send(401, "text/plane", "CONFIG_FAILED");
    delay(10);
    return false;
  }
  server.send(201, "text/plane", "CONFIG_COMPLETED.DEVICE RESTARTING NOW ");
  delay(2000);
  String t_ssid                 =    String  (json["SSID"].as<String>());
  String t_password             =    String  (json["PWD"].as<String>());
 // String t_device_id            =    String  (json["DEVICE_ID"].as<String>());
  String t_gateway              =    String  (json["GATEWAY"].as<String>());
  String t_subnetmask           =    String  (json["SUBNET"].as<String>());
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {  Serial.println("Failed to open config file");return false;}
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer_write;
  JsonObject& json_write = jsonBuffer_write.parseObject(buf.get());
  if (!json_write .success()) {Serial.println("Failed to parse config file");}
  json_write  ["SSID"]        =       t_ssid  ;
  json_write  ["PWD"]         =       t_password;
  //json_write  ["DEVICE_ID"]   =       t_device_id;
  json_write  ["GATEWAY"]     =       t_gateway;
  json_write  ["SUBNET"]      =       t_subnetmask;
  String output;
  json_write .printTo(output);
  File myFile = SPIFFS.open("/config.json", "w");
  if (myFile) {
    myFile.print(output);
    myFile.close();
    Serial.println("done.");
  } else {Serial.println("error opening config.json");}
  Serial.println("SET done.");
 // delay(10);
  _restart.enableDelayed(TASK_SECOND * 1);
 // ESP.restart();
  return true;
}
