bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {Serial.println("Failed to open config file");return false; }
  size_t size = configFile.size();
  if (size > 1024) {Serial.println("Config file size is too large");return false;}
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  if (!json.success()) {Serial.println("Failed to parse config file");return false;}
  wifi=int (json["WIFI"]);
  device_id  = int (json["DEVICE_ID"]);
  on_manual_timer    = int (json["MANUAL_TIMER"]);
  timer_duration     = double(json["REM_TIME"]);
  tim_on    = int (json["TIMER_ON"]);
  tim_off   = int (json["TIMER_OFF"]);
  fota_web_address= String  (json["WEB_UP_IP"].as<String>());
  wifiSSID =String  (json["SSID"].as<String>());
  wifiPassword=String  (json["PWD"].as<String>());
  APSSID= String  (json["AP_NAME"].as<String>());
  APPassword = String  (json["AP_PWD"].as<String>());
  {
    String gateway_data                 =   String  (json["GATEWAY"].as<String>());
    Serial.println(gateway_data );
    StringSplitter *splitter1 = new StringSplitter(gateway_data  , '.', 4);
    int itemCount = splitter1->getItemCount();
    for (int i = 0; i < itemCount; i++) {
      String item = splitter1->getItemAtIndex(i);
      // Serial.println("Item @ index" + String(i) + ": " + String(item));
      String  temp = String(item);
      gateway[i + 1] = temp.toInt();
    }
  }
  {
    String subnet_data     =   String  (json["SUBNET"].as<String>());
    Serial.println(subnet_data );
    StringSplitter *splitter2 = new StringSplitter(subnet_data  , '.', 4);
    int itemCount = splitter2->getItemCount();
    for (int i = 0; i < itemCount; i++) {
      String item = splitter2->getItemAtIndex(i);
      // Serial.println("Item @ index" + String(i) + ": " + String(item));
      String  temp = String(item);
      subnet[i + 1] = temp.toInt();
    }
  }
  Serial.println("conf load done.");
  return true;
}
