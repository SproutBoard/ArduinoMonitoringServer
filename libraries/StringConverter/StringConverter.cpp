#include "StringConverter.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

// given memory constraints, this method has no tolerance for errors.
unsigned char *StringConverter::ConvertStringToIPAddress( unsigned char *ip, char *string )
{
    int rack = 0;
    unsigned char *ipptr = ip;

    for ( ; *string; *string++ )
    {
        if ( *string >= '0' && *string <= '9' )
        {
            rack = ( 10 * rack ) + ( *string - '0' );
        }
        else
        {
            *ipptr++ = rack;
            rack = 0;
        }
    }

    *ipptr = rack;
    return ip;
}

char *StringConverter::AppendXMLAttributeToString( char *target, char *name, int value )
{
    strcat( target, name );
    strcat( target, "=\"" );
    AppendIntToString( target, value );
    strcat( target, "\" " );
}

char *StringConverter::AppendXMLTimeAttributeToString( char *target, char *name, time_t value, int format )
{
    strcat( target, name );
    strcat( target, "=\"" );
    AppendTimeToString( target, value, format );
    strcat( target, "\" " );
}

char *StringConverter::AppendTimeToString( char *target, time_t value, int format )
{
    StringConverter::AppendZeroPaddedIntToString( target, 2, hour( value ) );
    strcat( target, ":" );
    StringConverter::AppendZeroPaddedIntToString( target, 2, minute( value ) );

    if ( format == 2 ) // seconds
    {
        strcat( target, ":" );
        StringConverter::AppendZeroPaddedIntToString( target, 2, second( value ) );
    }
    else if ( format == 1 ) // flashing *
    {
        strcat( target, ( second( value ) % 2 == 0 ) ? " *" : "  " );
    }

    strcat( target, " " );
    StringConverter::AppendZeroPaddedIntToString( target, 2, month( value ) );
    strcat( target, "/" );
    StringConverter::AppendZeroPaddedIntToString( target, 2, day( value ) );
    strcat( target, "/" );
    StringConverter::AppendZeroPaddedIntToString( target, 2, year( value ) % 100 );

    return target;
}

char *StringConverter::AppendZeroPaddedIntToString( char *target, int width, int value )
{
    char temp[ 10 ];

    temp[ 0 ] = '\0';
    AppendIntToString( temp, value );
    for ( int i = strlen( temp ); i < width; i++ )
    {
        strcat( target, "0" );
    }

    strcat( target, temp );
    return target;
}


char *StringConverter::AppendIntToString( char *target, int value )
{
    int     count = 0, /* number of characters in string */
            i, /* loop control variable */
            sign; /* determine if the value is negative */

    char    *ptr, /* temporary pointer, index into string */
            temp[ 10 ], /* temporary string array */
            *ptemp = temp;

    if ((sign = value) < 0) /* assign value to sign, if negative */
    { /* keep track and invert value */
        value = -value;
        count++; /* increment count */
    }

    ptr = target + strlen( target );

    do 
    {
        *ptemp++ = value % 10 + '0'; /* obtain modulus and or with '0' */
        count++; /* increment count, track iterations*/
    } 
    while ( ( value /= 10 ) > 0 );

    if (sign < 0) /* add '-' when sign is negative */
    {
        *ptemp++ = '-';
    }

    *ptemp-- = '\0';

    for (i = 0; i < count; i++, ptemp--, ptr++)
    {
        *ptr = *ptemp;
    }

    *ptr = '\0';

    return target;
}

