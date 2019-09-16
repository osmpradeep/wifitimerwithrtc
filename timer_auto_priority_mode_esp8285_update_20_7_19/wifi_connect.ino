void connect_wifi()
{
 wl_status_t status = WiFi.status();
 bool ret=false;
    if(status == WL_DISCONNECTED || status == WL_NO_SSID_AVAIL || status == WL_IDLE_STATUS || status == WL_CONNECT_FAILED) {
        int8_t scanResult = WiFi.scanComplete();
        if(scanResult == WIFI_SCAN_RUNNING) {
            status = WL_NO_SSID_AVAIL;
            connect_wifi_t.disable();
          //  connect_wifi_t.stop();
           // return status;
           ret = true;
        } 
        if(scanResult == 0 && ret == false ) {  
          WiFi.scanDelete();
          delay(0);
          WiFi.disconnect();
          WiFi.scanNetworks(true);
        //  return status;
         ret = true;
        } 
       if(scanResult > 0 && ret == false) {   
            Serial.println("[WIFI] scan done");
            Serial.printf("[WIFI] %d networks found\n", scanResult);
           for(int8_t i = 0; i < scanResult; ++i) {
                
                String ssid_scan;
                int32_t rssi_scan;
                uint8_t sec_scan;
                uint8_t* BSSID_scan;
                int32_t chan_scan;
                bool hidden_scan;
                Serial.println("checking");
                WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan, hidden_scan);
                if( ssid_scan = wifiSSID)
                {
                WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str(),6);
                status = WiFi.status();
                i = scanResult;
                } 
              
            }
            WiFi.scanDelete();  
            connect_wifi_t.disable();
           // return status;
           ret =true;
        }
      //  WiFi.disconnect(); 
      if(ret == false){
        WiFi.scanNetworks(true);
        connect_wifi_t.disable();
        }
    }
    connect_wifi_t.disable();
}
