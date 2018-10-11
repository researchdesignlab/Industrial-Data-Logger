# Industrial-Data-Logger
Smart Factory IIoT 4.0

RDL data logger is a user programmable comprehensive real time industrial automation platform that offers complete flexibility to users build custom solutions to specific requirements. 

# Repository contents
• /Documents - Data Logger user manual, configuration user manual.                                                                       
• /Source Code - Sample code, which can be modified as per user requirements                                                             
• /Configuration Software - .exe file

# Source code contains
• Configuring the Hardware using Data Logger GUI                                                                                         
• Log information for the devices connected to MODBUS, Analog and Digital Input pins                                                     
• SD_Store(String values, String index, char FL) - log information in SD Card                                                           
• Read_FRAM(uint16_t ch) - Read data from FRAM                                                                                           
• Write_FRAM(uint8_t values[],uint16_t framAddr) - Store data in FRAM                                                                   
• System_Log(String data45) - Store system log in SD Card                                                                               
• Print_Directory(File dir, int numTabs) - List log files                                                                               
• Upload_FTP(byte val) -  Upload log files to FTP server                                                                                 
• Upload_JSON(String values) -  Upload log files to cloud using JSON                                                                   
• Upload_MQTT(String values) -  Upload log files to cloud using MQTT                                                                     

# Programming IDE
• Arduino IDE - This source code is written in Arduino IDE
• LabVIEW 
• AtmelStudio - User can write code as per his requirement in AtmelStudio.
		To get started with AtmelStudio look into https://github.com/researchdesignlab/ATMEL-STUDIO  
# Product Versions
• Industrial Data Logger - Version 1.0.

# Version History
• v10 GitHub version 1.0. Current version

# License Information
• This code is open source
• Redistribution and use in source and binary forms, with or without modification, are permitted

If you have any questions or queries, please contact support@researchdesignlab.com.

For similar products, please visit www.rdltech.in
