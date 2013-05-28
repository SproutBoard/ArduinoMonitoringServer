#include "PowerController.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <StringConverter.h>

PowerController::PowerController()
{
    _relay = 0;
    _schedule = AlwaysOff;
    _overrideState = DontOverride;
    _onTime = -1;
    _offTime = -1;
    _relayState = Unknown;
}

PowerController::PowerController( char *id, int relay, PowerControllerSchedule schedule )
{
    ID = id;
    _relay = relay;
    _schedule = schedule;
    _overrideState = DontOverride;
    _onTime = -1;
    _offTime = -1;
    _relayState = Unknown;
}

void PowerController::SetOnTime( char *value )
{
    int theTime = atoi( value );
    if ( theTime >= -1 && theTime <= 2400 )
    {
        _onTime = theTime;
        _schedule = Custom;
    }
}

void PowerController::SetOffTime( char *value )
{
    int theTime = atoi( value );
    if ( theTime >= -1 && theTime <= 2400 )
    {
        _offTime = theTime;
        _schedule = Custom;
    }
}

void PowerController::SetOverride( char *value )
{
    int theValue = atoi( value );

    switch ( theValue )
    {
        case 0:
        _overrideState = DontOverride;
        break;

        case 1:
        _overrideState = OverrideOff;
        break;

        case 2:
        _overrideState = OverrideOn;
        break;
    }
}

char *PowerController::StatusString( char *buf )
{
    //char temp[ 32 ];

    //sprintf( buf, "Relay Port: %d, Schedule: ", relay );
    //switch ( schedule )
    //{
    //    case AlwaysOn: strcat( buf, "Always On" ); break;
    //    case EighteenHour: strcat( buf, "18-Hour" ); break;
    //    case TwelveHour: strcat( buf, "12-Hour" ); break;
    //    case AlwaysOff: strcat( buf, "Always Off" ); break;

    //    case Custom: 
    //    sprintf( temp, "Custom: On at %d off at %d", _onTime, _offTime ); 
    //    strcat( buf, temp );
    //    break;
    //}

    //strcat( buf, ", Relay State: " );
    //switch ( relayState )
    //{
    //    case Unknown: strcat( buf, "Unknown" ); break;
    //    case On: strcat( buf, "On" ); break;
    //    case Off: strcat( buf, "Off" ); break;
    //}

    return buf;
}

char *PowerController::TRStatus( char *buf )
{
    strcpy( buf, "<tr><td class=\"name\">Power " );
    strcat( buf, ID );
    strcat( buf, "</td><td class=\"value\">" );
    strcat( buf, _relayState == On ? "On" : "Off" );
    strcat( buf, "</td></tr>" );

    return buf;
}

char *PowerController::XMLStatus( char *buf )
{
    strcpy( buf, "<pwr " );
    StringConverter::AppendXMLAttributeToString( buf, "port", _relay );
    StringConverter::AppendXMLAttributeToString( buf, "sch", ( int ) _schedule );
    StringConverter::AppendXMLAttributeToString( buf, "on", _onTime );
    StringConverter::AppendXMLAttributeToString( buf, "off", _offTime );
    StringConverter::AppendXMLAttributeToString( buf, "st", ( int ) _relayState );
    strcat( buf, "/>" );

    return buf;
}


// worker for GetExpectedState()
PowerControllerRelayState PowerController::GetExpectedStateFor( int hour, int minute, int onTime, int offTime )
{
    PowerControllerRelayState result = Unknown;
    int onHour;
    int onMinute;
    int offHour;
    int offMinute;

    // -1 (e.g.) for either time = always on; ontime = offtime => always on
    if ( onTime < 0 || offTime < 0 || onTime == offTime )
    {
        result = On;
    }
    else
    {
        onHour = onTime / 100;
        onMinute = onTime % 100;
        offHour = offTime / 100;
        offMinute = offTime % 100;
        
        if ( onHour == offHour && onMinute < offMinute )  // on/off within a single hour
        {
            if ( hour == onHour && minute >= onMinute && minute <= offMinute )
            {
                result = On;
            }
            else
            {
                result = Off;
            }
        }
        else if ( onHour == offHour && onMinute > offMinute ) // off/on within a single hour
        {
            if ( hour == onHour )
            {
                if ( minute >= onMinute || minute <= offMinute )
                {
                    result = On;
                }
                else
                {
                    result = Off;
                }
            }
            else
            {
                result = On;
            }
        }
        else if ( onTime < offTime ) // does not wrap around midnight
        {
            if ( hour == onHour && minute >= onMinute )
                result = On;
            else if ( hour == offHour && minute < offMinute )
                result = On;
            else if ( hour > onHour && hour < offHour )
                result = On;
            else
                result = Off;
        }
        else  // wraps around midnight
        {
            if ( hour == onHour && minute >= onMinute )
                result = On;
            else if ( hour == offHour && minute < offMinute )
                result = On;
            else if ( hour > onHour || hour < offHour )
                result = On;
            else
                result = Off;
        }
    }

    return result;
}

PowerControllerRelayState PowerController::GetExpectedState( int hour, int minute )
{
    PowerControllerRelayState result = Unknown;
    
    switch ( _overrideState )
    {
        case OverrideOff:
        result = Off;
        break;

        case OverrideOn:
        result = On;
        break;

        case DontOverride:
        switch ( _schedule )
        {
            case AlwaysOn:
            result = On;
            break;

            case EighteenHour:
            result = GetExpectedStateFor( hour, minute, LIGHT_18HOUR_ON, LIGHT_18HOUR_OFF );
            break;

            case TwelveHour:
            result = GetExpectedStateFor( hour, minute, LIGHT_12HOUR_ON, LIGHT_12HOUR_OFF );
            break;

            case AlwaysOff:
            result = Off;
            break;

            case Custom:
            result = GetExpectedStateFor( hour, minute, _onTime, _offTime );
            break;
        }
        break;
    }
    
    return result;
}

