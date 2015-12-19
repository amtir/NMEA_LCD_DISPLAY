/********************************************************************/
/* Program:  This program displays the wind speed [m/s] and the wind*/
/* direction on an LCD (20x4) from the  ELBW weather station  		*/
/* (RS485 bus, NMEA0183 protocol)									*/			           
/* MCU      : P89V51RB2 (XTAL=18.432 MHz) 			    		    */
/* Compiler : Keil C51 (V8.05a)      			        		    */
/* Writen By :  MTIR Akram 14/09/2010							    */
/* Examples : Display 20x4 characters LCD 4 bit mode, playing 		*/
/* with timers, UART/RS232/RS422/RS485 8,N,4800,1   				*/
/********************************************************************/
/**************************************************/
/* Pins used to control the LCD	(software)		  */
/* P1.2        --> RS (Register Select)    		  */
/* P1.3		   --> E (Enable signal)  	   		  */
/* P1.4-P1.7   --> Data bus					 	  */
/*------------------------------------------------*/
/* Pins used to control the UART (hardware)		  */
/* P3.1 	   -->	TXD							  */
/* P3.0		   -->  RXD							  */
/**************************************************/
/*	Schematic
	
(1) The weather station	   		      (2) LCD board	
						       _________					
O-------|||||||||			  ¦			¦				
		|       |			  ¦	  		¦		¦-------------¦
O-------|  UC	|=============¦   UC	¦:::::::¦ LCD Display ¦
		|   	| NMEA RS232  ¦			¦ 		¦-------------¦
O-------|||||||||			  ¦_________¦		
							  					
		  					
Sensors   UC8051			   UC 8052			LCD Display	   					
																		*/
//------------------------------------------------------------------------------------------------
#include <t89c51rd2.h>
#include <stdio.h>
#include <string.h>
//------------------------------------------------------------------------------------------------
unsigned char temp;
unsigned char test= 0;
code unsigned char end = 0x0A;  // LF Line feed
code unsigned char deg[2]= {0xDF,0};	// ° character	, Ending the string  with a null character
unsigned char dataWind[96]; 
unsigned char dataBuff[7]="       ";
unsigned char checksum=0; 
//------------------------------------------------------------------------------------------------
void Delay(unsigned char);
void LCD_Command(unsigned char);
void LCD_String(unsigned char *);
void LCD_Template();
unsigned char check(unsigned char )	;
unsigned char hex_byte(char);
//------------------------------------------------------------------------------------------------


//----------------------------------------------------
//--- Convert a single hex character into decimal 
//----------------------------------------------------
unsigned char hex_byte(char x)
{
   unsigned char r;

   switch(x)
   {
	  case 'A':	r=10; break;
	  case 'B': r=11; break;
	  case 'C': r=12; break;
	  case 'D': r=13; break;
	  case 'E': r=14; break;
	  case 'F': r=15; break;
	  default: r = x - '0';
   }
   return(r);
}
//------------------------------------------------------------------------------------------------
//--- Check the chechsum of the message and return the status
//-----------------------------------------------------------------
unsigned char check(unsigned char n)	  
{
 unsigned char j=n+1;
 unsigned char chk=0;

 checksum = 0;

  while(dataWind[j]!='*')
  {
   	switch(dataWind[j])
	{
	//	case '$':
	//		break;
	//	case '*':
	//		continue;
		default:
		{
			if (  checksum==0  )
				checksum=dataWind[j] ;
			else 
				checksum=checksum^dataWind[j];
		}
	    break;
	}
	j++;
 }

 chk = 16 * hex_byte(dataWind[j+1])  + hex_byte(dataWind[j+2])  ;

 if (checksum == chk)
 	return(1);
 else
 	return(0);
}
  //-------------------------------------------------------------------------------------------------------------------
Init_Serial()
{
 IE=0x90; // Set EA = 1 --> Interrupt can be serviced, ES = 1  --> Serial port interrupt enable
 SCON=0x50; // Serial Port control register 
 // SM0 = 0 SM1=1 --> 8 bit UART baud rate variable working in mode 1 
 //REN = 1 --> Enable serial reception 
 PCON=0; // Serial port working in normal mode
 T2MOD=0x00; // 
 RCAP2H=0xFF; // Set Baud rate to 9600 bps
 RCAP2L=0x88;  //0x88 -->  4800 bps and 0xC4 --> Set Baud rate to 9600 bps
 T2CON=0x34; // Enable Timer2 TR2 = '1' and use overflow of Timer 2 to generate clock signal for both Tx & Rx of serial
}
//-------------------------------------------------------------------------------------------------------------------------
void Delay(unsigned char time) // Delay(1)= delay 20 ms
{
 unsigned char count;
 for(count=0;count<time;count++) 
 {
  TH0=0x87;
  TL0=0xFF;
  TR0=1;
  TF0=0;
  while (TF0==0);
  TR0=0;
 }
}
//-------------------------------------------------------------------------------------
LCD_Enable()
{
 P1_3=0; // Clear Enable
 Delay(10);
 P1_3=1; // Set Enable
}
//-------------------------------------------------------------------------------------
void LCD_Command(unsigned char Command)
{
 P1_2=0; //Set for command
 P1_3=1; //Set Enable
 P1=(P1 & 0x0F)|(Command & 0xF0);
 LCD_Enable();
 P1=(P1 & 0x0F)|((Command<<4)&0xF0);
 LCD_Enable();
}
//-----------------------------------------------------------------------------------
void LCD_Data(unsigned char Data)
{
 P1_2=1; // Set for data
 P1_3=1; // Set Enable
 P1=(P1 & 0x0F)|(Data & 0xF0);
 LCD_Enable();
 P1=(P1 & 0x0F)|((Data<<4)&0xF0);
 LCD_Enable();
}
//-----------------------------------------------------------------------------------
void LCD_INIT_TEMP()
{
 	// Printing once the template on the LCD
	LCD_Command(0x01);
	//LCD_String("ELBW WEATHER STATION");
	LCD_Command(0x80);

	LCD_String("ELBW TECHNOLOGY SA");

 	LCD_Command(0xC0);
	LCD_String("TEST AND MEASUREMENT ");

	LCD_Command(0x94);
	LCD_String("INITIALISATION");

	LCD_Command(0xD4);
	LCD_String("READING DATA...");

	Delay(250);
	Delay(250);
}
 //-----------------------------------------------------------------------------------
void LCD_Template()
{
 	// Printing once the template on the LCD
	LCD_Command(0x01);
	//LCD_String("ELBW WEATHER STATION");
	LCD_Command(0x80);

	LCD_String("WIND     m/s       ");
	LCD_Command(0x93);
	LCD_String(&deg[0]);

 	LCD_Command(0xC0);
	LCD_String("TEMP ");
	LCD_Command(0xCA);

	LCD_String(&deg[0]); 
	LCD_Command(0xCB);
	LCD_String("C");

	LCD_Command(0x94);
	LCD_String("HR ");

	LCD_Command(0x9C);
	LCD_String( "%");

	LCD_Command(0x9E);
	LCD_String( "DP ");
	LCD_Command(0xA6);
	LCD_String(&deg[0]);
	LCD_Command(0xA7);
	LCD_String("C");

	LCD_Command(0xD4);
	LCD_String("PRES ");

	LCD_Command(0xDF);
	LCD_String("mbar");
}

//------------------------------------------------------------------------------------
void LCD_String(unsigned char *String)
{
 while(*String)
  LCD_Data(*String++);
}
LCD_Init()
{
 P1_3=1; //Set Enable 
 P1_2=0; //Clear RS for command LCD
 LCD_Command(0x33); // 
 LCD_Command(0x32); //
 LCD_Command(0x28); // 2 line & 4 bit operation  
 LCD_Command(0x0C); //
 LCD_Command(0x06); // Display with cursor right shift
 LCD_Command(0x01); // CLS
}

//------------------------------------------------------------------------------------
  void receive_serial()interrupt 4
{
 	static unsigned char i = 0 ;
	static unsigned char init = 0 ; 
	static unsigned char buff = 0;
	static unsigned char NbrByte=0;

if(init == 0) 
{
	if (RI==1)		 //	receive interrupt flag is set by hardware 
	 {
	  temp=SBUF;
	  RI=0;		// Clear the receive interrupt flag
		 // checking the start delimiter "$" and the length of the message "enough to determine the protocole type" 
		if ((temp=='$' || test ==1 )&& (i >= 0&& i <=95)) //
		{ 	
			test=1;
			dataWind[i]= temp;
			++i;
		}
		else //
		{	
			REN = 0 ;  	  // disable serial recption			
			i=0;
			init = 1;
				
			LCD_Command(0xD4);
			LCD_String("IDENTIFICATION...  ");
			Delay(250);
			Delay(250);
			i=0;
			for (i=0;i<=90; i++ )	// scanning the data
				{
				  if(dataWind[i]=='$')	 // looking for a '$' character
				  {
				  	if(dataWind[i+4]=='W' && dataWind[i+5]=='V' && check(i)==1 ) // Wind speed and angle?? checksum??
						{buff = buff | 1; }
					else if(dataWind[i+4]=='T' && dataWind[i+5]=='A' && check(i)==1)  // Air temperature?? checksum??
						{buff = buff | 2;}
					else if(dataWind[i+4]=='H' && dataWind[i+5]=='U' && check(i)==1) 	// Humidity?? checksum??
						{buff = buff | 4;}
					else if (dataWind[i+4]=='M' && dataWind[i+5]=='B' && check(i)==1) 	// Barometer?? checksum??
						{buff = buff | 8;}
				  }
				}

			i=0	;
			test = 0; 
					  
		  switch(buff)
		  { 	  
		 	 case 1:		  
				LCD_Command(0xD4);
				LCD_String("NMEA  Protocol  M  ");
				NbrByte = 28;
				Delay(250);
				Delay(250);	
 				LCD_Template();
 				Delay(250);
				Delay(250);
				break;
			case 3:		  
				LCD_Command(0xD4);
				LCD_String("MNEA  Protocol  R  ");
				NbrByte = 47;
				Delay(250);
				Delay(250);	
 				LCD_Template();
 				Delay(250);
				Delay(250);
				break;
			case 11:		  
				LCD_Command(0xD4);
				LCD_String("NMEA  Protocol  N  ");
				Delay(250);
				Delay(250);	
 				LCD_Template();
 				Delay(250);
				Delay(250);
				NbrByte = 69;
				break;
			 case 15:		  
				LCD_Command(0xD4);
				LCD_String("NMEA  Protocol  O  ");
				Delay(250);
				Delay(250);	
 				LCD_Template();
 				Delay(250);
				Delay(250);
				NbrByte = 95;
				break;
		  	default : 		  
				LCD_Command(0xD4);
				LCD_String("NMEA  Protocol  ?  ");
				Delay(250);
				Delay(250);	
 				LCD_INIT_TEMP();
 				Delay(250);
				Delay(250);	
				init = 0;
				break;
		  	}
							
		}
	}					
		REN = 1 ;	  // Enable serial reception					
	    TI=0;	  // transmit interrupt flag 
}

else 
{
	if (RI==1)		 //	receive interrupt flag is set by hardware 
	 {
	  temp=SBUF;
	  RI=0;		// Clear the receive interrupt flag
		 // checking the start delimiter "$" and the length of the message NbrByte 
		if ((temp=='$' || test ==1 )&& (i >= 0&& i <=NbrByte) )
		{ 	
			test=1;
			dataWind[i]= temp;
			++i;
		}
	else //
		{
			REN = 0 ;  	  // disable serial recption
			   
		//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX	

			for (i=0;i<=(NbrByte-5); i++ )	// scanning the data
				{
				  if (dataWind[i]=='$')	 // looking for a '$' character
				  {
				  	if(dataWind[i+4]=='W' && dataWind[i+5]=='V' && check(i)==1 ) // Wind speed and angle 
					{				
		     			LCD_Command(0x85); // setting the start position wind speed
			 			// updating the wind speed
			 			dataBuff[0] = dataWind[i+15];
			 			dataBuff[1] = dataWind[i+16];
			 			dataBuff[2] = dataWind[i+17];
			 			dataBuff[3] = dataWind[i+18];	 
			 			dataBuff[4] = 0; // Ending the string  with a null character
			 			LCD_String(&dataBuff[0]);

						LCD_Command(0x8E);	// setting the start position for the wind direction
						 //updating the wind direction 
			 			dataBuff[0] = dataWind[i+7];
			 			dataBuff[1] = dataWind[i+8];
			 			dataBuff[2] = dataWind[i+9];
			 			dataBuff[3] = dataWind[i+10];	 
			 			dataBuff[4] = dataWind[i+11];
			 			dataBuff[5] = 0; // Ending the string  with a null character
			 			LCD_String(&dataBuff[0]);
					 }
					else if(dataWind[i+4]=='T' && dataWind[i+5]=='A' && check(i)==1 )  // Air temperature
					{
						LCD_Command(0xC5);	// setting the start position for the temperature
		   				 // updating the temperature
			 			dataBuff[0] = dataWind[i+7];
			 			dataBuff[1] = dataWind[i+8];
			 			dataBuff[2] = dataWind[i+9];
			 			dataBuff[3] = dataWind[i+10];	 
			 			dataBuff[4] = dataWind[i+11];
			 			dataBuff[5] = 0; // Ending the string  with a null character
			 			LCD_String(&dataBuff[0]);
					}
					else if(dataWind[i+4]=='H' && dataWind[i+5]=='U' && check(i)==1) 	// Humidity
					{
							 LCD_Command(0x97);// setting the start position for the humidity
			 			// updating the humidity
			 			dataBuff[0] = dataWind[i+7];
			 			dataBuff[1] = dataWind[i+8];
			 			dataBuff[2] = dataWind[i+9];
			 			dataBuff[3] = dataWind[i+10];	 
			 			dataBuff[4] = dataWind[i+11];
			 			dataBuff[5] = 0; // Ending the string  with a null character
			 			LCD_String(&dataBuff[0]);

					  	LCD_Command(0xA1);// setting the start position for the dew point
						// updating the dew point
						dataBuff[0] = dataWind[i+14];
						dataBuff[1] = dataWind[i+15];
						dataBuff[2] = dataWind[i+16];
						dataBuff[3] = dataWind[i+17];	 
						dataBuff[4] = dataWind[i+18];
						dataBuff[5] = 0; // Ending the string  with a null character
						LCD_String(&dataBuff[0]);

					}
					else if(dataWind[i+4]=='M' && dataWind[i+5]=='B' && check(i)==1) 	// Barometer
					{
						LCD_Command(0xD9);// setting the start position for the pressure
			 			// updating the pressure
			 			dataBuff[0] = dataWind[i+9];
			 			dataBuff[1] = dataWind[i+10];
			 			dataBuff[2] = dataWind[i+11];
			 			dataBuff[3] = dataWind[i+12];	 
			 			dataBuff[4] = dataWind[i+13];
			 			dataBuff[5] = dataWind[i+14];
			 			dataBuff[6] = 0; // Ending the string  with a null character
			 			LCD_String(&dataBuff[0]);
					}
			
				  }

				}				
	//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX		

		  	i=0	;
			test = 0; 
		  	REN = 1 ;	  // Enable serial reception
		}
				}		
	 TI=0;	  // transmit interrupt flag 	 
}	 	
 }
//------------------------------------------------------------------------
//########################################################################
void main()
{
 LCD_Init();
 LCD_INIT_TEMP();
 Init_Serial();

 while(1) ;

}
//#########################################################################