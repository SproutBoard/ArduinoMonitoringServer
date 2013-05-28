#include "ReedSwitches.h"
#include <stdio.h>
#include <string.h>
#include <wiring.h>
#include <EEPROM.h>
#include <StringConverter.h>

#define DIGITAL_PIN_NONE 255

ReedSwitches::ReedSwitches()
{
    _switchCount = 0;
    _alert = false;
}

void ReedSwitches::RestoreFromEeprom( int address )
{
    for ( int i = 0; i < 10; i++ )
    {
        int thePin = EEPROM.read( address + i );

        if ( thePin != DIGITAL_PIN_NONE )
        {
            AddPin( thePin );
        }
    }
}

void ReedSwitches::StoreToEeprom( int address )
{
    for ( int i = 0; i < 10; i++ )
    {
        int oldValue = EEPROM.read( address + i );  // EEPROM addresses can only be written a finite number of times. Only write changes.
        int newValue = i >= _switchCount ? DIGITAL_PIN_NONE : _switchList[ i ];

        if ( newValue != oldValue )
        {
            EEPROM.write( address + i, newValue );
        }
    }
}

void ReedSwitches::ClearPins()
{
    _switchCount = 0;
}

void ReedSwitches::AddPin( int pin )
{
    _switchList[ _switchCount++ ] = pin;
    pinMode( pin, INPUT );
}

void ReedSwitches::RemovePin( int pin )
{
    for ( int i = 0; i < _switchCount; i++ )
    {
        if ( _switchList[ i ] == pin )
        {
            _switchList[ i ] = DIGITAL_PIN_NONE;
            break;
        }
    }
}

/// show the state of the reed switches
char *ReedSwitches::AsXML( char *buffer )
{
    if ( _alert )
    {
        strcpy( buffer, "<access " );
        StringConverter::AppendXMLAttributeToString( buffer, "site", ( int ) _locationID );
        StringConverter::AppendXMLTimeAttributeToString( buffer, "time", ( int ) _incidentTime, 2 );
        strcat( buffer, "/>" );
    }
    else
    {
        buffer[ 0 ] = '\0';
    }

    return buffer;
}

char *ReedSwitches::TRStatus( char *buffer )
{
    strcpy( buffer, "<tr><td class=\"name\">Reed Switches</td><td class=\"value\">" );
    for ( int i = 0; i < _switchCount; i++ )
    {
        if ( _switchList[ i ] != DIGITAL_PIN_NONE )
        {
            if ( i > 0 )
            {
                strcat( buffer, ", " );
            }
            StringConverter::AppendIntToString( buffer, _switchList[ i ] );
        }
    }

    strcat( buffer, "</td></tr>" );

    return buffer;
}

char *ReedSwitches::XMLStatus( char *buffer )
{
    strcpy( buffer, "<reeds>" );
    for ( int i = 0; i < _switchCount; i++ )
    {
        if ( _switchList[ i ] != DIGITAL_PIN_NONE )
        {
            strcat( buffer, "<port " );
            StringConverter::AppendXMLAttributeToString( buffer, "pin", ( int ) _switchList[ i ] );
            strcat( buffer, "/>" );
        }
    }
    strcat( buffer, "</reeds>" );
    return buffer;
}

bool ReedSwitches::Poll()
{
    bool result = false;
    for ( int i = 0; i < _switchCount; i++ )
    {
        if ( digitalRead( _switchList[ i ] ) == 1 )
        {
            result = true;
            _alert = true;  // a "latch" -- only set to true. Must manually set to false.
            _locationID = i;
            _incidentTime = now();
        }
    }

    return result;
}
