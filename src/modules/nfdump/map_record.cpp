/******************************************************************************
 *
 * map_record.cpp
 * Implementation file for the map-based Record class
 *
 * last modified by Greg Colombo
 * Thu Jun  8 10:09:24 CDT 2006
 *
 *****************************************************************************/

#include "value.h"
#include "map_record.h"
#include <utility>
#include <iostream>

using namespace std;

MapRecord::~MapRecord()
{
    // rmap.clear();
}

// int getField(string const & s, Value & v)
// Get the field with key s
//
// Arguments: s, the key of the field to find
// v, the Value in which the result will be stored
// Returns: 0 if the object exists and is non-NULL, 1 otherwise
// Modifies: Might actually add objects to the map because of the nature
// of operator[]
int MapRecord::getField(char* const s, Value & v)
{
	map<char* const, Value, ltstr>::iterator it;
	it = rmap.find(s);
	if (it == rmap.end())
	{
		return 1;
	}
	else
	{
		// v = (*it).second;
		v = rmap[s];
		return 0;
	}
};

// int setField(string const & s, Value const & v)
// Set the field with key s
//
// Arguments: s, the key of the field to set
// v, the Value to store at that location
// Returns: 0 on success
// Modifies: Changes the values stored in the Map
int MapRecord::setField(char* const s, Value const & v)
{
    rmap.insert(pair<char* const, Value>(s, v));
    return 0;
}

string MapRecord::printFieldName()
{
	map<char* const, Value, ltstr>::iterator it;
	for ( it=rmap.begin() ; it != rmap.end(); it++ )
		cout << "Field Name: " << (*it).first << endl;
}
