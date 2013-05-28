
class Statistic
{
    public:
    Statistic( char *ID, int eepromLocation, int defaultPin );
    Statistic( char *ID, int lowAlarm, int highAlarm, int eepromLocation, int defaultPin );

    void Add( int value, int hour );
    void ResetAllTimeValues();
    void ResetDailyValues();
    void RestorePinSetting();
    void StorePinSetting();
    void SetPin( int pin );
    char *AsXML( char *temp );
    char *XMLStatus( char *temp );
    char *TRStatus( char *temp, char *hint );
    bool LowAlarm();
    bool HighAlarm();
    void Sample();

    char *_ID;
    int _lowAlarm;
    int _highAlarm;
    int _lastValue;
    int _dailyLow;
    int _dailyHigh;
    int _allTimeLow;
    int _allTimeHigh;
    int    _lastObservationHour;
    bool   _alarm;
    int    _eepromLocation;
    int    _pin;
    int    _samplesPerRead;

    private:
    bool _alarmsSet;
    bool _anyDailyValues;
    bool _anyAllTimeValues;
    void Initialize();
};

