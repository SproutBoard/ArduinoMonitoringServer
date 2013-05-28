#include <stdio.h>
#include <string.h>
#include "StringPairList.h"

StringPairList::StringPairList()
{
    Count = 0;
    //_source[ 0 ] = '\0';
}

StringPairList::StringPairList( char *source )
{
    Count = 0;
   // _source[ 0 ] = '\0';
    if ( strlen( source ) < SPL_MAXLENGTH )
    {
	//    strcpy( _source, source );
	    Parse( source );
    }
}

NameValuePair StringPairList::operator[] ( const int index )
{
    return _nameValuePairArray[ index ];
}

//void StringPairList::Parse( char *source )
//{
//    if ( strlen( source ) < SPL_MAXLENGTH )
//    {
//	    strcpy( _source, source );
//	    Parse();
//    }
//}

void StringPairList::Parse( char *source )
{
    int len = strlen( source );
	char *ptr = source;

    Count = 0;
	for ( int i = 0; i < len; i++ )
	{
		if ( source[ i ] == '=' )
		{
			source[ i ] = '\0';
			_nameValuePairArray[ Count ].name = ptr;
			ptr = &source[ i + 1 ];
		}
		else if ( source[ i ] == '&' )
		{
			source[ i ] = '\0';
			_nameValuePairArray[ Count++ ].value = ptr;
            if ( Count == SPL_MAXTOKENS )
            {
                break;
            }

			ptr = &source[ i + 1 ];
		}
	}

	_nameValuePairArray[ Count++ ].value = ptr;
}

//char *StringPairList::ToString( char *result )
//{
	//result[ 0 ] = '\0';
	//for ( int i = 0; i < Count; i++ )
	//{
	//	char temp[ 40 ];

	//	sprintf( temp, "%s = %s", _nameValuePairArray[ i ].name, _nameValuePairArray[ i ].value );
	//	if ( i > 0 )
	//	{
	//		strcat( result, ", " );
	//	}

	//	strcat( result, temp );
	//}

	//return result;
//}

//char *StringPairList::ValueOf( char *key )
//{
//	char *result = 0;

//	for ( int i = 0; i < Count; i++ )
//	{
//		if ( ! strcmp( _nameValuePairArray[ i ].name, key ) )
//		{
//			result = _nameValuePairArray[ i ].value;
//			break;
//		}
//	}

//	return result;
//}
