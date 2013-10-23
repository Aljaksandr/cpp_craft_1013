#pragma once
#include <basetsd.h>
#include <fstream>
using namespace std;
struct DealsElem
{
	UINT32 Type;
	UINT32 Time;
	UINT32 Len;
	char* Msg;
private:
	DealsElem(void);
	DealsElem(const DealsElem& de);
	void operator=(const DealsElem& de);

public:
	DealsElem(UINT32 type, UINT32 time, UINT32 len, char* str);
	~DealsElem(void);
	void operator<<(ofstream& out);
};

ofstream& operator<<(ofstream& out, const DealsElem de);
enum Types {
	MARKET_OPEN = 1u,
	TRADE = 2u,
	QUOTE = 3u,
	MARKET_CLOSE = 4u
};
UINT32 read_uint32( fstream& ins);
UINT32 write_uint32( ofstream& ins, UINT32 value);
char* write_str(ofstream& ins, char* str);

void write_double( ofstream& out, double rational ) ;