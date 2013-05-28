#include "statistic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EEPROM.h>
#include <StringConverter.h>

#define NO_ALARM (-99)
#define ANALOG_PIN_NONE 255

Statistic::Statistic( char *ID, int eepromLocation, int defaultPin )
{
    _ID = ID;
    _lowAlarm = NO_ALARM;
    _highAlarm = NO_ALARM;
    _alarmsSet = false;
    _eepromLocation = eepromLocation;
    _pin = defaultPin;
    Initialize();
}

Statistic::Statistic( char *ID, int lowAlarm, int highAlarm, int eepromLocation, int defaultPin )
{
    _ID = ID;
    _lowAlarm = lowAlarm;
    _highAlarm = highAlarm;
    _alarmsSet = true;
    _eepromLocation = eepromLocation;
    _pin = defaultPin;
    Initialize();
}


void Statistic::RestorePinSetting()
{
    int value = EEPROM.read( _eepromLocation );

    if ( value >= 0 && value <= 5 || value == ANALOG_PIN_NONE )
    {
        _pin = value;
    }
}

void Statistic::StorePinSetting()
{
    EEPROM.write( _eepromLocation, _pin );
}

void Statistic::SetPin( int pin )
{
    if ( pin != _pin )
    {
        _pin = pin;
        StorePinSetting();
    }
}

void Statistic::Initialize()
{
    _anyDailyValues = false;
    _anyAllTimeValues = false;
    _lastObservationHour = 0;
    _samplesPerRead = 1;
}

void Statistic::ResetAllTimeValues()
{
    _anyAllTimeValues = false;
    _allTimeLow = 0;
    _allTimeHigh = 0;
}

void Statistic::ResetDailyValues()
{
    _anyDailyValues = false;
    _dailyLow = 0;
    _dailyHigh = 0;
}

bool Statistic::LowAlarm()
{
    return _lowAlarm != NO_ALARM && _lastValue < _lowAlarm;
}

bool Statistic::HighAlarm()
{
    return _highAlarm != NO_ALARM && _lastValue > _highAlarm;
}

void Statistic::Add( int value, int hour )
{
    if ( _alarmsSet )
    {
        _alarm = value < _lowAlarm || value > _highAlarm;
    }

    if ( hour < _lastObservationHour || ! _anyDailyValues ) // date rollover
    {
        _dailyLow = value;
        _dailyHigh = value;
        _anyDailyValues = true;
    }

    if ( ! _anyAllTimeValues )
    {
        _allTimeLow = value;
        _allTimeHigh = value;
        _anyAllTimeValues = true;
    }

    _lastValue = value;
    _lastObservationHour = hour;
    if ( value < _dailyLow )
    {
        _dailyLow = value;
    }

    if ( value > _dailyHigh )
    {
        _dailyHigh = value;
    }

    if ( value < _allTimeLow )
    {
        _allTimeLow = value;
    }

    if ( value > _allTimeHigh )
    {
        _allTimeHigh = value;
    }
}

char *Statistic::TRStatus( char *buf, char *hint )
{
    strcpy( buf, "<tr><td class=\"name\">Sensor " );
    strcat( buf, _ID );
    strcat( buf, "</td><td class=\"value\">" );
    StringConverter::AppendIntToString( buf, _lastValue );
    strcat( buf, " (" );
    StringConverter::AppendIntToString( buf, _lastValue / 10 );
    strcat( buf, hint );
    strcat( buf, ")</td></tr>" );

    return buf;
}

char *Statistic::XMLStatus( char *temp )
{
    strcpy( temp, "<" );
    strcat( temp, _ID );
    strcat( temp, " " );
    StringConverter::AppendXMLAttributeToString( temp, "port", _pin );
    StringConverter::AppendXMLAttributeToString( temp, "alarm1", _lowAlarm );
    StringConverter::AppendXMLAttributeToString( temp, "alarm2", _highAlarm );
    strcat( temp, "/>" );
    
    return temp;
}

char *Statistic::AsXML( char *temp )
{
    char alarm[ 20 ];

    if ( LowAlarm() )
    {
        strcpy( alarm, " alarm=\"low\"" );
    }
    else if ( HighAlarm() )
    {
        strcpy( alarm, " alarm=\"high\"" );
    }
    else
    {
        alarm[ 0 ] = '\0';
    }

    strcpy( temp, "<" );
    strcat( temp, _ID );
    strcat( temp, " " );
    StringConverter::AppendXMLAttributeToString( temp, "value", _lastValue );
    strcat( temp, alarm );
    strcat( temp, "><daily " );
    StringConverter::AppendXMLAttributeToString( temp, "lo", ( int ) _dailyLow );
    StringConverter::AppendXMLAttributeToString( temp, "hi", ( int ) _dailyHigh );
    strcat( temp, "/><at " );
    StringConverter::AppendXMLAttributeToString( temp, "lo", ( int ) _allTimeLow );
    StringConverter::AppendXMLAttributeToString( temp, "hi", ( int ) _allTimeHigh );
    strcat( temp, "/></" );
    strcat( temp, _ID );
    strcat( temp, ">" );
    
    return temp;
}



