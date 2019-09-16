void manualon_check()
{
  
   Serial.println("CHECKING MANUAL");  
    if (on_manual_timer == 0)
  {check_alarm.enable();
  mode =3;
  }
  
  
  // Serial.println(tim_off);
 else if (on_manual_timer == 1)
  {
    Serial.println("Maual: - turn lights on");
    digitalWrite(relay1, 1);
    relay_flag_manual = 1;
    relay_flag = 1;
    mode =1;    
  }
 else if (on_manual_timer == 2)
  {
    Serial.println("CHECKING TIMER");
  
 
  if ( relay_flag == 0 && relay_flag_manual==0 && timer_duration != 0)
    { 
  Serial.println(timer_duration);
  double set_dur= double(timer_duration)/60;
  timer_duration= set_dur;
  Serial.println(set_dur);
  on_timer.enable();
  off_timer.enableDelayed(TASK_MINUTE *  set_dur);
  mode = 2;
  }
  } 
}
