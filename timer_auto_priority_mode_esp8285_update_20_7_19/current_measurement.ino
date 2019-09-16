void crnt_measurement()
{
  int Current_Adc= analogRead(A0);
  //Serial.println(Current_Adc);
  if(ci<=sample)
  {
    adc_buf[ci]=Current_Adc;
    adc_avg = adc_avg +adc_buf[ci];
    ci++;
    if(ci==sample +1)
    {    
      adc_avg = adc_avg / sample;
   //  current= ((adc_avg) * 63.5)/(double)(3905.61);
     current= ((adc_avg)/(double)1023) * 15.6;
     // Serial.println(adc_avg);
     // Serial.println(current);
      ci=1;
      adc_avg = 0;
      }
    } 
current_measurement.setInterval( TASK_MILLISECOND * 8);
}
