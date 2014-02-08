/******************************************************************************
 *
 * record.h
 * Header file for the FLAIM Record class, an abstract base class
 *
 * last modified by Greg Colombo
 * Thu Jun  8 09:58:40 CDT 2006
 *
 *****************************************************************************/

#ifndef _RECORD_H
#define _RECORD_H

#include "value.h"
#include <string>

using std::string;

class Record
{
    public:
	virtual ~Record() { };
	virtual int getField(char* const s, Value & v) = 0;
	virtual int setField(char* const s, Value const & v) = 0;
	virtual string printFieldName() = 0;
};

#endif	
