bool reset_data(){

  
  int data;
  String file_name = "";
  for (int i = 0 ; i <= 7; i++) {
    if (i == 0) {
      data = 1;
      file_name = "/data1.json";
    }
    else  if (i == 1) {
      data = 8;
      file_name = "/data2.json";
    }
    else if (i == 2) {
      data = 15;
      file_name = "/data3.json";
    }
    else if (i == 3) {
      data = 22;
      file_name = "/data4.json";
    }
    else if (i == 4) {
      data = 29;
      file_name = "/data5.json";
    }
    else if (i == 5) {
      data = 36;
      file_name = "/data6.json";
    }
    else if (i == 6) {
      data = 43;
      file_name = "/data7.json";
    }
    else  if (i == 7) {
      data = 50;
      file_name = "/data8.json";
    }
    File configFile = SPIFFS.open(file_name, "r");
    if (!configFile) {
      Serial.println("Failed to open data file");
      return false;
    }
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer_write;
    JsonObject& json_write = jsonBuffer_write.parseObject(buf.get());
    if (!json_write .success()) {
      Serial.println("Failed to parse config file");
    }
    json_write  ["R1_ON_HR"] = 0;
    json_write  ["R1_ON_MT"] = 0;
    json_write  ["R1_OF_HR"] = 0;
    json_write  ["R1_OF_MT"] = 0;
    json_write  ["R1_ON_HR"] = 0;
    json_write  ["R1_NO_DY"] = 0;
    String output;
    json_write .printTo(output);
    File myFile = SPIFFS.open(file_name, "w");
    if (myFile) {
      myFile.print(output);
      myFile.close();
      Serial.println("done.");
    } else {
      Serial.println("error opening data.json");
    } 
    Serial.println("alaram clear done.");
    } 
 String resetdata="{\"SSID\":\"TEST1234\",\"PWD\":\"test1234\",\"GATEWAY\":\"192.168.1.1\",\"SUBNET\":\"255.255.255.0\",\"MODE\":\"2\",\"DEVICE_ID\":\"201\",\"MANUAL_TIMER\":\"0\",\"TIMER_ON\":\"129761\",\"TIMER_OFF\":\"130661.00\",\"REM_TIME\":\"0\",\"AP_NAME\":\"\",\"AP_PWD\":\"\",\"WIFI\":\"0\"}";
  File myFile = SPIFFS.open("/config.json", "w");
  if (myFile) {
    myFile.print(resetdata);
    myFile.close();
    Serial.println("done.");
  } else {Serial.println("error opening config.json");}
  Serial.println("SET done.");
  Serial.println("RESTTING PLZ WAIT");
  server.send(278, "text/plane", "DEVICE RESET COMPLETED. DEVICE RESTARTING NOW");
  on_wifi.disable();
  find_device.enable();
  digitalWrite(led2,0);
 // delay(2000);  
  _restart.enableDelayed(TASK_SECOND * 1);
  }
