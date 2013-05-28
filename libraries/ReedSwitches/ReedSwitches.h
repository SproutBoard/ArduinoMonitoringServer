#include <Time.h> 

class ReedSwitches
{
    public:
    ReedSwitches();
    bool Poll();
    void AddPin( int pin );
    void RemovePin( int pin );
    void ClearPins();
    void RestoreFromEeprom( int address );
    void StoreToEeprom( int address );
    char *AsXML( char *buffer );
    char *XMLStatus( char *buffer );
    char *TRStatus( char *buffer );

    bool _alert;
    int _locationID;
    time_t _incidentTime;

    private:
    int _switchList[ 10 ];
    int _switchCount;
};

