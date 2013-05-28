

#define LIGHT_12HOUR_ON         600   // turn on 12-hour lights at 6:00 AM
#define LIGHT_12HOUR_OFF       1800
#define LIGHT_18HOUR_ON         500
#define LIGHT_18HOUR_OFF       2300

typedef enum PowerControllerOverride { DontOverride, OverrideOff, OverrideOn };
typedef enum PowerControllerSchedule { AlwaysOn, EighteenHour, TwelveHour, AlwaysOff, Custom };
typedef enum PowerControllerRelayState { Unknown, Off, On };

class PowerController
{
    public:
    PowerController();
    PowerController( char *id, int relay, PowerControllerSchedule schedule );

    char *ID;
    int _relay;
    PowerControllerSchedule _schedule;
    PowerControllerOverride _overrideState;
    int _onTime;
    int _offTime;
    PowerControllerRelayState _relayState;

    PowerControllerRelayState GetExpectedState( int hour, int minute );
    char *StatusString( char *buf );
    char *XMLStatus( char *buf );
    char *TRStatus( char *buf );
    void SetOnTime( char *value );
    void SetOffTime( char *value );
    void SetOverride( char *value );

    private:
    PowerControllerRelayState GetExpectedStateFor( int hour, int minute, int onTime, int offTime );
};

