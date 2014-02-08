/******************************************************************************
 *
 * map_record.h
 * Header file which defines a Record-type object based on an STL map
 *
 * last modified by Greg Colombo
 * Thu Jun  8 10:03:12 CDT 2006
 *
 *****************************************************************************/

#ifndef _MAP_RECORD_H
#define _MAP_RECORD_H

#include <cstring>
#include <map>
#include <utility>
#include "value.h"
#include "record.h"
//#include "my_alloc.h"

using namespace std;

class ltstr
{
    public:
    bool operator()(char* const s1, char* const s2)
    {
	return strcmp(s1, s2) < 0;
    }
};

class MapRecord : public Record
{
    public:
	~MapRecord();
	int getField(char* const s, Value & v);
	int setField(char* const s, Value const & v);
    private:
	map<char* const, Value, ltstr> rmap; 
};

#endif
