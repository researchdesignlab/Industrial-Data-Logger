/* Include files */
#include <ModbusMaster.h>
#include <Adafruit_ADS1015.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <EEPROM.h>
#include "RTClib.h"  

/*  starting address of the configuration settings stored in FRAM are defined here */
           /*Setting*/   /*start 
                          addr*/
#define    MODBUS_SETT    10     /* Modbus settings are stored from address 10 */
#define    MODBUS_DATA    20     /* Modbus data are stored in FRAM starting from address 20     */
#define    ADC_4TO20      230    /* ADC settings are stored in FRAM starting from address 230   */
#define    DIGITAL        270    /* Digital input settings are stored in FRAM starting from address 270 */
#define    HOST           300    /* HostName of FTP Server is stored in FRAM starting from address 300 */
#define    USER           330    /* UserName of FTP Server is stored in FRAM starting from address 330 */
#define    PASS           360    /* Password of FTP Server is stored in FRAM starting from address 360 */
#define    FOLDER         390    /* Folder name for pushing data through FTP is stored in FRAM starting from address 390 */
#define    APN            420    /* APN for GPRS Settingd is stored in FRAM starting from address 420 */
#define    POLL           450    /* Polling interval for Modbus, ADC and digital input are stored in FRAM starting from address 450 */
#define    URL            470    /* URL for uploading data to server through JSON is stored in FRAM starting from address 470 */
#define    PATH           500    /* Path for uploading data to server through JSON is stored in FRAM starting from address 500 */
#define    KEY            530    /* Path Extension for uploading data to server through JSON is stored in FRAM starting from address 530 */
#define    TYPE           580    /* JSON content type setting is stored in FRAM starting from address 580 */
#define    connection     600    /* Connection settings are stored in FRAM starting from address 600 */
#define    SLOG           750    /* Log settings are stored in FRAM starting from address 750 */
#define    BURN           770    /* BurnOut Detaction settings are stored in FRAM starting from address 770 */
#define    METH           780    /* Protocol used for data upload is stored in FRAM starting from address 780 */
#define    COST           800    /* Cloud HostName for data upload through MQTT is stored in FRAM starting from address 800 */
#define    CSER           850    /* Cloud UserName for data upload through MQTT is stored in FRAM starting from address 850 */
#define    CASS           900    /* Cloud Passowrd for data upload through MQTT is stored in FRAM starting from address 900 */
#define    CORT           940    /* Port Number for data upload through MQTT is stored in FRAM starting from address 940 */
#define    COPIC          950    /* Cloud Topic for data upload through MQTT is stored in FRAM starting from address 950 */
#define    SELECT1        1100   /* Channel Select settings are stored in FRAM starting from address 1100 */
#define    DLF            1170   /* DLF settings are stored in FRAM starting from address 1170 */
#define    PORT1          1160   /* Port1 settings are stored in FRAM starting from address 1160 */
#define    UPT            1180   /* UPT settings are stored in FRAM starting from address 1180 */

/* Global Variables */
ModbusMaster node;
Adafruit_ADS1115 ads;  
File myFile;
File root12;
RTC_DS1307 RTC; 

char date[] = " Apr 20 2016 ";
char time1[] = " 17:11:45 "; 
uint8_t answer = 0, x = 0, F_UP = 0;
uint16_t DATA_ch;
byte arraySize = 0;
char SMS[200];  /* Global array to hold the string */
uint8_t ch = 0, POLL_DIG;
uint16_t i = 0;
char fileName[15];
byte GSm_ready = 0,DATA_UPDATE = 0,GPRS_satus = 0;
int sensor;
uint16_t old_SEC, OLD_ATE;
uint32_t cal_value, OLD_value_ADC, OLD_value_MDS = 0, OLD_value_UPADC = 0, OLD_value_UPMDS = 0, OLD_value_UPDIG = 0;
uint32_t POLL_ADC, POLL_MDS;
char strtemp[100];
char FILE_NAME1[] = "14_0618A.CSV";
uint8_t DIGITAL1_OLD[] = {2,2,2,2,2,2,2,2,2},staus=0;
static const uint8_t CHANNEL[] = {0,2,3,1,0,A10,A9,A8,A15};
static const uint8_t SELCH[] = {0,26,22,27,23,28,24,25,29};
static const uint8_t DIG_pin[] = {0,45,44,43,42,38,41,40,37}; //pins used for digital input
String dataString = "";
String INDEX1 = "DATE, TIME,";   
uint8_t j, result,AX = 0;
uint16_t data[6];   
byte DATE, MONTH, SEC, HR, MIN, YEAR;  
uint8_t ADC1[36], FLAG_Q = 0;
uint8_t DIGITAL1[28];
uint8_t METHOD = 0,UP_ADC = 0, UP_DIG = 0, UP_MDS = 0; 
byte SEL_MDS = 0, SEL_ADC = 0, SEL_DIG = 0;

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);  /* Init default Serial at 9600 baud */
  delay(200);
  Wire.begin();  /* Init I2C */
  delay(200);
  Serial2.begin(9600);  /* Init Serial2 at 9600 baud */
  Serial1.begin(9600);  /* Init Serial1 at 9600 baud */
  delay(200);

  /* restore the baudrate settings that user had done */
  unsigned long baud1 = EEPROM.read(20);
  baud1 = baud1 * 4800;
  delay(1000);
  
  Serial3.begin(9600);  /* Init Serial3  at 9600 baud */
  Serial.println(baud1);
  
  Serial.println(EEPROM.read(21),HEX);
   
  node.begin(Serial3);  /* Init Modbus Communication */
  delay(100);

  RTC.begin();  /* Init RTC */
  delay(100);
  RTC.adjust(DateTime("Jul 21 2018","17:11:45")); 

  /* Configure D0,D45,D44,D43,D42,D38,D41,D40,D37 as INPUT_PULLUP pins */
  for(i = 1; i < 9; i++)
    pinMode(DIG_pin[i], INPUT_PULLUP);

  /* Configure D0,D26,D22,D27,D23,D28,D24,D25,D29 as OUTPUT pins */
  for(i = 1; i < 9; i++)
    pinMode(SELCH[i], OUTPUT);

  /* Reset pins D0,D26,D22,D27,D23,D28,D24,D25,D29 */
  for(i = 1; i < 9; i++)
    digitalWrite(SELCH[i], LOW);

  pinMode(39, OUTPUT);  /* Configure D39 as OUTPUT pin, used as Chip Select pin for Flash  */
  pinMode(10, OUTPUT);  /* Configure D10 as OUTPUT pin, used as Chip Select pin for Ethernet */
  pinMode(3, OUTPUT);  /* Configure D3 as OUTPUT pin, used as GSM Power Key(Softwae Controlled) */
  digitalWrite( 39, HIGH);  /* By default Flash CS should stay high */
  digitalWrite( 10, HIGH);  /* By default Ethernet CS should stay high */
  
  /* A High-to-Low pulse on D3 will switch ON GSM module */
  digitalWrite( 3, HIGH);  
  delay(2000);
  digitalWrite( 3, LOW);

  pinMode(35, INPUT_PULLUP);  /* D35 is used to monitor incoming data from serial */ 
  delay(200);

  /* Init SD Card */
  if (!SD.begin(53)) {
    Serial.println("SD failed!");
    return;
  }
  
  /* D13 is used for status LED */
  pinMode(13, OUTPUT); 
  delay(500);  

  /* Restore User Configuration Settings for ADC from FRAM */
  memset(SMS, '\0', sizeof(SMS));
  Read_FRAM(ADC_4TO20);    
  for(i = 0; i < 32; i++)
    ADC1[i] = SMS[i];  
        
  /* Restore User Configuration Settings for Digital Input Pins from FRAM */
   memset(SMS, '\0', sizeof(SMS));
   Read_FRAM(DIGITAL);
   for(i=0;i<25;i++)
     DIGITAL1[i]=SMS[i];

  /* Restore User Configuration Settings for ADC,Modbus and Digital input Polling interval */
  char *token;
  memset(SMS, '\0', sizeof(SMS));
  Read_FRAM(POLL);
  token = strtok(SMS, ",");  
  POLL_MDS= atoi(token);
  token = strtok(NULL, ",");  
  POLL_ADC = atoi(token);  
  token = strtok(NULL, ","); 
  POLL_DIG = atoi(token);    

  /* Restore User Configuration Settings for ADC,Modbus and Digital input Upload interval */
  memset(SMS, '\0', sizeof(SMS));
  Read_FRAM(UPT);
  token = strtok(SMS, ",");  
  UP_MDS= atoi(token);
  token = strtok(NULL, ",");  
  UP_ADC = atoi(token);  
  token = strtok(NULL, ","); 
  UP_DIG = atoi(token);      

  /* Restore User Configuration Settings for ADC,Modbus and Digital input Channel Selection for upload */
  memset(SMS, '\0', sizeof(SMS));
  Read_FRAM(SELECT1);
  SEL_MDS= SMS[0]; 
  SEL_ADC =  SMS[1];    
  SEL_DIG =  SMS[2];

  /* Restore User Configuration Settings for BurnOut Detection */
  memset(SMS, '\0', sizeof(SMS));
  Read_FRAM(BURN);

  /* Till D35 becomes high keep receiving incoming data from serial port */
  while(digitalRead(35) == LOW)
    seriaL_REC(); 

  Serial.println("START...");

  /* Restore User Configuration Settings for method to upload data on server */
  memset(SMS, '\0', sizeof(SMS));
  Read_FRAM(METH);
  METHOD = SMS[0];  
  METHOD = 1;
  FLAG_Q = 0;

  /* Power on GSM module */
  power_on();
}

void loop() {

  if(FLAG_Q == 0)
  {
/*************** RTC *********************/
    /* Get the current Time and store it into a DateTime object  */
    DateTime now = RTC.now();  
    DATE = now.day();
    MONTH = now.month();
    YEAR = now.year() - 2000;
    HR = now.hour();
    MIN = now.minute();
    SEC = now.second();

    /* Check whether polling interval (for ADC, Modbus and Digital Input) is elapsed */
    if(SEC != old_SEC)   
      old_SEC = SEC;

    if(DATE != OLD_ATE)
    {
      cal_value = 0; 
      OLD_value_ADC = 0;  
      OLD_value_MDS = 0;
      OLD_ATE = DATE;
    }
    
    cal_value = SEC;
    cal_value = (((HR * 60) + MIN) * 60) + cal_value;
    
/*********************ADC*************************/
    /* If polling interval is elapsed and the data logging is enabled, prepare datastring for logging */
    if(((cal_value - OLD_value_ADC) >= POLL_ADC) && (SEL_ADC == 2))
    {
      i = 0;
      AX = 0;
      INDEX1 = "DTAE ,TIME ,";
      dataString = "";
      OLD_value_ADC = cal_value;
      
      while(ADC1[i+3] != 200)
      {   
        i = 0;
        i = i + AX;
        Serial1.print("AI_");
        Serial1.print(i/4);
        Serial1.write(':');
        
        if(ADC1[i+1] == 2)
          Serial1.write('1');
        else
          Serial1.write('0');
        
        Serial1.println();

        /* if the channel is configured for voltage by user, read voltage on ADC pins */
        if((ADC1[i+1] == 2) && (ADC1[i+2] == 1))
        {    
          INDEX1 += "ADC- ";
          INDEX1 += String(ADC1[i]);
          INDEX1 += ",";

          /* Configure pins for reading voltage */
          digitalWrite(SELCH[i], LOW);

          /* Read voltage from 16-bit ADC channels 0-4 of ADS115 */
          if(ADC1[i] < 4)
          {
            sensor = ads.readADC_SingleEnded(CHANNEL[ADC1[i]]);
            dataString += String(sensor); 
          }
          else /* Read voltage from 10-bit ADC channels 4-8 of ATMeGA */
          {
            sensor = analogRead(CHANNEL[ADC1[i]]);
            dataString += String(sensor);  
          }
          dataString += ",";
        }
        
        /* if the channel is configured for 4-20mA current by user, read current on ADC pins */        
        if((ADC1[i+1] == 2) && (ADC1[i+2] == 2))
        {
          INDEX1 += "20MA- ";
          INDEX1 += String(ADC1[i]);
          INDEX1 += ",";
        
          /* Configure pins for reading current */
          digitalWrite( SELCH[i], HIGH);

          /* Read voltage from 16-bit ADC channels 0-4 of ADS115 */
          if(ADC1[i] < 4)
          {
            sensor = ads.readADC_SingleEnded(CHANNEL[ADC1[i]]);
            dataString += String(sensor);  
          }
          else /* Read voltage from 10-bit ADC channels 4-8 of ATMeGA */
          {
            sensor = analogRead(CHANNEL[ADC1[i]]);
            dataString += String(sensor);    
          }   
          dataString += ",";
        
        }
        
        AX = AX+4;
        /* Maximum slaves are 32 */
        if(i > 32)
          break;
      }
      /* Store logged data in SD Card */
      SD_Store(dataString, INDEX1, 1);
    }
    
    /**********************DIGITAL*********************/
    /* If logging is enabled for digital input, prepare datasring */
    if(( POLL_DIG == 2) && (SEL_DIG == 2))
    {
      i = 0;
      AX = 0;

      while(i<24)
      {
        dataString = " ";        
        INDEX1 = "DATE, TIME, INPUT ,STATUS";
        
        if(DIGITAL1[i+1] == 2)
        {      
          /* Write file header */
          dataString += "IN-";
          dataString += DIGITAL1[i];
          dataString += ",";
          sensor = digitalRead(DIG_pin[DIGITAL1[i]]);
  
          if(sensor == LOW)
          {
            dataString += "HIGH";
            staus = 1;
          }
          else
          {
            dataString += "LOW";
            staus = 0;
          }
        
          dataString += ",";

          if(DIGITAL1_OLD[DIGITAL1[i]] != staus)
          {
            Serial1.print("DI_");
            Serial1.print(DIGITAL1[i]);
            Serial1.write(':');
            Serial1.println(staus);
            DIGITAL1_OLD[DIGITAL1[i]] = staus;
            SD_Store(dataString, INDEX1, 2);
          }
        }
        i = i+3;
      }
    }
/***********************MODBUS********************/
    if(((cal_value - OLD_value_MDS) >= POLL_MDS) && (SEL_MDS == 2))
    {
       memset(SMS, '\0', sizeof(SMS));
       Read_FRAM(MODBUS_DATA);
      
       i = 0;
       AX = 0;
       byte flag = 0, mdobus = 0;

       OLD_value_MDS=cal_value;
       while((SMS[i+5]!=200) && ((SMS[i]!=13) && (SMS[i+1]!=10)) && (flag!=1))
       {
         i = 0; 
         i = i + AX;

        /* As per user configuration read the respective function codes */
        if((SMS[i+2] == 1) && (SMS[i+4] == 2))
          result = node.readCoils(SMS[i+0], SMS[i+1], SMS[i+3]);  /* Slave_ID,START_ADDR,ADD_LENGHT */
        else if((SMS[i+2] == 2) && (SMS[i+4] == 2))
          result = node.readDiscreteInputs(SMS[i+0], SMS[i+1], SMS[i+3]);  /* Slave_ID,START_ADDR,ADD_LENGHT */
        else if((SMS[i+2]==4)&&(SMS[i+4]==2)) 
          result = node.readInputRegisters(SMS[i+0],SMS[i+1],SMS[i+3]);  /* Slave_ID,START_ADDR,ADD_LENGHT */
        else if((SMS[i+2]==3)&&(SMS[i+4]==2))
          result = node.readHoldingRegisters(SMS[i+0],SMS[i+1],SMS[i+3]);  /* Slave_ID,START_ADDR,ADD_LENGHT */
        else{}

        /* do something with data if read is successful */
        if (result == node.ku8MBSuccess)
        {
          for (int Aj = 0; Aj <SMS[i+3]; Aj++)
          {       
            data[Aj] = node.getResponseBuffer(Aj);
            mdobus = 1;
          }   
        }

        AX = AX + 6;
        if(i > 192)
          break;

        if((SMS[i] == 0) && (SMS[i+1] == 0) && (SMS[i+2] == 0) && (SMS[i+3] == 0) && (SMS[i+4] == 0))
        flag = 1;

        /* If the response received correctly then log the data */
        if(mdobus == 1)
        {
          byte flag1 = 0;    
          sprintf(fileName, "%i_%i%iM.csv", DATE, MONTH, YEAR);
          Serial.print(fileName);
          delay(10);
          
          /* check if file already exists */
          if (SD.exists(fileName))
            flag = 0;
          else
            flag1 = 1;
          delay(10);

          /* Open file in wite mode */
          myFile = SD.open(fileName, FILE_WRITE);
          
          if (myFile)
          {
            Serial.print("  storing...");

            /* if file is not created yet, create and write the file header */
            if(flag1 == 1)
            myFile.println("DATE ,TIME , Slave_Id ,Address, QTY, Data");

            /* Log the data */
            myFile.print(DATE, DEC);  
            myFile.print('/');  
            myFile.print(MONTH, DEC);  
            myFile.print('/');  
            myFile.print(YEAR, DEC); 
            myFile.print(", ");   
            myFile.print(HR, DEC);  
            myFile.print(':');  
            myFile.print(MIN, DEC);  
            myFile.print(':');  
            myFile.print(SEC, DEC);
            myFile.print(", ");      
            myFile.print(SMS[i+0],DEC);   
            myFile.print(", ");  
            myFile.print(SMS[i+1], DEC);   
            myFile.print(", "); 
            myFile.print(SMS[i+3], DEC);   
            myFile.print(", "); 
                                                                                                   
            dataString = "Slave_Id=";
            ch = SMS[i+0];
            dataString += ch; 
            dataString += ",Address=";   
            ch = SMS[i+1];
            dataString += ch; 
            dataString += ",QTY="; 
            ch = SMS[i+3];
            dataString += ch; 
            dataString += ",DATA="; 
            
            for (int Aj = 0; Aj < SMS[i+3]; Aj++)
            {
              myFile.print(data[Aj] ,HEX);      
              myFile.print(" , ");
              dataString += data[Aj]; 
              dataString += ", "; 
            }
            myFile.println("");
            Serial.println("DONE...");
            myFile.close();
          }
          mdobus = 0;
        }
        else         /* If response is incorrect, log this info */
        {
          dataString = "Slave_Id=";
          ch = SMS[i+0];
          dataString += ch; 
          dataString += ",Address=";   
          ch = SMS[i+1];
          dataString += ch; 
          dataString += ",QTY="; 
          ch = SMS[i+3];
          dataString += "  Uplaodinf failed"; 
          System_Log(dataString);   
        }
      }
    }
  }
}


/* Function to Handle Serial Data */
void seriaL_REC() 
{  
  /* Handle data untill there is */
  while (Serial.available()) 
  {
    ch = Serial.read();  /* Read the command */
    if(ch == '$')
    {
      memset(SMS, '\0', sizeof(SMS));
      char *token;
      x = 0;
      do
      {
        /* if there are data in the UART input buffer, read it and check for the answer */
        if(Serial.available() > 0)
        {    
          ch = Serial.read();             
          SMS[x] = ch;
        
          /* store bytes untill you receive delimiter (CR and LN) */
          if((SMS[x] == 10) && (SMS[x-1] == 13))
            answer = 1;
          
          x++; 
          if(x > 150)
            x = 0;
        }
      }while(answer == 0);  /* Waits for the asnwer with time out */

      /* Remove delimeter from your SMS array */
      SMS[x] = '\0';
      x--;
      SMS[x] = '\0';
      x--;
      SMS[x] = '\0';

      FLAG_Q = 1;

      /* if AT+READ is received in serial, read content from FRAM and send it to seial */
      if (strstr(SMS, "AT+READ") != NULL)
      {
        F_UP = 1;
        token = strtok(SMS, "=");       
        token = strtok(NULL, "=");
        DATA_ch = atoi(token);     
        Read_FRAM(DATA_ch); /* read from FRAM */
        F_UP = 0;
      }
      else /* it is a write command, so store the data into FRAM */
      {
        Serial.println("write");

        /* extract the command */
        token = strtok(SMS, "=");       
        token = strtok(NULL, "=");  
        Serial.println(token); 

        /* Store the data in their respective locations in FRAM */
        if (strstr(SMS, "AT+MBD") != NULL)
          Write_FRAM(token, MODBUS_DATA); 
        else if (strstr(SMS, "AT+ADC") != NULL)
          Write_FRAM(token,ADC_4TO20); 
        else if (strstr(SMS, "AT+DIG") != NULL)
          Write_FRAM(token,DIGITAL); 
        else if (strstr(SMS, "AT+HST") != NULL)
          Write_FRAM(token,HOST); 
        else if (strstr(SMS, "AT+USR") != NULL)
          Write_FRAM(token,USER);
        else if (strstr(SMS,"AT+PWR") != NULL)
          Write_FRAM(token,PASS); 
        else if (strstr(SMS, "AT+FLD") != NULL)
          Write_FRAM(token,FOLDER);  
        else if (strstr(SMS, "AT+APN") != NULL)
          Write_FRAM(token,APN); 
        else if (strstr(SMS, "AT+POL") != NULL)
          Write_FRAM(token,POLL); 
        else if (strstr(SMS, "AT+URL") != NULL)
          Write_FRAM(token,URL); 
        else if (strstr(SMS, "AT+KEY") != NULL)
          Write_FRAM(token,KEY); 
        else if (strstr(SMS, "AT+PTH") != NULL)
          Write_FRAM(token,PATH); 
        else if (strstr(SMS, "AT+TYP") != NULL)
          Write_FRAM(token,TYPE); 
        else if (strstr(SMS, "AT+CON") != NULL)
          Write_FRAM(token,connection); 
        else if (strstr(SMS, "AT+LOG") != NULL)   //SYSTEM LOG
          Write_FRAM(token,SLOG);
        else if (strstr(SMS, "AT+BURN") != NULL)
          Write_FRAM(token,BURN);
        else if (strstr(SMS, "AT+METH") != NULL)
          Write_FRAM(token,METH);
        else if (strstr(SMS, "AT+BURN") != NULL)
          Write_FRAM(token,BURN);   
        else if (strstr(SMS, "AT+CHOST") != NULL)
          Write_FRAM(token,COST); 
        else if (strstr(SMS, "AT+CUSER") != NULL)   //SYSTEM LOG
          Write_FRAM(token,CSER);
        else if (strstr(SMS, "AT+CPASS") != NULL)
          Write_FRAM(token,CASS);
        else if (strstr(SMS, "AT+CTOPIC") != NULL)
          Write_FRAM(token,COPIC);
        else if (strstr(SMS, "AT+CPORT") != NULL)
          Write_FRAM(token,CORT);
        else if (strstr(SMS, "AT+SELECT") != NULL)
          Write_FRAM(token,SELECT1);
        else if (strstr(SMS, "AT+PNO") != NULL)
          Write_FRAM(token,  PORT1);
        else if (strstr(SMS, "AT+DLF") != NULL)
          Write_FRAM(token,DLF);
        else if (strstr(SMS, "AT+FPOL") != NULL)
          Write_FRAM(token,UPT);
        else if (strstr(SMS, "AT+QUIT") != NULL)
          FLAG_Q = 0;

        /* if file transfer command sent */
        else if (strstr(SMS, "AT+FT") != NULL)    
        { 
          sprintf(FILE_NAME1, "%s", token);
          Serial.println(token);   
          Serial.print("DFDF : "); 
          Serial.println(FILE_NAME1);   
          
          myFile = SD.open(FILE_NAME1,FILE_READ);

          /* if file is not empty, read the data byte by byte and send it to serial */
          if (myFile) 
          {              
            while (myFile.available()) 
              Serial.write(myFile.read());
            
            myFile.close();
              Serial.write('+');
          }
          else /* if file is empty, just send 0 */
          {
            Serial.write('0');
          }
        } 

        /* if command is for listing the file, open directory and list file one by one */
        else if (strstr(SMS, "AT+LIST") != NULL)    
        {
          root12 = SD.open("/");
          Print_Directory(root12, 0);
          delay(2000);
          root12.rewindDirectory();
          Print_Directory(root12, 0);
          root12.close();
        }

        /* If command is for time, send the current time to serial */
        else if (strstr(SMS, "AT+TIM") != NULL)    
        {                 
          token = strtok(token, ",");
          ch = token[0];    
          strcpy(date, token);
          token = strtok(NULL, ","); 
        
          strcpy(time1, token);
          
          if(ch == 'K')
          {
            DateTime now = RTC.now();
            Serial.print(now.day(), DEC);  
            Serial.print('/');  
            Serial.print(now.month(), DEC);  
            Serial.print('/');  
            Serial.print(now.year(), DEC);  
            Serial.print(",");  
            Serial.print(now.hour(), DEC);  
            Serial.print(':');  
            Serial.print(now.minute(), DEC);  
            Serial.print(':');  
            Serial.print(now.second(), DEC);  
            Serial.println(" + "); 
          }
          else
          {
            RTC.adjust(DateTime(date,time1)); 
            Serial.println(date); 
            Serial.print(time1); 
          } 
        }  
        /* if command is for modbus settings, read from EEPROM and send the same */
        else if (strstr(SMS, "AT+MBS") != NULL)    
        {
          Serial.println("baud RATE"); 
          ch = token[0];
          EEPROM.write(20, ch); 
          ch = token[1];
          EEPROM.write(21, ch); 
          ch = token[2];
          EEPROM.write(10, ch);     
          Serial.write('1'); 
        } 
        else    
          Serial.println("NO");
      }
   
      x = 0;  
      memset(SMS, '\0', sizeof(SMS));
      answer = 0;

      while (Serial.available())
        ch = Serial.read();
    }
  }
}

/* Function to store log information in a SD card file 
 * Params : values(IN) - string to be stored in a file
 *          index(IN)  - string to be stored in a file
 *          FL(IN)     - 1 = store data in ADC file, 2 = store data in digital input file
 */          
void SD_Store(String values, String index, char FL)
{
  byte flag1 = 0;
  memset(fileName, '\0', sizeof(fileName)); 

  /* If FL = 1, store data in ADC file */
  if(FL == 1)
  sprintf(fileName, "%i_%i%iA.csv", DATE,MONTH,YEAR);

  /* If FL = 2, store data in digital input file */
  else if(FL == 2)
  sprintf(fileName, "%i_%i%iD.csv", DATE,MONTH,YEAR);
  delay(100); 

  /* check whether the filename exist for the day */
  if (SD.exists(fileName))
    flag1 = 0;
  else
    flag1 = 1;

  delay(100); 

  /* Open file to log the data */         
  File myFile = SD.open(fileName, FILE_WRITE);

  /* If file opened successfully, then start writing into it */
  if (myFile) 
  {
    if(flag1 == 1) 
      myFile.println(index);

    /* write date, time and the string of data */
    myFile.print(DATE,DEC);  
    myFile.print('/');  
    myFile.print(MONTH, DEC);  
    myFile.print('/');  
    myFile.print(YEAR, DEC); 
    myFile.print(", ");  
    myFile.print(HR, DEC);  
    myFile.print(':');  
    myFile.print(MIN, DEC);  
    myFile.print(':');  
    myFile.print(SEC, DEC);
    myFile.print(", ");
    myFile.println(values);
    myFile.close();

    /* debug message */
    Serial.println(values);
  }
}

/**********************FRAM*************************/
/* Function to read data from FRAM 
 * Params : framAddrues(IN) - 16 bit address to read the data from
 *          items(IN)  - Number of bytes to read.
 */ 
void readArray (uint16_t framAddr, byte items)
{
  /* First send slave adress */
  Wire.beginTransmission(0x50);
  /* send the 2 byte read address */  
  Wire.write(framAddr >> 8);
  Wire.write(framAddr & 0xFF);
  Wire.endTransmission();
  delay(100);
  /* send read command */
  Wire.requestFrom(0x50, (uint8_t)items);
  for ( i = 0; i < items; i++) 
  {
    SMS[i] = Wire.read();
    delay(10);
  }
}

/* Function to store data in FRAM 
 * Params : values[](IN) - Data to be written
 *          framAddr(IN) - 16 bit address to write the data to
 */ 
void Write_FRAM( uint8_t values[],uint16_t framAddr)
{
  /* Get the number of bytes to write */  
  arraySize = strlen(values);

  /* debug message */  
  Serial.println(arraySize);

  /* First send slave adress */
  Wire.beginTransmission(0x50);
  /* send the 2 byte write address */ 
  Wire.write(framAddr >> 8);
  Wire.write(framAddr & 0xFF);
  /* In first location write the size of the data */ 
  Wire.write(arraySize);  
  /* increment the fram address to start writing the data */
  framAddr = framAddr + 1;
  Wire.endTransmission();
  delay(100);
  
  Wire.beginTransmission(0x50);
  Wire.write(framAddr >> 8);
  Wire.write(framAddr & 0xFF);

  /* start writing data */
  for (int i = 0; i < arraySize ; i++) 
  {
    Wire.write(values[i]);
    Serial.write(values[i]);
    delay(10);
  }
  Wire.endTransmission();
}

/* Function to extract data from FRAM 
 * Params : ch(IN) - 16 bit address to read the data from
 */ 
char *Read_FRAM(uint16_t ch)
{ 
  arraySize = 0;
  memset(SMS, '\0', sizeof(SMS));
  /* call to fram read */
  readArray(ch, 1);
  arraySize = SMS[0];
  delay(10);  
  memset(SMS, '\0', sizeof(SMS));
  readArray(ch + 1, arraySize);
  if(F_UP == 1)
  Serial.println(SMS); 
  return SMS;
}

/* Function to store system log in SD Card
 * Params : data45(IN) - string of data
 */
void System_Log(String data45)
{
  Serial1.println("Log:1");
  delay(200);
  Serial1.println("Log:0");
  
  /* open system log file */
  File myFile = SD.open("SYSTEM.CSV", FILE_WRITE);
  /* if file opened successfully, then write into the file */
  if (myFile) 
  {
    myFile.print(DATE,DEC);  
    myFile.print('/');  
    myFile.print(MONTH, DEC);  
    myFile.print('/');  
    myFile.print(YEAR, DEC); 
    myFile.print(", ");  
    myFile.print(HR, DEC);  
    myFile.print(':');  
    myFile.print(MIN, DEC);  
    myFile.print(':');  
    myFile.print(SEC, DEC);
    myFile.print(", ");
    myFile.print(data45);
    myFile.print(", ");
    myFile.println("FAILD");
    
    myFile.close();
  }
}

/* Function to print file names in SD Card directory
 * Params : dir(IN) - directory
 *          numTabs - directory depth
 */
void Print_Directory(File dir, int numTabs) 
{
  while (true) 
  {
    File entry =  dir.openNextFile();
    /* If no more files, dont look further */
    if (! entry) {
      break;
    }
    
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }

    /* print the directory name */
    Serial.print(entry.name());

    /* If the item is a directory, traverse in and print all filenames with a '/' inbetween */
    if (entry.isDirectory()) {
      Serial.println("/");
      Print_Directory(entry, numTabs + 1);
    } 
    else 
    {
      // files have sizes, directories do not
      Serial.println();
    }
    entry.close();
  }
}

/*********************GPRS**********************/
/* Function to send AT Command to GPRS
 * Params : ATcommand(IN) - AT command to be send
 *          expected_answer[IN] - expected response (normally "OK")
 *          timeout - how long to wait for the response
 */
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout)
{
  uint8_t x = 0, answer = 0; 
  unsigned long previous;

  memset(SMS, '\0', sizeof(SMS));    // Initialice the string  
  delay(100);
    
  while( Serial2.available() > 0) 
    Serial2.read();    // Dummy read to Clean the input buffer

  x = 0;
  /* get the current time for timeout management */
  previous = millis();

  /* send AT command */
  Serial2.println(ATcommand);     
  
   do
   {
      if(Serial2.available())
      {    
        SMS[x] = Serial2.read();
        Serial.write(SMS[x]);
        x++;
        // check if the desired answer is in the response of the module
        if (strstr(SMS, expected_answer) != NULL)    
          answer = 1;

        if(x > 150)
        {
          memset(SMS, '\0', sizeof(SMS));
          x=0;
        }
      }
    // Waits for the asnwer with time out
    }while((answer == 0) && ((millis() - previous) < timeout));  
      
  while( Serial2.available() > 0) 
    Serial2.read();   
    
  return answer;
}

/* Function to power on GSM
 * Params : None
 */
void power_on()
{
  uint8_t answer = 0, Ans_count = 0;
  
  delay(200);
  
  answer = 0;
  GSm_ready = 0;
  Ans_count = 0;
  
  Serial1.println("GPRS:0");
  while(answer == 0)
  {
    /* High to Low pulse on digital pin 3 switches ON the GSM */
    digitalWrite(3, HIGH); 
    delay(1000);
    digitalWrite(3, LOW); 
    
    Ans_count++;

    /* If SIM is ready GSM responses with "Call Ready" */
    answer = sendATcommand("", "Call Ready", 10000); 

    /* Test for AT command with response "OK" */
    answer = sendATcommand("AT", "OK", 2000);

    /* Set GSM baudrate to 9600 */
    answer = sendATcommand("AT+IPR = 9600", "OK", 2000);

    /* answer is 1 if the esponse is correct */
    if(answer == 1)
    {
      answer = sendATcommand("AT", "OK", 2000);                
      
      if(answer == 1)
      {
        delay(300);
        /* Set GSM to text mode instead of PDU mode */
        sendATcommand("AT+CMGF=1", "OK", 1000);
        delay(300);
        sendATcommand("AT+CLIP=1", "OK", 1000);
      }
    }
    /* If response is not correct, retry the command 5 times before leaving the loop */
    if(Ans_count > 5)
      break;            
  }
  delay(300);
  if(answer == 1)  
  { 
    GPRS_satus = 0;
    GPRS();
  }
  else
    GPRS_satus = 1;
}

/* Function to configure GPRS
 * Params : None
 */
void GPRS(void)
{
  GPRS_satus = 0;
  answer = sendATcommand("AT+AT+QILOCIP", "ERROR", 4000);
  
  if (answer == 1)
  { 
    sendATcommand("AT+QIMODE=0", "OK", 1000);        
    answer = sendATcommand("AT+QIFGCNT=0", "OK", 1000);        
    
    memset(strtemp, '\0',sizeof(strtemp));
    memset(SMS, '\0', sizeof(SMS));
    
    Read_FRAM(APN);
    delay(100);
    sprintf(strtemp, "AT+QICSGP=1,\"%s\"",SMS);
    answer = sendATcommand(strtemp, "OK", 8000);   // send the SMS number
    if (answer == 1)
    {
      delay(300);
      sendATcommand("AT+QIREGAPP", "OK", 1000);
      delay(300);
      sendATcommand("AT+QIACT", "OK", 1000);                
      GPRS_satus = 0;
      Serial1.println("GPRS:1");
    }
    else
    {
      GPRS_satus=1;
      Read_FRAM(SLOG);
      if(SMS[2]==2)
      System_Log("GPRS");
      Serial1.println("GPRS:0");
    }
  }                        
}

/* Function to Upload files on FTP server
 * Params : val[IN] - val = 1 or 2 or 3
 */
void Upload_FTP(byte val)
{
  unsigned char c;
  unsigned int filelength;
  
  DATA_UPDATE = 0; 
  if(GPRS_satus != 0)
  power_on();
  /*********************************************/
  if(GPRS_satus==0)
  {
    answer=0;
    
    memset(SMS, '\0', sizeof(SMS));
    memset(strtemp, '\0', sizeof(strtemp));
    
    Read_FRAM(HOST);
    Serial.println(arraySize);
    for(int i = 0; i < arraySize; i++)
    strtemp[i] = SMS[i];
    answer = sendATcommand("AT+QFTPOPEN?",strtemp, 4000);    // send the SMS number

    if (answer == 0)
    { 
      /* Send command for username */
      memset(strtemp, '\0',sizeof(strtemp));
      memset(SMS, '\0', sizeof(SMS));     
      Read_FRAM(USER);
      delay(100); 
      sprintf(strtemp, "AT+QFTPUSER=\"%s\"", SMS);      
      answer = sendATcommand(strtemp,"OK",3000);
      delay(300);
      memset(strtemp, '\0',sizeof(strtemp));

      /* Send command for password */
      memset(SMS, '\0', sizeof(SMS));
      Read_FRAM(PASS);
      delay(100);       
      sprintf(strtemp, "AT+QFTPPASS=\"%s\"", SMS);         
      answer = sendATcommand(strtemp,"OK",3000);
      delay(300);

      /* Send command for host */
      memset(strtemp, '\0',sizeof(strtemp));
      memset(SMS, '\0', sizeof(SMS));
      Read_FRAM(HOST);
      delay(100);    
      sprintf(strtemp,"AT+QFTPOPEN=\"%s\",21",SMS );
      answer = sendATcommand(strtemp,"+QFTPOPEN:0",50000);
      delay(300);
    }
    /****************************************/
    memset(fileName, '\0' ,sizeof(fileName));
    
    if(val == 1)  /* If val=1, upload ADC log file */
      sprintf(fileName, "%i_%i%iA.csv", DATE,MONTH,YEAR);
    else  if(val == 2)  /* If val=2, upload Digital Input log file */
      sprintf(fileName, "%i_%i%iD.csv", DATE,MONTH,YEAR);
    else  if(val == 3)  /* If val=3, upload modbus log file */
    sprintf(fileName, "%i_%i%iM.csv", DATE,MONTH,YEAR);
    
    delay(100);
    myFile = SD.open(fileName, FILE_WRITE); 
    filelength = myFile.size();//, DEC);
    myFile.close(); 

    /* Send command for the local folder to pick file from */
    delay(100);
    memset(strtemp, '\0' ,sizeof(strtemp));
    memset(SMS, '\0', sizeof(SMS));
    Read_FRAM(FOLDER);
    delay(100); 
    sprintf(strtemp, "AT+QFTPPATH=\"/%s/\"", SMS);
    answer = sendATcommand(strtemp,"+QFTPPATH",50000);
    memset(strtemp, '\0' ,sizeof(strtemp));
    sprintf(strtemp, "AT+QFTPPUT=\"%s\",%i,%d", fileName,filelength,2000);
    Serial.println(strtemp);
    answer = sendATcommand(strtemp, "CONNECT", 30000);
    if(answer==1)
    {
      myFile = SD.open(fileName);
      if (myFile) 
      {
        while (myFile.available())
          Serial2.write(myFile.read());
          
        myFile.close();        // close the file:
      }
      answer = sendATcommand("", "QFTPPUT", 30000);
      if(answer == 1)
      {
        DATA_UPDATE = 0;
        Serial.println("DONE..."); 
      }
      else
        DATA_UPDATE = 1; 
    }
    else
      DATA_UPDATE = 1;

    if(DATA_UPDATE == 1)
    {
      Read_FRAM(SLOG);
      if(SMS[5] == 2)
        System_Log("FTP uploading ");
    }
    while( Serial2.available() > 0) Serial2.read();  
  }
}

/* Function to Upload log data using JSON method
 * Params : values[IN] - data to be pushed to server
 */
void Upload_JSON(String values)
{
  unsigned char c;
  unsigned int filelength;
  DATA_UPDATE = 0;

  /* If GSM is off, power it ON */
  if(GPRS_satus != 0)
    power_on();
  
  if(GPRS_satus == 0)
  {  
    String upData="{\"id\":\"";// GSMSerial.println(upData.length());          
    upData+= values;
    upData+="\"}";

    sendATcommand("ATV1", "OK", 2000);
    sendATcommand("AT+QIDNSIP=1", "OK", 2000);

    /* Send command for URL */
    memset(strtemp, '\0' ,sizeof(strtemp));
    memset(SMS, '\0', sizeof(SMS));
    Read_FRAM(URL);
    delay(100);
    sprintf(strtemp, "AT+QIOPEN=\"TCP\",\"%s\",\"80\"",SMS);

    byte answer = sendATcommand(strtemp, "CONNECT", 15000);
    if (answer == 1)
    {
      answer = 0;
      Serial.println("Connected"); 
      answer = sendATcommand("AT+QISEND", ">", 5000);    // send the SMS number
      if (answer == 1)
      {             
        memset(strtemp, '\0' ,sizeof(strtemp));
        memset(SMS, '\0', sizeof(SMS));
        Read_FRAM(KEY);
        sprintf(strtemp, "POST %s HTTP/1.1",SMS);     
        Serial2.println(strtemp);
        
        memset(strtemp, '\0' ,sizeof(strtemp));
        memset(SMS, '\0', sizeof(SMS));
        Read_FRAM(URL);
        delay(100);
        sprintf(strtemp, "Host: %s",SMS);
        Serial2.println(strtemp);
        memset(strtemp, '\0' ,sizeof(strtemp));
        memset(SMS, '\0', sizeof(SMS));
        Read_FRAM(TYPE);
        delay(100);
        sprintf(strtemp, "Content-Type: %s",SMS);
        Serial2.println(strtemp);
        Serial2.print(F("Content-Length: "));
        Serial2.println(upData.length());
        Serial2.println();                
        Serial2.println(upData);
        Serial2.println();
        Serial2.write(byte(0x1A));

        answer = sendATcommand("", "HTTP/1.1 200 OK", 15000);    // send the SMS number
        if (answer == 1)
        {                 
          Serial.println("Uploaded"); 
          while( Serial2.available() > 0) Serial2.read(); 
        }
        else
          DATA_UPDATE = 1;
      }
    }        
    if(DATA_UPDATE == 1)
    {
      Read_FRAM(SLOG);
      if(SMS[6] == 2)
      System_Log("JISON uploading ");
    }
  }
  while( Serial2.available() > 0) Serial2.read();  
}

/* Function to Upload log data using MQTT protocol
 * Params : values[IN] - data to be pushed to cloud
 */
void Upload_MQTT(String values)
{
  String MQTT;
  DATA_UPDATE = 0;
  answer = 0;

  if(GPRS_satus != 0)
    power_on();
  
  if(GPRS_satus == 0)
  {
    memset(SMS, '\0', sizeof(SMS));
    memset(strtemp, '\0', sizeof(strtemp));
    Read_FRAM(COST);
    Serial.println(arraySize);
    for(int i = 0; i < arraySize; i++)
    strtemp[i] = SMS[i];
    answer = sendATcommand("AT+QMTOPEN?",strtemp, 4000);    // send the SMS number

    if (answer == 0)
    { 
      /* Send Host for the MQTT server */
      memset(SMS, '\0', sizeof(SMS));
      Read_FRAM(COST);
      delay(100);    
      MQTT = "AT+QMTOPEN=0,\"";
      MQTT += SMS;
      MQTT += "\",";

      /* Send Port Number for the MQTT server */
      memset(SMS, '\0', sizeof(SMS));
      Read_FRAM(CORT);
      MQTT += SMS;  
      Serial2.print(MQTT);  
      answer = sendATcommand("","QMTOPEN",10000);
      if(answer == 1)
      {
        memset(SMS, '\0', sizeof(SMS));
        Read_FRAM(COST);
        delay(100);    
        MQTT = "AT+QMTCONN=0,\"";
        MQTT += SMS;
        MQTT += "\",\"";
        memset(SMS, '\0', sizeof(SMS));
        Read_FRAM(CSER);
        MQTT += SMS; 
        MQTT += "\",\"";
        memset(SMS, '\0', sizeof(SMS));
        Read_FRAM(CASS);
        MQTT += SMS; 
        MQTT += "\"";  
        Serial2.print(MQTT);            
        answer = sendATcommand("","QMTCONN",8000);
     }
     else
      DATA_UPDATE = 1;
   }

   /* Send Topic for the MQTT server */
   memset(SMS, '\0', sizeof(SMS));
   Read_FRAM(COPIC);
   delay(100);    
   MQTT = "AT+QMTPUB=0,2,1,1,\"";
   MQTT += SMS;
   MQTT += "\""; 
   Serial2.print(MQTT);            
   answer = sendATcommand("",">",8000);
   Serial2.print(values);   
    /* Send end command(0x1A) */
   Serial2.write(0X1A); 

   if(DATA_UPDATE == 1)
   {
     Read_FRAM(SLOG);
     if(SMS[6] == 2)
       System_Log("MQTT uploading ");
    }
  }
}
