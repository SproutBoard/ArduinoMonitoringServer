
#ifndef STRINGCONVERTER_H
#define STRINGCONVERTER_H
#include <Time.h>

class StringConverter
{
    public:
    static char *AppendIntToString( char *target, int value );
    static char *AppendTimeToString( char *target, time_t value, int format );
    static char *AppendZeroPaddedIntToString( char *target, int width, int value );
    static char *AppendXMLAttributeToString( char *target, char *name, int value );
    static char *AppendXMLTimeAttributeToString( char *target, char *name, time_t value, int format );
    static unsigned char *ConvertStringToIPAddress( unsigned char *ip, char *string );
};

#endif
