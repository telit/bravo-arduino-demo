//#==============================================================================================================================#
//#                        >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved.  <<<
//#
//#
//#
//#@brief
//#The file contains the main user entry point and all the functions to use the module with the Bravo Board
//#
//#
//#@details
//# -The correct xml file, must be present inside the modem, in this example is "object_25261.xml", must be uploaded manually before execute this script
//# -The first executed step is verify if the modem is power ON or OFF.
//# -The Network connection provides a network registration.
//# -echo_demo function opens a PDP context, sends a sample string to an echo server and prints out the returned string.
//# -lwm2m_demo function open a lwm2m connection enabling the lwm2m client on the module, reads data from the BOSH sensors on the Bravo board and write them in the LWM2M portal.
//#
//#
//#@author
//#Alessio Quieti

#define M2M_INFO Serial.println 
#define Arduino_Serial Serial
#define Modem_Serial Serial1

String APN = "web.omnitel.it";

String result_AT;
String result_Data;
int res = 0;
int i=1;

//--------------Global Variables-----------------------
int    g_lwm2m = 0;
String g_temp = "" ;
String g_press = "";
String g_hum = "" ;
String g_airQ = "";

int channel_is_on = A3; //Connection to pin3 to check the power of the module

void setup() 
{
  SerialUSB.begin(115200);
  Modem_Serial.begin(115200);
  delay(10000);  
  
  M2M_INFO("########## Welcome to Bravo Board Project with Arduino ##########\r\n");
  
  //Check if the module is power on/off
  while(gpio_check()== -1)
  {
      delay(2000); 
  }
  network_connection();
  
}

void loop() 
{
}

int gpio_check()
{
  M2M_INFO("Gpio Check Called");
  int val;
  val = analogRead(channel_is_on);  // read the input pin
  if(val==0)
  {
    M2M_INFO("Module is power Off");
    return -1;
  }
  else
  {
    M2M_INFO("Module is power ON");
    delay(20000);
    return 0;  
  }
}

void extract_value(String string_to_analyze)
{
  strtok((char*)string_to_analyze.c_str(),",");
  g_temp = strtok(NULL,",");
  g_press = strtok(NULL,",");
  g_hum = strtok(NULL,",");
  g_airQ = strtok(NULL,"\r\n");
  
}
void lwm2m_demo()
{
  int counter=0; 
  String cmd_to_send_temp="";
  String cmd_to_send_press="";
  String cmd_to_send_hum ="";
  String cmd_to_send_airQ="";
  
  result_AT ="";
  result_AT = sendATcommand("AT#LWM2MENA?",1000); 
  
  if(result_AT.indexOf("ACTIVE")>0)
  {
    M2M_INFO("LWM2M Client Enabled");
  }
   else
  {
     M2M_INFO("LWM2M Client Not Enabled");
     result_AT ="";
     result_AT = sendATcommand("AT#LWM2MENA=1",5000);
     M2M_INFO("Calling lwm2m_demo ");
     lwm2m_demo();
  }  
  while(counter<10)
  {
      cmd_to_send_temp  = "";
      cmd_to_send_press = "";
      cmd_to_send_hum   = "";
      cmd_to_send_airQ  = "";
      
      result_AT = "";
      result_AT = sendATcommand("AT#BSENS=1",1000); 
      extract_value(result_AT);
      
      delay(2000);
      
      cmd_to_send_temp.concat("AT#LWM2MSET=1,26251,0,1,0,");
      cmd_to_send_temp.concat(g_temp);
      
      cmd_to_send_press.concat("AT#LWM2MSET=1,26251,0,2,0,");
      cmd_to_send_press.concat(g_press) ;
      
      cmd_to_send_hum.concat("AT#LWM2MSET=1,26251,0,3,0,");
      cmd_to_send_hum.concat(g_hum) ;
      
      cmd_to_send_airQ.concat("AT#LWM2MSET=0,26251,0,4,0,");
      cmd_to_send_airQ.concat(g_airQ);
      
      sendATcommand(cmd_to_send_temp.c_str(),2000);
      sendATcommand(cmd_to_send_press.c_str(),2000);
      sendATcommand(cmd_to_send_hum.c_str(),2000);
      sendATcommand(cmd_to_send_airQ.c_str(),2000);
      
      counter = counter +1;
      delay(2000);
  }
  
  sendATcommand("AT#LWM2MENA=0",10000);
  M2M_INFO("Application Ended");
  exit(0);
}

void echo_demo()
{
  M2M_INFO("Echo Demo Called");

  result_AT ="";
  result_AT = sendATcommand("AT#SGACT?",1000); 

  int active_context =0;
  delay(1000);

  active_context = atoi(result_AT.substring(result_AT.indexOf(":")+4).c_str()); 
  
  if(active_context == 0)
  {
    active_context = 0;
    M2M_INFO("CONTEXT NOT ACTIVE");
    result_AT ="";
    result_AT = sendATcommand("AT#SGACT=1,1",1000); 
    
    if(result_AT.indexOf("ERROR")== -1)
    {
      active_context =1;
    }
  }
  else
  {
    active_context = 1;
    M2M_INFO("Context already activated");
  }
    if(active_context == 1 )
    {
       result_AT ="";
       result_AT = sendATcommand("AT#SD=1,0,10510,modules.telit.com",5000); 
       while(result_AT.indexOf("CONNECT")== -1)
       {
         M2M_INFO("Not Connected");
          delay(1000);
       }
       if(result_AT.indexOf("CONNECT")> 0)
       {
          sendATcommand("Hello from Bravo Board !! ",8000);
        
          result_Data = "";
          result_Data = send_data("+++",5000);
          
          if(result_Data.indexOf("OK")>0)
          {
            M2M_INFO("Closing Socket...");
            sendATcommand("AT#SH=1",10000);
            M2M_INFO("Closing Context...");
            sendATcommand("AT#SGACT=1,0",5000);
            M2M_INFO("Application exit...");
            exit;
          }
       }
       else
       {
         M2M_INFO("SGACT ERROR...Application exit");
       }
    }
    else
    {
      M2M_INFO("CONTEXT NOT OPENED");
    }
 
}

void network_connection() 
{
    M2M_INFO("NETWORK CONNECTION CALLED");
    int reg_status = 0;
    int counter = 0;

    result_AT ="";
    result_AT = sendATcommand("AT+CPIN?",1000);  

   M2M_INFO("result_AT: ");
   M2M_INFO(result_AT);
    if(result_AT.indexOf("READY")== -1 or result_AT.indexOf("ERROR")== -1)
    {
    result_AT ="";
    result_AT = sendATcommand("AT+CPIN?",1000);  
    delay(1000);
    }
   
    if(result_AT.indexOf("ERROR")>0)
    { 
      M2M_INFO("SIM NOT INSERTED - PLEASE VERIFY ");
    }
    
    if(result_AT.indexOf("READY")>0)
    {
      result_AT = "";
      result_AT = sendATcommand("AT+CREG?",1000);
       if((result_AT.indexOf("1")>0) or (result_AT.indexOf("5")>0))
       {
            reg_status = 1;
       }  
       else
       {
          counter = 0;
          while(reg_status != 1 or reg_status !=5)
          {   
            M2M_INFO("Module is not registered...retrying"); 
            delay(2000);  
            counter = counter +1;
            if(counter==10)
            {
                M2M_INFO("Network error, application end");
                return;
            }
          }
       }
      M2M_INFO(" Module registered: "+reg_status);
      
      result_AT = "";
      M2M_INFO("Setting Context with APN: "+APN);
      String cmd_to_send = "";
      cmd_to_send.concat("AT+CGDCONT=1,\"IPV4V6\",\"");
      cmd_to_send.concat(APN);
      cmd_to_send.concat("\"");
      result_AT = sendATcommand(cmd_to_send.c_str(),1000);
      delay(2000); 

      //Uncomment one of the following examples
      //echo_demo(); 
        result_AT ="";
      result_AT = sendATcommand("AT#M2MLIST=/XML/",1000); 
      if(result_AT.indexOf("object_26251.xml") == -1)
      {
        M2M_INFO("File not found, please upload file in the module");  
        M2M_INFO("Application exit");
        exit(0);
      }
      else
      {
       lwm2m_demo();
      }
  
    }
}

String send_data(const char *toSend, unsigned long milliseconds) 
{
String result;
  Arduino_Serial.print("Sending: ");
  Arduino_Serial.println(toSend);
  Modem_Serial.write(toSend);
  
  unsigned long startTime = millis();
  Arduino_Serial.print("Received: <");
  while (millis() - startTime < milliseconds) {
    if (Modem_Serial.available()) {
      char c = Modem_Serial.read();
      Arduino_Serial.write(c);
      result += c;  // append to the result string
    }
  }
Arduino_Serial.println(">");  // new line after timeout.
return result;
  
}


String sendATcommand(const char *toSend, unsigned long milliseconds) {
String result;
  Arduino_Serial.print("Sending: ");
  Arduino_Serial.println(toSend);
  Modem_Serial.write(toSend);
  Modem_Serial.write("\r");
  unsigned long startTime = millis();
  Arduino_Serial.print("Received: <");
  while (millis() - startTime < milliseconds) {
    if (Modem_Serial.available()) {
      char c = Modem_Serial.read();
      Arduino_Serial.write(c);
      result += c;  // append to the result string
    }
  }
Arduino_Serial.println(">");  // new line after timeout.
return result;
}
