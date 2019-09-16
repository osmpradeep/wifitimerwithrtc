
void read_rtc_Time_date()
{  
  // Reset the register pointer
  Wire.beginTransmission(rtc);
  Wire.write(0x10);
  Wire.endTransmission(); 
  Wire.requestFrom(rtc, 7);
  // A few of these need masks because certain bits are control bits
  rtc_second     = bcdToDec(Wire.read());
  rtc_minute     = bcdToDec(Wire.read());
  rtc_hour       = bcdToDec(Wire.read());  // Need to change this if 12 hour am/pm
  rtc_day        = bcdToDec(Wire.read());
  rtc_date       = bcdToDec(Wire.read());
  rtc_month      = bcdToDec(Wire.read());
  rtc_year       = bcdToDec(Wire.read());
  Serial.print(rtc_hour);
  Serial.print(":");
  Serial.print(rtc_minute);
  Serial.print(":");
  Serial.print(rtc_second);
  Serial.print("  ");
  Serial.print(rtc_month);
  Serial.print("/");
  Serial.print(rtc_date);
  Serial.print("/");
  Serial.print(rtc_year);
  Serial.print("  Day_of_week:");
  Serial.println(rtc_day);
}
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

void set_rtc_Time_date(byte second, byte minute, byte hour, byte day, byte date, byte month,byte year) // 0-59,0-59,1-23,1-7,1-28/29/30/31,1-12,0-99
{
   Wire.beginTransmission(rtc);
   Wire.write(0x10);
   Wire.write(decToBcd(second));    // 0 to bit 7 starts the clock
   Wire.write(decToBcd(minute));
   Wire.write(decToBcd(hour));     
   Wire.write(decToBcd(day));
   Wire.write(decToBcd(date));
   Wire.write(decToBcd(month));
   Wire.write(decToBcd(year));
   Wire.endTransmission();
}
