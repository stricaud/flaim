/*****************************************************************************
 *
 * value.h
 * Header file for a generic Value class, which encapsulates pieces of data
 * so that they can be sent to FLAIM
 *
 * last modified by Greg Colombo
 * Wed Jun  7 14:21:16 CDT 2006
 * 
 *****************************************************************************/

#ifndef _VALUE_H
#define _VALUE_H

class Value
{
    public:

	// no-argument ctor -- used to create a "null" value for comparisons
	Value();
	
	// 3-argument ctor -- initialize each field of the value
	Value(void *ptr_i, int type_i, int bytes_i);

	// the Big Three
	/*
	~Value();
	Value(Value const & origVal);
	Value const & operator=(Value const & origVal);
	*/

	// get functions
	void* Ptr() const;
	int Type() const;
	int Bytes() const;

    private:

	void *ptr; // pointer to the encapsulated data
	int type; // integer identifier of that data's type, for casting
	int bytes; // size of the data in bytes

};

#endif
