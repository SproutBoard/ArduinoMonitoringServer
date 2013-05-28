
#define SPL_MAXLENGTH 72
#define SPL_MAXTOKENS 4

typedef struct NameValuePair
{
	char *name;
	char *value;
};

class StringPairList
{
	public:
	StringPairList();
	StringPairList( char *source );

	void Parse( char * source );
	//char *ToString( char *result );
	//char *ValueOf( char *key );

	int Count;
    NameValuePair operator[] ( const int index );

	private:
    NameValuePair _nameValuePairArray[ SPL_MAXTOKENS ];

	//char _source[ 1 + SPL_MAXLENGTH ];
	//void Parse();
};


