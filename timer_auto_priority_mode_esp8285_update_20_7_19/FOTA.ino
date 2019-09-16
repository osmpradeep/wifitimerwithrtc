void ota()
{
   
   String data = server.arg("ADDRESS");
   fota_web_address= data;
   Set_Data(data ,"WEB_UP_IP");
   FOTA.enable(); 
}

 
 void fota(){
if ((WiFi.status() == WL_CONNECTED)) {       
            Serial.println("CHECKING UPDATE");
            String address= fota_web_address+nextfrimware;
            Serial.println(address);
            t_httpUpdate_return ret = ESPhttpUpdate.update(address);
            switch (ret) {
            case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            server.send(501, "text/plane", ESPhttpUpdate.getLastErrorString().c_str());
            
            break;        
            case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            server.send(501, "text/plane", "NO_UPDATES");
            break;
            case HTTP_UPDATE_OK:            
            delay(100);
            server.send(502, "text/plane", "UPDATE COMPLETED. DEVICE RESTARTING");
            delay(1000);
            Serial.println("HTTP_UPDATE_OK");        
            WiFi.disconnect();
           // delay(1000);
            _restart.enableDelayed(TASK_SECOND * 1);
           // ESP.restart();
            break;
        }
      }
   FOTA.disable(); 
 }
