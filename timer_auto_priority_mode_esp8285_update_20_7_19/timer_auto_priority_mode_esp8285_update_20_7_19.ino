#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "StringSplitter.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <ESP8266WiFiMulti.h>
#include "FS.h"
#include <DNSServer.h>
//#include "SPIFFS.h"
#include <TaskScheduler.h>
#include <DNSServer.h>
#include <elapsedMillis.h>
#include <PinButton.h>
#include "Wire.h"
#define rtc 0x32
#define FIRMWARE_VERSION  " "
#define sample 100
#define overload_cutoff 14
ESP8266WebServer server; //Server on port 80
AlarmId alarm_on[60], alarm_off[60], id1;
AlarmId timer_on, timer_off;
Scheduler runner;
elapsedMillis timeElapsed;
ESP8266WiFiMulti wifiMulti;
PinButton myButton(12);
DNSServer dnsServer;
const char* serverIndex= "<html><head><title>WIFI SWITCH</title><meta http-equiv=\"refresh\" content = \"2; url = http://wifiswitch.io/update.html\" /></head><body><p>LOADING PLEASE WAIT</p></body></html>";
//const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
// Callback methods prototypes
bool w_status = false;
const char* ssid = "";
const char* password = "";
const char* ssidAP = "TIMER_SWITCH";
const char* passwordAP = "timer_test";
String mode1;
String fota_web_address = "";
String nextfrimware = "/update.bin";
int mode;
int tim_on;
int tim_off;
uint16_t rtc_second, rtc_minute, rtc_hour, rtc_date, rtc_month, rtc_year, rtc_day;
double current = 0;
int adc_buf[sample];
int ci = 1;
int adc_avg = 0;
int on_manual_timer ;
double duration, timer_duration;
int r_on[56];
int r_off[56];
int ip[4];
uint16_t relay_flag = 0, check = 0;
uint16_t relay_flag_manual=0, relay_flag_timer=0;
int gateway[4];
int subnet[4];
int device_id;
int wifi;
String wifiSSID, wifiPassword;
String APSSID, APPassword;
//int relay1 = 4;
//int led=0;
uint64_t chipid;
uint16_t relay1 = 5;
uint16_t led = 4;
uint16_t led2 = 13;
//uint16_t relay1 = 5;
//uint16_t led=4;
uint16_t fd = 0;
uint16_t od = 0;
uint16_t reset = 0, reset_init = 0;
const byte DNS_PORT = 53;
byte mac[6];
String sl;
String mac2;
String filesystem = "FS_SPIFFS";
void alarm_check();
void reset_mode();
void RESTART();
void _wifi_status();
void _find_device();
void _on_device();
void _on_wifi();
void digitalClockDisplay();
void ota();
void fota();
void _on_timer();
void _off_timer();
void rem_timer();
void crnt_measurement();
void connect_wifi();
void led_sync();
void led_fota();
Task FOTA(TASK_SECOND, TASK_FOREVER, &fota);
Task REM_TIMER(TASK_SECOND, TASK_FOREVER, &rem_timer);
Task on_timer(TASK_SECOND, TASK_FOREVER, &_on_timer);
Task off_timer(TASK_SECOND, TASK_FOREVER, &_off_timer);
Task find_device(TASK_SECOND, TASK_FOREVER, &_find_device);
Task check_alarm(TASK_SECOND, TASK_FOREVER, &alarm_check);
Task wifi_status(TASK_SECOND, TASK_FOREVER, &_wifi_status);
Task mode_reset(TASK_SECOND, TASK_FOREVER,  &reset_mode);
Task _restart(TASK_SECOND, TASK_FOREVER,  &RESTART);
Task clock_display(TASK_SECOND, TASK_FOREVER,  &digitalClockDisplay);
Task current_measurement(TASK_SECOND, TASK_FOREVER,  &crnt_measurement);
Task on_device(TASK_SECOND, TASK_FOREVER,  &_on_device);
Task on_wifi(TASK_SECOND, TASK_FOREVER,  &_on_wifi);
Task connect_wifi_t(TASK_SECOND, TASK_FOREVER,  &connect_wifi);
Task sync_led(TASK_SECOND, TASK_FOREVER,  &led_sync);
Task fota_led(TASK_SECOND, TASK_FOREVER,  &led_fota);
struct alarm_daily_day
{
  uint16_t dayoftheweek_on;
  uint16_t houroftheday_on;
  uint16_t minuteoftheday_on;
  uint16_t dayoftheweek_off;
  uint16_t houroftheday_off;
  uint16_t minuteoftheday_off;
  String   days;
} _channel[8];

void setup() {
  pinMode(relay1, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(0, INPUT);
  digitalWrite(led2, 0);
  // relay_flag_manual=0;
  Wire.begin(2, 14);
  delay(10);
  Serial.begin(74880);
  delay(10);
  // Serial.setDebugOutput(1);
  if (!SPIFFS.begin()) {
    Serial.println("Failed: File System Initialize");
  } else {
    Serial.println("File System Initialized");
  }

  
  if (!loadConfig()) {
    Serial.println("Failed to load config");
  } else {
    Serial.println("Config loaded");
  }
  //  delay(10);
  //  Serial.println(tim_off);
  if (device_id == 0) {
    device_id = 201;
  }
  IPAddress ipaddres(gateway[1], gateway[2], gateway[3], device_id);//Node static IP
  IPAddress gateway_ip(gateway[1], gateway[2], gateway[3], gateway[4]);
  IPAddress subnet_mask(subnet[1], subnet[2], subnet[3], subnet[4]);
  IPAddress staticIP(192, 168, 4, device_id);
  IPAddress gatewayAP(192, 168, 4, device_id );
  IPAddress subnetAP(255, 255, 255, 0);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  ESP.eraseConfig();

  
  if (!WiFi.config(ipaddres, gateway_ip, subnet_mask)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.softAPConfig(staticIP, gatewayAP, subnet_mask);
  Serial.println("WiFi began");
  WiFi.macAddress(mac);
  for (int i = 0; i <= 5; i++) {
    sl = decToHex(mac[i], 1);
    mac2 = mac2 + sl;
  }
  mac2.toUpperCase();
  if (APSSID.length() == 0)
  {
    APSSID = "TIMER_" + mac2;
  }
  // APPassword = "timer_test" ;
  WiFi.mode(WIFI_AP_STA);
  // WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str(),10);
  wifiMulti.addAP(wifiSSID.c_str(), wifiPassword.c_str());
  // WiFi.enableAP(true);
  WiFi.softAP(APSSID.c_str(), APPassword.c_str(), 6, false);
  Serial.println("Connecting Wifi...");
  int retry;
  int retry_delay=0;
  if(wifi==0){retry=5;retry_delay=1000;}else if(wifi==1){retry==1;retry_delay=0;}
  for (int c = 1 ; c <= retry ; c++)
  { digitalWrite(led, !digitalRead(led));
    Serial.println(".");
    if (wifiMulti.run() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      c = 5;
    }else {delay(retry_delay);}
  }
  digitalWrite(led, 0);
  delay(100);
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //Serial.println(tim_off);
  Serial.println("Auto connect");
  // WiFi.setAutoReconnect(false);
  // Serial.print(WiFi.getAutoConnect());

  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT, "wifiswitch.io", WiFi.softAPIP());
 
  
  read_rtc_Time_date();
  //setTime(20, 29, 0, 2, 1, 11);
  setTime(rtc_hour, rtc_minute, rtc_second, rtc_date, rtc_month, rtc_year);
  relay_flag = 0;


 server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
    });
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "UPDATE FAILED" : "Device update completed...! \tDevice Restarting now");
     // ESP.restart9();
     Serial.println(Update.hasError());
      if(Update.hasError()==0){
         Serial.println("UPDATE COMPLETED");
      digitalWrite(led, 0);
      digitalWrite(led2, 0);
      wifi_status.disable();
      on_wifi.disable();
      on_device.disable();
      sync_led.disable();
      fota_led.enable();
     _restart.enableDelayed(TASK_SECOND * 5);
      }
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
       // Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      //  Serial.setDebugOutput(false);
      }
      yield();
    });
 
  server.on("/SET_CONF",       Set_Config);
  //server.on("/SET_MODE",       Mode_Change);
  server.on("/SET_DATE_TIME",  Set_Date_Time);
  server.on("/SET_REALY_TIME", Set_Relay_Time);
  server.on("/CLEAR_ALL",      clear_all_relay);
  server.on("/GET_DATE_TIME",  Send_Date_Time);
  server.on("/GET_STATUS",     Send_Status);
  server.on("/CLR_RELAY_CH",   Clear_Channel);
  server.on("/FIND_DEVICE",    device_find);
  server.on("/M_ON",           manual_on);
  server.on("/CHECK_UPDATE",   ota);
  server.on("/SLEEP",         _sleep);
  server.on("/TIMER",         _timer);
  server.on("/GET_VERSION",    get_version);
  server.on("/DEVICE_ID",      change_devcie_id);
  server.on("/CNG_NP",         change_name_password);
  server.on("/RESET_DEVICE",   reset_data);
  server.on("/DEVICE_INFO",    device_info);
  server.onNotFound(handleWebRequests);
  server.begin();
  Serial.println("HTTP server started");
  Serial.println(tim_off);
  int f = 0;
  for (f = 0; f <= 7; f++) {
    setup_alarm(f);
  }
  runner.init();
  Serial.println("Initialized scheduler");
  runner.addTask(check_alarm);
  runner.addTask(mode_reset);
  runner.addTask(_restart);
  runner.addTask(wifi_status);
  runner.addTask(find_device);
  runner.addTask(clock_display);
  runner.addTask(FOTA);
  runner.addTask(on_timer);
  runner.addTask(off_timer);
  runner.addTask(REM_TIMER);
  runner.addTask(on_device);
  runner.addTask(current_measurement);
  runner.addTask(on_wifi);
  runner.addTask(connect_wifi_t);
  runner.addTask(sync_led);
  runner.addTask(fota_led);
  clock_display.enable();
  on_wifi.enable();
  on_device.enable();
  sync_led.enable();
  relay_flag_timer=0;
  manualon_check();
  current_measurement.enable();
  wifi_status.enable();
  reset = 0;
  Serial.println("Working Mode");
  if(wifi==1){ 
     _sleep();
    }
}

void loop(void) {
  server.handleClient();
  runner.execute();
  dnsServer.processNextRequest();
  myButton.update();
  if (current >=overload_cutoff)
  {
    if (relay_flag == 1) {
      Serial.println("Overload :Device Turned off");
      digitalWrite(relay1, 0);
      relay_flag_manual = 0;
      relay_flag_timer = 0;
      relay_flag = 0;
      duration = 0;
      mode = 3;
      on_timer.disable();
      off_timer.disable();
      REM_TIMER.disable();
      sync_led.enable();
      Set_Data("0", "MANUAL_TIMER");
      Set_Data("0" , "REM_TIME");
    }
  }
  if (myButton.isSingleClick()) {
    Serial.println("single");
    if (relay_flag == 0) {
      Serial.println("Maual: - turn lights on");
      digitalWrite(relay1, 1);
      relay_flag_manual = 1;
      relay_flag = 1;
      Set_Data("1", "MANUAL_TIMER");
      mode = 1;
    }
    else if (relay_flag == 1) {
      Serial.println("Maual: - turn lights off");
      digitalWrite(relay1, 0);
      relay_flag_manual = 0;
      relay_flag_timer = 0;
      relay_flag = 0;
      duration = 0;
      mode = 3;
      on_timer.disable();
      off_timer.disable();
      REM_TIMER.disable();
       sync_led.enable();
      Set_Data("0", "MANUAL_TIMER");
      Set_Data("0" , "REM_TIME");
    }
  }

  if (myButton.isDoubleClick())
  {
    Serial.println("double");
    WiFiMode_t prepareWiFi_m = WiFi.getMode();
    Serial.println(prepareWiFi_m);
    if (prepareWiFi_m == 0)
    {
      WiFi.forceSleepWake();
      wifi_setup();
    }
    else  if (prepareWiFi_m == 1 || prepareWiFi_m == 2 || prepareWiFi_m == 3) {
      _sleep();

    }
  }

  if (myButton.isLongClick()) {
    Serial.println("long");
    reset_data();
  }
  Alarm.delay(10);
  delay(20);


}

void wifi_setup()
{

  if (device_id == 0) {
    device_id = 201;
  }
  IPAddress ipaddres(gateway[1], gateway[2], gateway[3], device_id);//Node static IP
  IPAddress gateway_ip(gateway[1], gateway[2], gateway[3], gateway[4]);
  IPAddress subnet_mask(subnet[1], subnet[2], subnet[3], subnet[4]);
  IPAddress staticIP(192, 168, 4, device_id);
  IPAddress gatewayAP(192, 168, 4, device_id);
  IPAddress subnetAP(255, 255, 255, 0);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  ESP.eraseConfig();
  if (!WiFi.config(ipaddres, gateway_ip, subnet_mask)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.softAPConfig(staticIP, gatewayAP, subnet_mask);
  Serial.println("WiFi began");
  WiFi.macAddress(mac);
  for (int i = 0; i <= 5; i++) {
    sl = decToHex(mac[i], 1);
    mac2 = mac2 + sl;
  }
  mac2.toUpperCase();
  if (APSSID.length() == 0)
  {
    APSSID = "TIMER_" + mac2;
  }
  // APPassword = "timer_test" ;

  WiFi.persistent( false );
  WiFi.mode(WIFI_OFF);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str(), 6);
  //wifiMulti.addAP(wifiSSID.c_str(), wifiPassword.c_str());
  // WiFi.enableAP(true);
  WiFi.softAP(APSSID.c_str(), APPassword.c_str(), 10, false);
  Serial.println("Connecting Wifi...");
  wifi_status.enable();
  on_wifi.enable();
  sync_led.enable();
  Set_Data("0", "WIFI");
}
void get_version() {
  String Version =  "Current Version : " + String (FIRMWARE_VERSION);
  server.send(250, "text/plane", Version);
}
bool _timer()
{
  String data2 = server.arg("DUR");
  Serial.println(data2 );
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data2);
  if (!json.success()) {
    Serial.println("Failed to parse");
    return false;
  }
  timer_duration       =    int  (json["DURATION"]);
  if (timer_duration != 0 ) {
    int today = weekday();
    timer_on  = Alarm.alarmRepeat(today, hour(), minute(), second(), check_data);
    tim_on = Alarm.read(timer_on);
    //timer_off = timer_on + (timer_duration * 60);
    tim_off = Alarm.read(timer_on) + (timer_duration * 60);
    //Serial.println(tim_on );
    //Serial.println(tim_off);
    Set_Data(String(Alarm.read(timer_on)) , "TIMER_ON");
    Set_Data(String(Alarm.read(timer_on) + (timer_duration * 60) ), "TIMER_OFF");
    on_timer.enable();
    off_timer.enableDelayed(TASK_MINUTE * timer_duration);
    if ( relay_flag_manual == 0)
      server.send(600, "text/plane", "TIMER ON");
    else
      server.send(602, "text/plane", "Device already Turned ON Manually");
  }
}
void _on_timer() {
  if ( relay_flag_manual == 0) {

    digitalWrite(relay1, 1);
    relay_flag_timer = 1;
    relay_flag = 1;
    Serial.println("TIMER ON" );
    mode = 2;
    on_timer.disable();
    duration = timer_duration * 60;
    Serial.println(duration);
    REM_TIMER.enable();
    //on_device.enable();
    Set_Data("2", "MANUAL_TIMER");
    Set_Data(String(duration) , "REM_TIME");
  }
}
void _off_timer() {
  if ( relay_flag_manual == 0) {
    digitalWrite(relay1, 0);
    relay_flag_timer = 0;
    relay_flag = 0;
    mode = 3;
    on_timer.disable();
    //on_device.enable();
    Serial.println("TIMER OFF" );
    off_timer.disable();
    REM_TIMER.disable();
    sync_led.enable();
    Set_Data("0", "MANUAL_TIMER");
    Set_Data("0" , "TIMER_ON");
    Set_Data("0", "TIMER_OFF");
    Set_Data("0" , "REM_TIME");
    check_alarm.enable();
  }
}
void _sleep() {
  Serial.flush();
  wifi_status.disable();
  server.send(222, "text/plane", "WIFI TURNED OFF");
  delay(100);
  WiFi.disconnect();
  WiFi.forceSleepBegin();
  delay(30);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    digitalWrite(led, LOW);
  }
  wifi_status.disable();
  on_wifi.disable();
  Set_Data("1", "WIFI");
  //  delay(30);
}
void device_find() {
  on_wifi.disable();
  find_device.enable();
  server.send(210, "text/plane", "find");
  Serial.println("FIND");
}
bool manual_on()
{
  String data2 = server.arg("MAN");
  // Serial.println(data2);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data2);
  if (!json.success()) {
    Serial.println("Failed to parse");
    return false;
  }
  String manual       =    String  (json["MANUAL"].as<String>());

  if (manual == "ON") {
    Serial.println("Maual: - turn lights on");
    digitalWrite(relay1, 1);
    //on_device.enable();
    relay_flag_manual = 1;
    relay_flag = 1;
    mode = 1;
    //  server.send(503, "text/plane", "SWITCHED ON");
    server.send(291, "text/plane", "SWITCHED ON");
    delay(20);
    Set_Data("1", "MANUAL_TIMER");
    return true;
  }
  if (manual == "OFF") {
    Serial.println("Maual: - turn lights off");
    digitalWrite(relay1, 0);
    //on_device.enable();
    relay_flag_manual = 0;
    relay_flag_timer = 0;
    relay_flag = 0;
    duration = 0;
    mode = 3;
    on_timer.disable();
    off_timer.disable();
    // server.send(504, "text/plane", "SWITCHED OFF");
    server.send(292, "text/plane", "SWITCHED OFF");
    delay(20);
    REM_TIMER.disable();
    sync_led.enable();
    Set_Data("0", "MANUAL_TIMER");
    Set_Data("0" , "REM_TIME");
    //check_alarm.enable();
    return true;
  }
  //server.send(501, "text/plane", "PLZ CHANGE MODE");
}

void _find_device() {
  digitalWrite(led, !digitalRead(led));
  find_device.setInterval( TASK_MILLISECOND * 100);
  fd++;
  if (fd == 6)
  {
    fd = 0;
    find_device.disable();
    on_wifi.enable();
    sync_led.enable();
  }
}


void check_data()
{}
void  RESTART() {
  digitalWrite(relay1, 0);
  digitalWrite(led, 0);
  digitalWrite(led2, 0);
  ESP.restart();
}
void reset_mode()
{
  //  String Str1 = "AP_MODE";
  //  Set_Data(Str1, "MODE");
  //ESP.restart();
  _restart.enableDelayed(TASK_SECOND * 1);
}
void handleWebRequests() {
  if (loadFromSpiffs(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  //  Serial.println(message);
}

void alarm_check() {
  int today = weekday();
  id1 = Alarm.alarmRepeat(today, hour(), minute(), second(), check_data);
  int _now = Alarm.read(id1);
  // Serial.println(_now);
  Alarm.disable(id1);
  Alarm.free(id1);


  for (int k = 1 ; k <= 56; k++)
  {
    r_on[k] = Alarm.read(alarm_on[k]);
    r_off[k] = Alarm.read(alarm_off[k]);
    //Serial.println(r_on[k]);
    // Serial.println(r_off[k]);
  }
  for (int l = 1; l <= 56; l++)
  {
    if (_now >= r_on[l] && _now < r_off[l] && r_on[l] != -1  && r_off[l] != -1 && r_on[l] != r_off[l] && relay_flag == 0 && relay_flag_manual == 0 && relay_flag_timer == 0   )
    {
      Serial.println("ON TIME ");
      Serial.println(relay_flag_manual);
      onled();
      Serial.println(l);
      Serial.println(r_on[l]);
      Serial.println(r_off[l]);
      Serial.println(_now);
      Serial.println("SET");
    }
  }
  check_alarm.disable();
}
void check_state(int x) {
  int today = weekday();
  id1 = Alarm.alarmRepeat(today, hour(), minute(), second(), check_data);
  int _now = Alarm.read(id1);
  // Serial.println(_now);
  Alarm.disable(id1);
  Alarm.free(id1);
  int temp = x + 6;
  for (int k = x ; k <= temp; k++)
  {
    r_on[k] = Alarm.read(alarm_on[k]);
    r_off[k] = Alarm.read(alarm_off[k]);
  }
  for (int l = x; l <= temp; l++)
  {
    if (_now >= r_on[l] && _now < r_off[l] && r_on[l] != -1  && r_off[l] != -1 && r_on[l] != r_off[l] && relay_flag == 1 )
    {
      Serial.println("ch clear");
      Serial.println(relay_flag_manual);
      offled();
    }
  }

}
bool  Clear_Channel()
{
  int ch = 0;
  int data = 1;
  String data2 = server.arg("CRT");//Set Relay Time
  Serial.println(data2);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data2);
  if (!json.success()) {
    Serial.println("Failed to parse");
    return false;
  }
  String t_channel       =    String  (json["CHANNEL"].as<String>());
  String file_name = "";

  if (t_channel == "1") {
    ch = 0;
    data = 1;
    file_name = "/data1.json";
  }
  else if (t_channel == "2") {
    ch = 1;
    data = 8;
    file_name = "/data2.json";
  }
  else if (t_channel == "3") {
    ch = 2;
    data = 15;
    file_name = "/data3.json";
  }
  else if (t_channel == "4") {
    ch = 3;
    data = 22;
    file_name = "/data4.json";
  }
  else if (t_channel == "5") {
    ch = 4;
    data = 29;
    file_name = "/data5.json";
  }
  else if (t_channel == "6") {
    ch = 5;
    data = 36;
    file_name = "/data6.json";
  }
  else if (t_channel == "7") {
    ch = 6;
    data = 43;
    file_name = "/data7.json";
  }
  else if (t_channel == "8") {
    ch = 7;
    data = 50;
    file_name = "/data8.json";
  }
  if (relay_flag == 1) {
    check_state(data);
  }
  File configFile = SPIFFS.open(file_name, "r");
  if (!configFile) {
    Serial.println("Failed to open data file");
    return false;
  }
  server.send(204, "text/plane", "CLEAR_COMPLETED");
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer_write;
  JsonObject& json_write = jsonBuffer_write.parseObject(buf.get());
  if (!json_write .success()) {
    Serial.println("Failed to parse config file");
  }
  json_write  ["CHANNEL"]  = t_channel;
  json_write  ["R1_ON_HR"] = 0;
  json_write  ["R1_ON_MT"] = 0;
  json_write  ["R1_OF_HR"] = 0;
  json_write  ["R1_OF_MT"] = 0;
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
  int temp = data + 6;
  for (int m = data; m <= temp; m++) {
    Alarm.free(alarm_on[m]);
    Alarm.free(alarm_off[m]);
    alarm_on[m]  = dtINVALID_ALARM_ID;
    alarm_off[m] = dtINVALID_ALARM_ID;
  }
  Serial.println("CLEAR done.");
  check_alarm.enable();
  return true;
}

void onled() {
  Serial.println("Alarm");
  if ( relay_flag_manual == 0 && relay_flag_timer == 0 ) {
    Serial.println("Alarm: - turn lights on");
    digitalWrite(relay1, 1);
    relay_flag = 1;
  }
}

void offled() {
  Serial.println("Alarm");
  if ( relay_flag_manual == 0 && relay_flag_timer == 0   ) {
    Serial.println("Alarm: - turn lights off");
    digitalWrite(relay1, 0);
     sync_led.enable();
    relay_flag = 0;
    
  }
}
bool loadFromSpiffs(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/")) path += "index.html";
  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".txt")) dataType = "text/html";
  else if (path.endsWith(".json")) dataType = "text/json";
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }
  dataFile.close();
  return true;
}
//void Mode_Change(){
//
//  String str = server.arg("MODE");
//  DynamicJsonBuffer jsonBuffer;
//  JsonObject& root = jsonBuffer.parseObject(str);
//  if (!root.success()) {
//    Serial.println("Failed to parse config file");
//  }
//  String Str1 =  String  (root["MODE"].as<String>());
//  mode= int(root["MODE"]);
//  server.send(207, "text/plane", "MODE CHANGED");
//  Set_Data(Str1, "MODE");
//  digitalWrite(relay1, 0);
//  relay_flag = 0;
//  relay_flag_manual =0;
//  relay_flag_timer = 0;
//  if(mode==3)
//  { check_alarm.enable();}
//  Serial.println("Working Mode");
//  Serial.print(mode);
//}
void Send_Date_Time() {
  String AM_PM;
  String input = "{\"YEAR\":\"2018\",\"MONTH\":\"12\",\"DAY\":\"15\",\"HOUR\":\"1\",\"MINUTE\":\"15\",\"SECOND\":\"15\",\"AM_PM\":\"AM\"}";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(input);
  if (!root.success()) {
    Serial.println("Failed to parse config file");
  }
  if (isAM() == true) {
    AM_PM = "AM";
  } else if (isAM() == false) {
    AM_PM = "PM";
  }
  Serial.println(AM_PM);
  root ["YEAR"]       = String (year())  ;
  root ["MONTH"]      = String (month());
  root ["DAY"]        = String (day())  ;
  root ["HOUR"]       = String (hourFormat12());
  root ["MINUTE"]     = String (minute())  ;
  root ["SECOND"]     = String (second());
  root ["AM_PM"]      = String (AM_PM);
  String output;
  root.printTo(output);
  server.send(301, "text/plane", output);
}

void Send_Status()
{

  double set_dur;

  // Serial.println("CHECKING TIMER");
  //   loadConfig();
  //   int today = weekday();
  //   id1 = Alarm.alarmRepeat(today, hour(), minute(), second(), check_data);
  //   int _now = Alarm.read(id1);
  //   Alarm.disable(id1);
  //   Alarm.free(id1);
  //  if (_now >= tim_on && _now < tim_off && tim_on  != -1  && tim_off  != -1 && tim_on != tim_off  && relay_flag == 1 && relay_flag_timer ==1 )
  //    {
  //   duration= tim_off- tim_on;
  ////  Serial.println(duration);
  //  set_dur= (duration - (_now-tim_on ))/60;
  //  }

  set_dur = duration / 60;
  //Serial.println(set_dur);
  String input = "{\"RELAY1\":\"ON\",\"REM_TIME\":\"3\",\"MODE\":\"3\"}";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(input);
  if (!root.success()) {
    Serial.println("Failed to parse config file");
  }
  if (relay_flag_manual == 1) {
    mode = 1;
  } else if (relay_flag_timer == 1) {
    mode = 2;
  } else {
    mode = 3;
  }
  root["RELAY1"]    =   String (relay_flag);
  root["MODE"]      =   String (mode);
  root["REM_TIME"]  =   String (set_dur);
  String output;
  root.printTo(output);
  if (relay_flag == 1) {
    // server.send(500, "text/plane", output);
    server.send(288, "text/plane", output);
  }
  else {
    // server.send(501, "text/plane", output);
    server.send(289, "text/plane", output);
  }
}
bool Set_Date_Time() {
  String data = server.arg("SDT");//Set Date and Time
  Serial.println(data);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data);
  if (!json.success()) {
    Serial.println("Failed to parse");
    return false;
  }
  int t_year            =    int (json["YEAR"]);
  int t_month           =    int (json["MONTH"]);
  int t_day             =    int (json["DAY"]);
  int t_hour            =    int (json["HOUR"]);
  int t_minute          =    int (json["MINUTE"]);
  int t_second          =    int (json["SECOND"]);

  setTime(t_hour, t_minute, t_second , t_day , t_month, t_year); //// set time
  int t_weekday = weekday();
  set_rtc_Time_date(t_second, t_minute, t_hour , t_weekday, t_day , t_month , t_year);
  read_rtc_Time_date();
  setTime(rtc_hour, rtc_minute, rtc_second, rtc_date, rtc_month, rtc_year);
  server.send(202, "text/plane", "TIME UPDATED");
  if (mode == 3)
  {
    check_alarm.enableDelayed(TASK_SECOND * 1);
  }
  return true;
}
bool Set_Relay_Time() {
  int ch = 0;
  int data = 0;
  String data2 = server.arg("SRT");//Set Relay Time
  Serial.println(data2);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data2);
  if (!json.success()) {
    Serial.println("Failed to parse");
    return false;
  }
  String t_channel       =    String  (json["CHANNEL"].as<String>());
  String t_on_hour       =    String  (json["HOUR_ON"].as<String>());
  String t_on_minute     =    String  (json["MINUTE_ON"].as<String>());
  String t_off_hour      =    String  (json["HOUR_OFF"].as<String>());
  String t_off_minute    =    String  (json["MINUTE_OFF"].as<String>());
  String t_days          =    String  (json["DAYS"].as<String>());
  String file_name = "";

  if (t_channel == "1") {
    ch = 0;
    data = 1;
    file_name = "/data1.json";
  }
  else if (t_channel == "2") {
    ch = 1;
    data = 8;
    file_name = "/data2.json";
  }
  else if (t_channel == "3") {
    ch = 2;
    data = 15;
    file_name = "/data3.json";
  }
  else if (t_channel == "4") {
    ch = 3;
    data = 22;
    file_name = "/data4.json";
  }
  else if (t_channel == "5") {
    ch = 4;
    data = 29;
    file_name = "/data5.json";
  }
  else if (t_channel == "6") {
    ch = 5;
    data = 36;
    file_name = "/data6.json";
  }
  else if (t_channel == "7") {
    ch = 6;
    data = 43;
    file_name = "/data7.json";
  }
  else if (t_channel == "8") {
    ch = 7;
    data = 50;
    file_name = "/data8.json";
  }

  int k = data + 6;
  if ( relay_flag == 1 && mode == 3) {
    int today = weekday();
    id1 = Alarm.alarmRepeat(today, hour(), minute(), second(), check_data);
    int _now = Alarm.read(id1);
    Serial.println("checking");
    Alarm.disable(id1);
    Alarm.free(id1);
    for (int l = data; l <= k; l++)
    { Serial.println("checking 2");
      Serial.println(l);
      if (_now >= r_on[l] && _now < r_off[l] && r_on[l] != -1  && r_off[l] != -1 && r_on[l] != r_off[l]  )
      {
        Serial.println(r_on[l]);
        Serial.println(r_off[l]);
        Serial.println("Alarm: - turn lights off");
        digitalWrite(relay1, 0);
        relay_flag = 0;
        sync_led.enable();
      }
    }
  }
  File configFile = SPIFFS.open(file_name, "r");
  if (!configFile) {
    Serial.println("Failed to open data file");
    return false;
  }
  server.send(203, "text/plane", "SET_COMPLETED");
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer_write;
  JsonObject& json_write = jsonBuffer_write.parseObject(buf.get());
  if (!json_write .success()) {
    Serial.println("Failed to parse config file");
  }
  json_write  ["CHANNEL"]  = t_channel;
  json_write  ["R1_ON_HR"] = t_on_hour;
  json_write  ["R1_ON_MT"] = t_on_minute;
  json_write  ["R1_OF_HR"] = t_off_hour;
  json_write  ["R1_OF_MT"] = t_off_minute;
  json_write  ["R1_NO_DY"] = t_days ;
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
  setup_alarm(ch);
  Serial.println("SET done.");
  check_alarm.enable();
  return true;
}

bool Set_Data(String getdata, String id)
{
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer_write;
  JsonObject& json_write = jsonBuffer_write.parseObject(buf.get());
  String input = String(getdata);
  if (!json_write .success()) {
    Serial.println("Failed to parse config file");
  }
  json_write  [id] = input;
  String output;
  json_write .printTo(output);
  File myFile = SPIFFS.open("/config.json", "w");
  if (myFile) {
    myFile.print(output);
    myFile.close();
    // Serial.println("done.");
  } else {
    Serial.println("error opening config.json");
  }
  return true;
}


bool clear_all_relay()
{
  server.send(206, "text/plane", "CLAER_COMPLETED");
  delay(10);
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
  for (int m = 1; m <= 56; m++) {
    Alarm.free(alarm_on[m]);
    Alarm.free(alarm_off[m]);
    alarm_on[m] = dtINVALID_ALARM_ID;
    alarm_off[m] = dtINVALID_ALARM_ID;
  }
  offled();
  check_alarm.enable();
}
void digitalClockDisplay() {
  // digital clock display of the time
  String AM_PM = "";
  if (isAM() == true) {
    AM_PM = "AM";
  } else if (isAM() == false) {
    AM_PM = "PM";
  }

  Serial.print(hourFormat12());
  printDigits(minute());
  printDigits(second());
  Serial.print(" " + AM_PM);
  Serial.println();
  //  Serial.println("IO : ");
  //  Serial.println(digitalRead(0));
  //  Serial.println(digitalRead(button));
  clock_display.setInterval( TASK_SECOND * 10);
  //  Serial.println(Alarm.read(alarm_on[7]));
  //  Serial.println(Alarm.read(alarm_off[7]));
  //  Serial.println(Alarm.read(alarm_on[14]));
  //  Serial.println(Alarm.read(alarm_off[14]));
}

void printDigits(int digits) {
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);


}

bool setup_alarm(int i) {
  int data = 0;
  String file_name = "";
  if (i == 0) {
    data = 1;
    file_name = "/data1.json";
  }
  if (i == 1) {
    data = 8;
    file_name = "/data2.json";
  }
  if (i == 2) {
    data = 15;
    file_name = "/data3.json";
  }
  if (i == 3) {
    data = 22;
    file_name = "/data4.json";
  }
  if (i == 4) {
    data = 29;
    file_name = "/data5.json";
  }
  if (i == 5) {
    data = 36;
    file_name = "/data6.json";
  }
  if (i == 6) {
    data = 43;
    file_name = "/data7.json";
  }
  if (i == 7) {
    data = 50;
    file_name = "/data8.json";
  }
  File configFile = SPIFFS.open(file_name, "r");
  if (!configFile) {
    Serial.println("Failed to open data file");
    return false;
  }
  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  //Serial.println(buf.get());
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }
  _channel[i].houroftheday_on     =   int       (json["R1_ON_HR"]);
  _channel[i].minuteoftheday_on   =   int       (json["R1_ON_MT"]);
  _channel[i].houroftheday_off    =   int       (json["R1_OF_HR"]);
  _channel[i].minuteoftheday_off  =   int       (json["R1_OF_MT"]);
  _channel[i].days                =   String    (json["R1_NO_DY"].as<String>());

  if ( _channel[i].days == "SMTWXFY"  ) {
    for (int j = 1; j <= 7; j++) {
      set_alarm_on(data , j, _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      set_alarm_off(data , j , _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
      data = data + 1;
      Serial.printf("alarm set on : %d:%d ", _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      Serial.printf("alarm set off : %d:%d ", _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
    }
  }
  else if ( _channel[i].days != "SMTWXFY"  )
  {
    if ( _channel[i].days.substring(0, 1) == "S") {
      set_alarm_on(data , 1, _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      set_alarm_off(data , 1 , _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
      data = data + 1;
      Serial.printf("alarm set on : %d:%d ", _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      Serial.printf("alarm set off : %d:%d ", _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
    }
    else {
      clear_alarm(data);
      data = data + 1;
    }
    if ( _channel[i].days.substring(1, 2) == "M" ) {
      set_alarm_on(data , 2, _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      set_alarm_off(data , 2 , _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
      data = data + 1;
      Serial.printf("alarm set on : %d:%d ", _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      Serial.printf("alarm set off : %d:%d ", _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
    }
    else {
      clear_alarm(data);
      data = data + 1;
    }
    if (  _channel[i].days.substring(2, 3) == "T" ) {
      set_alarm_on(data , 3, _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      set_alarm_off(data , 3 , _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
      data = data + 1;
      Serial.printf("alarm set on : %d:%d ", _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      Serial.printf("alarm set off : %d:%d ", _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
    }
    else {
      clear_alarm(data);
      data = data + 1;
    }
    if ( _channel[i].days.substring(3, 4) == "W") {

      set_alarm_on(data , 4, _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      set_alarm_off(data , 4 , _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
      data = data + 1;
      Serial.printf("alarm set on : %d:%d ", _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      Serial.printf("alarm set off : %d:%d ", _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
    }
    else {
      clear_alarm(data);
      data = data + 1;
    }
    if (_channel[i].days.substring(4, 5) == "X" ) {

      set_alarm_on(data , 5, _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      set_alarm_off(data , 5 , _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
      data = data + 1;
      Serial.printf("alarm set on : %d:%d ", _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      Serial.printf("alarm set off : %d:%d ", _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);

    }
    else {
      clear_alarm(data);
      data = data + 1;
    }
    if ( _channel[i].days.substring(5, 6) == "F") {

      set_alarm_on(data , 6, _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      set_alarm_off(data , 6 , _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
      data = data + 1;
      Serial.printf("alarm set on : %d:%d ", _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      Serial.printf("alarm set off : %d:%d ", _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
    }
    else {
      clear_alarm(data);
      data = data + 1;
    }
    if ( _channel[i].days.substring(6, 7) == "Y" ) {
      //Serial.printf("data : %d", data);
      set_alarm_on(data , 7, _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      set_alarm_off(data , 7 , _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
      data = data + 1;
      Serial.printf("alarm set on : %d:%d ", _channel[i].houroftheday_on, _channel[i].minuteoftheday_on);
      Serial.printf("alarm set off : %d:%d ", _channel[i].houroftheday_off, _channel[i].minuteoftheday_off);
    }
    else {
      clear_alarm(data);
      data = data + 1;
    }

  }
  return true;
}

void set_alarm_on(int x, int day, int hour, int minute)
{
  alarm_on[x] = Alarm.alarmRepeat(day, hour, minute, 0, onled);
  Serial.println(Alarm.read(alarm_on[x]));
}
void set_alarm_off(int x, int  day, int hour, int minute)
{
  alarm_off[x] = Alarm.alarmRepeat(day, hour, minute, 0, offled);
  Serial.println(Alarm.read(alarm_off[x]));
}
void clear_alarm(int m)
{
  int today = weekday();
  alarm_on[m] = Alarm.alarmRepeat(today, hour(), minute(), second(), check_data);

  alarm_off[m] = Alarm.alarmRepeat(today, hour(), minute(), second(), check_data);
  Alarm.disable(alarm_on[m]);
  Alarm.disable(alarm_off[m]);
  Alarm.free(alarm_on[m]);
  Alarm.free(alarm_off[m]);
  Serial.println("Clear");
}
void _wifi_status()
{

  if (wifiMulti.run() != WL_CONNECTED) {
    //connect_wifi();
    // connect_wifi_t.enable();
    Serial.println("WiFi not connected!");
     if(w_status == true)
    {
      w_status=false;
      sync_led.enable();
      
      }
    
    // digitalWrite(led, LOW);
    //wifi_state_change=true;
    // delay(1000);
  }
  else if (WiFi.status() == WL_CONNECTED) {
   // digitalWrite(led, HIGH);
   w_status= true;
    //  Serial.println("WiFi  connected!");
  }
  wifi_status.setInterval( TASK_SECOND * 12);
}

void _on_wifi() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(led, HIGH);
    on_wifi.setInterval( TASK_MILLISECOND * 1);
  } else if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led, !digitalRead(led));
    on_wifi.setInterval( TASK_MILLISECOND * 800);
  }
}

void _on_device() {
  if (digitalRead(relay1) == false) {
    digitalWrite(led2, !digitalRead(led2));
    on_device.setInterval( TASK_MILLISECOND * 800);
  } else if (digitalRead(relay1) == true) {
    digitalWrite(led2, 1);
    on_device.setInterval( TASK_MILLISECOND * 1);
  }
}

void led_sync(){ 

     WiFiMode_t prepareWiFi_m = WiFi.getMode();
    Serial.println(prepareWiFi_m); 
if (prepareWiFi_m != 0 && WiFi.status() != WL_CONNECTED  && digitalRead(relay1) == false) { 
   digitalWrite(led, LOW);
   digitalWrite(led2, LOW);
   on_wifi.disable();
   on_device.disable();
   on_wifi.enable();
   on_device.enable();
  Serial.println("Sync");
  }
  sync_led.disable();
}
void led_fota(){
     digitalWrite(led, !digitalRead(led));
    digitalWrite(led2, !digitalRead(led2));
    fota_led.setInterval( TASK_MILLISECOND * 100);
}
String decToHex(byte decValue, byte desiredStringLength) {

  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;

  return hexString;
}
