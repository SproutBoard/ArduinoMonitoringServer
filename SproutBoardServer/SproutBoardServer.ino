#define COMMUNITY_EDITION

#include <statistic.h>
#include <PowerController.h>
#include <StringPairList.h>
#include <ReedSwitches.h>
#include <WString.h>
#include <Time.h> 
#include <Ethernet.h>
#include <string.h>
#include "WebServer.h"
#include <UdpBytewise.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008 
#include <EEPROM.h>
#include <StringConverter.h>

#if UDP_TX_PACKET_MAX_SIZE < 64 || UDP_RX_PACKET_MAX_SIZE < 64
#error : UDP packet size to small - modify UdpBytewise.h to set buffers to 64 bytes
#endif

/*
Sproutboard Garden Room Monitor
Sproutboard.com

Analog I/O:
Analog 0 - CCD Light Sensor (CDS1)
Analog 1 - Accessory Terminal Block 1 (TA1) unused
Analog 2 - Accessory Terminal Block 2 (TA2) unused
Analog 3 - Onboard Accessories Socket 1 (Temperature sensor on external board J9)
Analog 4 - Onboard Accessories Socket 1 (Humidity sensor on external board J9)
Analog 5 - Onboard Accessories Socket 2 (Sound J10)

Digital I/O:
Digital 0 - Not used
Digital 1 - Serial Terminal (TS1)
Digital 2 - Onboard Switch (SW1)
Digital 3 - Onboard Peizo Speaker (SP1)
Digital 4 - LED 1 (LED 1)
Digital 5 - LED 2 (LED 2)
Digital 6 - Accessory Terminal Block 1 (DA1) unused (reed switch #1)
Digital 7 - Accessory Terminal Block 2 (DA2) unused (power #1)
Digital 8 - Accessory Terminal Block 3 (DA3) unused (power #2)
Digital 9 - Accessory Terminal Block 4 (DA4) unused (power #3)
Digital 10 - Reserved For Additional Shield (Ethernet shield pins correspond)
Digital 11 - Reserved For Additional Shield (Ethernet shield pins correspond)
Digital 12 - Reserved For Additional Shield (Ethernet shield pins correspond)
Digital 13 - Reserved For Additional Shield (Ethernet shield pins correspond)
*/
#define LCD_DELAY                   790   // 790 msec. With everything else, should work out to about 1.0 second

#define TEMPERATURE_LOW_ALARM         10
#define TEMPERATURE_HIGH_ALARM       300

#define HUMIDITY_LOW_ALARM           200
#define HUMIDITY_HIGH_ALARM          650

#define VERSION_STRING  "v0.1.6C 9/14/10"
time_t prevDisplay = 0; // when the digital clock was displayed
time_t systemStartTime = 0;

// const long timeZoneOffset = 32400L; // Hawai'i Time
// const long timeZoneOffset = 28800L; // Alaska Time
// const long timeZoneOffset = 25200L; // Pacific Time
const long timeZoneOffset = 21600L; // set this to the offset in seconds to your local time. This is Mountain Time.
// const long timeZoneOffset = 18000L; // Central Time
// const long timeZoneOffset = 14400L; // Eastern Time
// const long timeZoneOffset = 10800L; // Atlantic Time

// IPv4 settings. You may wish to customize. If you change the IP address through the setCmd() interface, the last
// octet of the MAC address will change with it, sort-of ensuring that it is unique on a network with multiple
// Sproutboards.
byte mac[] =        { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x2A };    // the mac address, you wont likely need to change this.
byte ip[] =         { 192, 168, 0, 42 };                        // the ip address of the monitor
byte gateway[] =    { 192, 168, 0, 1 };
byte subnet[] =     { 255, 255, 255, 0 };

WebServer webserver( "", 80 );

// NTP server IP addresses... select one. s
byte SNTP_server_IP[]   = { 192, 43, 244, 18 };     // time.nist.gov
//byte SNTP_server_IP[] = { 130, 149, 17, 21 };     // ntps1-0.cs.tu-berlin.de
//byte SNTP_server_IP[] = { 192, 53, 103, 108 };    // ptbtime1.ptb.de

/* application settings, you may not want to tinker with these.*/

#define analogPinNone  255  // avoid the temptation of using (-1). We have to read/write this to EEPROM which is bytes.
#define analogPin0       0    // the pin that the potentiometer is attached to
#define analogPin1       1    // the pin that the potentiometer is attached to
#define analogPin2       2    // the pin that the potentiometer is attached to
#define analogPin3       3    // the pin that the potentiometer is attached to
#define analogPin4       4    // the pin that the potentiometer is attached to
#define analogPin5       5    // the pin that the potentiometer is attached to

const int buttonPin = 2;     // the number of the pushbutton pin
int buttonState = 0;

const int speakerPin = 3; 
const int ledPin1 = 4;    // alarm LED
const int ledPin2 = 5;    // activity LED

ReedSwitches reedSwitches;

PowerController powerController1( "A", 7, EighteenHour );
PowerController powerController2( "B", 8, TwelveHour );
PowerController powerController3( "C", 9, AlwaysOn );

int alarmsilent = 0; // shuts up the alarm sound
int alarmstate = 0; //0 is no alarm, 1 is alarm
char *alarmMessage;

int _iteration = 0;

Statistic temperature( "temp", TEMPERATURE_LOW_ALARM, TEMPERATURE_HIGH_ALARM, 1, analogPin3 );
Statistic humidity( "humidity", HUMIDITY_LOW_ALARM, HUMIDITY_HIGH_ALARM, 2, analogPin4 );
Statistic soundLevel( "sound", 3, analogPin5 );
Statistic lightLevel( "light", 4, analogPin0 );

void ( *restart )( void ) = 0; //declare reset function @ address 0

char temp[ 100 ];    // universal temp buffer.


static const prog_uchar header[] PROGMEM = 
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\r\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\r\n"
    "<head>\r\n<title>SproutBoard Status</title>\r\n"
	"<style type=\"text/css\">\r\n"
	    "body { font-size: 10pt; font-family: sans-serif }\r\n"
	    "td { font-size: 10pt; font-family: sans-serif }\r\n"
	    "th { font-size: 10pt; font-family: sans-serif }\r\n"
            "h2 { font-size: 18pt; font-weight: bold; color: #000040; }\r\n"
            ".footer { font-size: 12pt; font-style: italic; }\r\n"
            ".name { color: #808080; background-color: #FFE0FF; }\r\n" 
            ".value { color: #004000; background-color: #E0FFE0; font-weight: bold; }\r\n"
	"</style>\r\n"
    "</head>\r\n";

static const prog_uchar body[] PROGMEM = 
    "<body>\r\n"
    "<h2>SproutBoard&trade; Status Report</h2>\r\n<center>\r\n"
    "<table cellpadding=\"3\" cellspacing=\"1\">\r\n";

static const prog_uchar final[] PROGMEM =
    "</table><br /><br /><div class=\"footer\">\r\n"
    "For SproutBoard sales, hardware and software support, visit us at <a href=\"http://www.sproutboard.com\" title=\"SproutBoard\">www.sproutboard.com</a>\r\n"
    "</div></center></body></html>\r\n";
    
char *TRNameString( char *dest, char *title )
{
    strcpy( dest, "<tr><td class=\"name\">" );
    strcat( dest, title );
    strcat( dest, "</td><td class=\"value\">" );
    
    return dest;
}

void statusCmd( WebServer &server, WebServer::ConnectionType type, char *tail, bool )
{
    server.httpSuccess( "text/xml" );

    if ( type != WebServer::HEAD )
    {
        server.printP( header );
        server.printP( body );
        
        server.print( TRNameString( temp, "Software Rev." ) );
        server.print( VERSION_STRING );
        server.println( "</td></tr>" );
        server.print( TRNameString( temp, "Startup" ) );
        temp[ 0 ] = '\0';
        StringConverter::AppendTimeToString( temp, systemStartTime, 2 );
        server.print( temp );
        server.println( "</td></tr>" );

        server.print( TRNameString( temp, "System Time" ) );
        temp[ 0 ] = '\0';
        StringConverter::AppendTimeToString( temp, now(), 2 );
        server.print( temp );
        server.println( "</td></tr>" );
        
        server.print( TRNameString( temp, "Uptime" ) );
        server.print( FormatUpTime( temp, systemStartTime, prevDisplay ) );
        server.println( "</td></tr>" );

        server.println( powerController1.TRStatus( temp ) );
        server.println( powerController2.TRStatus( temp ) );
        server.println( powerController3.TRStatus( temp ) );
        server.println( temperature.TRStatus( temp, "&deg; C" ) );
        server.println( humidity.TRStatus( temp, "%"  ) );
        server.println( lightLevel.TRStatus( temp, "%" ) );
        server.println( soundLevel.TRStatus( temp, "%" ) );
        server.println( reedSwitches.TRStatus( temp ) );
        server.print( TRNameString( temp, "HTTP Server IP Address" ) );
        for ( int i = 0; i < 4; i++ )
        {
            server.print( ( int ) ip[ i ] );
            if ( i != 3 )
            {
                server.print( "." );
            }
        }
        server.println( "</td></tr>" );

        server.print( TRNameString( temp, "HTTP Server Gateway" ) );
        for ( int i = 0; i < 4; i++ )
        {
            server.print( ( int ) gateway[ i ] );
            if ( i != 3 )
            {
                server.print( "." );
            }
        }
        server.println( "</td></tr>" );        
        server.printP( final );
    }
}

// Arduino Interface

void setup() 
{
     mac[ 5 ] = ip[ 3 ]; // hack ensures each Grow unit has its own IP address.
     
     reedSwitches.AddPin( 6 );
     // set digital pin I/O modes
     pinMode( powerController1._relay, OUTPUT);
     pinMode( powerController2._relay, OUTPUT);
     pinMode( powerController3._relay, OUTPUT);
     pinMode(ledPin1, OUTPUT);     
     pinMode(ledPin2, OUTPUT);
     pinMode(speakerPin, OUTPUT);
     pinMode(buttonPin, INPUT);
     temperature._samplesPerRead = 8;
     humidity._samplesPerRead = 8;
     BlinkLed( ledPin1 );
     BlinkLed( ledPin2 );
     
     delay(100);
     Serial.begin(9600);  
     delay(100);
     ClearLCD();
     
     Serial.print( VERSION_STRING );    
     
     delay( 2000 );  // give user time to reset the Ethernet Shield
     ClearLCD();

     for ( int i = 0; i < 4; i++ )
     {
         Serial.print( ( int ) ip[ i ] );
         if ( i != 3 )
         {
             Serial.print( "." );
         }
     }
     delay( 2000 );  // give user time to reset the Ethernet Shield

    // make the ethernet shield work
     Ethernet.begin( mac, ip, gateway, subnet );
     SetupWebserver();

     //ClearLCD();
     //Serial.print( "Getting NTP Time" );
     setSyncProvider( getNtpTime );
     while ( timeStatus() == timeNotSet )
     {
     }
     
     systemStartTime = now();
     ClearLCD();
     BlinkLed( ledPin1 );
     BlinkLed( ledPin2 );
}


void SetupWebserver()
{
    webserver.begin();
    webserver.setDefaultCommand( &statusCmd );
}

void loop()
{
    reedSwitches.Poll();
    ManageMains();
    
    // temp and humidity get eight reads each.
    temperature.Add( ReadAnalogAverage( temperature ), hour() );
    humidity.Add( ReadAnalogAverage( humidity ), hour() );
    if ( soundLevel._pin != analogPinNone )
    {
        soundLevel.Add( analogRead( soundLevel._pin ), hour() );
    }
    
    if ( lightLevel._pin != analogPinNone )
    {
        lightLevel.Add( analogRead( lightLevel._pin ), hour() );
    }
    
    ReadNTPClock();
    
    BackgroundWork( _iteration );
    RefreshLCD( ++_iteration );
}

void ReadNTPClock()
{
    if ( now() != prevDisplay ) //update the display only if the time has changed
    {
        prevDisplay = now();
    }
}

char *FormatUpTime( char *buf, time_t start, time_t current )
{
    long seconds = current - start;

    if ( seconds > 0 )
    {
        buf[ 0 ] = '\0';
        StringConverter::AppendIntToString( buf, ( int ) ( seconds / 3600L ) );
        strcat( buf, ":" );
        StringConverter::AppendZeroPaddedIntToString( buf, 2, ( int ) ( ( seconds % 3600L ) / 60L ) );
        strcat( buf, ":" );
        StringConverter::AppendZeroPaddedIntToString( buf, 2, ( int ) ( seconds % 60L ) );
    }
    else
    {
        strcpy( buf, "" );
    }

    return buf;
}

// cycles through display stuff for LCD.
void RefreshLCD( int count )
{
    ClearLCD(); 
    
    if ( strcmp( alarmMessage, "" ) )
    {
        CenterLineOne( alarmMessage );
        Serial.print( alarmMessage );
        SelectLineTwo();
    }

    switch ( count % 10 )
    {
        case 0: // show humidity, temperature  
        case 1:  
        if ( temperature._pin != analogPinNone )
        {
            Serial.print( ( int ) temperature._lastValue / 10 );
            Serial.print( "C  " );
        }
        if ( humidity._pin != analogPinNone )
        {        
            Serial.print( "RH: " );
            Serial.print( ( int ) humidity._lastValue / 10 );
            Serial.print( "%" );
        }
        break;

        case 2: // light level, sound level
        case 3:
        if ( lightLevel._pin != analogPinNone )
        {
            Serial.print( "Light " );
            Serial.print( ( int ) lightLevel._lastValue / 10 );
            Serial.print( "%" );
        }
        break;

        case 4:
        case 5: // noise
        if ( soundLevel._pin != analogPinNone )
        {
            Serial.print( "Noise " );
            Serial.print( ( int ) soundLevel._lastValue / 10 );
            Serial.print( "%" );
        }
        break;
        
        case 6:
        case 7:
        Serial.print( "PWR " );
        Serial.print( powerController1._relayState == On ? "ON " : "off " );
        Serial.print( powerController2._relayState == On ? "ON " : "off " );
        Serial.print( powerController3._relayState == On ? "ON" : "off" );
        break;
 
        case 8:
        case 9:
        FormatUpTime( temp, systemStartTime, prevDisplay );
        strcat( temp, " Up" );
        Serial.print( temp );
        break;
    } 

    if ( ! strcmp( alarmMessage, "" ) )
    {
        SelectLineTwo();
        
        temp[ 0 ] = '\0';
        StringConverter::AppendTimeToString( temp, now(), 1 );
        Serial.print( temp );
    }
    
    delay( LCD_DELAY );                  // wait for a second
}

// manages the three power mains
void ManageMains()
{
    powerController1._relayState = ManageMain( powerController1 );
    powerController2._relayState = ManageMain( powerController2 );
    powerController3._relayState = ManageMain( powerController3 );
}

PowerControllerRelayState ManageMain( PowerController powerController )
{
    PowerControllerRelayState expectedState = powerController.GetExpectedState( hour(), minute() );
    
    if ( expectedState != powerController._relayState )
    {
        digitalWrite( powerController._relay, ( expectedState == On ) ? HIGH : LOW );
    }
    
    return expectedState;
}

void BackgroundWork( int count )
{
    Alarm();
    if ( count % 4 == 0 )
    {
        BlinkLed( ledPin2 );
    }
    
    if ( reedSwitches._alert && alarmstate == 0 )
    {
        BlinkLed( ledPin1 );
    }
    
    ReadButton();

    //char buff[ 64 ];
    //int len = 64;

  /* process incoming connections one at a time forever */
    //webserver.processConnection( buff, &len );
    webserver.processConnection();
}


/**
 * computes the average of a list, striking out the high and low values as outliers.
**/
int AverageOf( int *list, int count )
{
    int accumulator = 0;
    int high = -1000;
    int low = 1000;

    for ( int i = 0; i < count; i++ )
    {
        accumulator += list[ i ];
        if ( list[ i ] < low )
        {
            low = list[ i ];
        }

        if ( list[ i ] > high )
        {
            high = list[ i ];
        }
    }

    accumulator -= ( low + high );
    return accumulator / ( count - 2 );
}

// reads the temperature. Cost is 80 msec
int ReadAnalogAverage( Statistic which )
{
    if ( which._pin != analogPinNone )
    {
      int samples[ 8 ];
  
      for ( int i = 0; i <= 7; i++ )
      { // gets 8 samples of temperature   
          samples[ i ] = analogRead( which._pin ); 
          delay( 10 );
      }
         
      //which.Add( AverageOf( samples, 8 ), hour() );
      return AverageOf( samples, 8 );
    }
}

void SelectLineOne()
{  //puts the LCD cursor at line 0 char 0.
    Serial.write( byte(0xFE) );   //command flag
    Serial.write( byte(128) );    //position
}

void CenterLineOne( char *s )
{  //puts the LCD cursor at line 0 char 0.
    Serial.write( byte(0xFE) );   //command flag
    Serial.write( byte(136 - ( strlen( s ) / 2 )) );    //position
}

void SelectLineTwo()
{  //puts the LCD cursor at line 0 char 0.
   Serial.write( byte(0xFE) );   //command flag
   Serial.write( byte(192) );    //position
}

void CenterLineTwo( char *s )
{  //puts the LCD cursor at line 0 char 0.
    Serial.write( byte(0xFE) );   //command flag
    Serial.write( byte(200 - ( strlen( s ) / 2 )) );    //position
}

void ClearLCD()
{
    Serial.write( byte(0xFE) );   //command flag
    Serial.write( byte(0x01) );   //clear command.
    Serial.write( byte(0xFE) );   //command flag
    Serial.write( byte(128) );    //position to line 0
}

// Flashes the "activity" LED. Cost=50 msec.
void BlinkLed( int thePin )
{
    digitalWrite( thePin, HIGH );   // set the LED on
    delay( 50 );                  // wait for 50ms
    digitalWrite( thePin, LOW );    // set the LED off
}

void Alarm()
{
    AlarmThreshholds();
  
    if ( alarmstate == 1 )
    { 
        digitalWrite( ledPin1, HIGH );
        if ( alarmsilent == 0 )
        {
           // Beep();
        }
    }
    else
    {
        digitalWrite( ledPin1, LOW );
    }
}

void AlarmThreshholds()
{
    alarmstate = 0;
    alarmMessage = "";
    
    // alarms in order of importance, most important goes last.
    
    if ( humidity.LowAlarm() )
    {
        alarmstate = 1;
        alarmsilent = 0;
        alarmMessage = "LO HUMID";
    }

    if ( humidity.HighAlarm() )
    {
        alarmstate = 1;
        alarmsilent = 0;
        alarmMessage = "HI HUMID";
    }
    
    if ( reedSwitches._alert )
    {
        alarmstate = 1;
        alarmsilent = 0;
        alarmMessage = "ACCESS";
    }
    
    if ( temperature.LowAlarm() )
    {
        alarmstate = 1;
        alarmsilent = 0;
        alarmMessage = "LO TEMP";
    }

    if ( temperature.HighAlarm() )
    {
        alarmstate = 1;
        alarmsilent = 0;
        alarmMessage = "HI TEMP";
    }

}

// cost of one second (1000 ms)
void Beep()
{
      for ( int i = 0; i < 1000; i++ ) 
      {  // generate tone
          digitalWrite( speakerPin, HIGH );
          delayMicroseconds( 500 );
          digitalWrite( speakerPin, LOW );
          delayMicroseconds( 500 );
      }
}

void ReadButton()
{
    if ( ( buttonState = digitalRead( buttonPin ) ) == HIGH )
    {
        alarmsilent = 1;
    } 
}
 
/*-------- NTP code ----------*/

unsigned long getNtpTime()
{
  sendNTPpacket(SNTP_server_IP);
  delay(1000);
  if ( UdpBytewise.available() ) {
    for(int i=0; i < 40; i++)
       UdpBytewise.read(); // ignore every field except the time
    const unsigned long seventy_years = 2208988800UL + timeZoneOffset;        
    return getUlong() -  seventy_years;      
  }
  return 0; // return 0 if unable to get the time
}

unsigned long sendNTPpacket(byte *address)
{
  UdpBytewise.begin(123);
  UdpBytewise.beginPacket(address, 123);
  UdpBytewise.write(B11100011);   // LI, Version, Mode
  UdpBytewise.write(0);    // Stratum
  UdpBytewise.write(6);  // Polling Interval
  UdpBytewise.write(0xEC); // Peer Clock Precision
  write_n(0, 8);    // Root Delay & Root Dispersion
  UdpBytewise.write(49); 
  UdpBytewise.write(0x4E);
  UdpBytewise.write(49);
  UdpBytewise.write(52);
  write_n(0, 32); //Reference and time stamps  
  UdpBytewise.endPacket();   
}

unsigned long getUlong()
{
    unsigned long ulong = (unsigned long)UdpBytewise.read() << 24;
    ulong |= (unsigned long)UdpBytewise.read() << 16;
    ulong |= (unsigned long)UdpBytewise.read() << 8;
    ulong |= (unsigned long)UdpBytewise.read();
    return ulong;
}

void write_n(int what, int how_many)
{
  for ( int i = 0; i < how_many; i++ )
  {
      UdpBytewise.write( what );
  }
}


