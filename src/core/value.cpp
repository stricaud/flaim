//********************************************************************
//        ___           ___       ___                       ___     
//       /\  \         /\__\     /\  \          ___        /\__\    
//      /::\  \       /:/  /    /::\  \        /\  \      /::|  |   
//     /:/\:\  \     /:/  /    /:/\:\  \       \:\  \    /:|:|  |   
//    /::\~\:\  \   /:/  /    /::\~\:\  \      /::\__\  /:/|:|__|__ 
//   /:/\:\ \:\__\ /:/__/    /:/\:\ \:\__\  __/:/\/__/ /:/ |::::\__\
//   \/__\:\ \/__/ \:\  \    \/__\:\/:/  / /\/:/  /    \/__/~~/:/  /
//        \:\__\    \:\  \        \::/  /  \::/__/           /:/  / 
//         \/__/     \:\  \       /:/  /    \:\__\          /:/  /  
//                    \:\__\     /:/  /      \/__/         /:/  /   
//                     \/__/     \/__/                     \/__/    
//  
//
// Copyright © 2005-2006
// The Board of Trustees of the University of Illinois.
// All rights reserved.
// 
// Developed by:
//   LAIM Working Group
//   National Center for Supercomputing Applications
//   University of Illinois
// 
// Webpage:
//   http://flaim.ncsa.uiuc.edu/
// 
// ASCII Art generated using: http://www.network-science.de/ascii/
// 
//********************************************************************
// $Id: value.cpp 237 2006-07-27 10:31:56Z vdhar $
//
// Implementation file for the FLAIM Value class
//
//********************************************************************


#include "value.h"
#include "value_types.h"
#include <cstring>
#include <cassert>

using std::string;

// Value()
// no-argument constructor: initializes a null Value, which cannot be changed
// and is mostly useful for comparison purposes (e.g., "value not found")
//
// Arguments: None
// Returns: Nothing
// Modifies: Nothing
Value::Value() : ptr(NULL), type(VAL_TYPE_NULL), bytes(-1)
{
    // no body
}

// Value(void *ptr, int type, int bytes)
// Initialize a new Value with the given fields
//
// Arguments: ptr, an address the Value should point to
//            type, an integer constant identifying the type of the pointee
//            bytes, an integer giving the size of the pointee in bytes, useful
//                   for buffers
// Returns: Nothing
// Modifies: Nothing
Value::Value(void *ptr_i, int type_i, int bytes_i) : ptr(ptr_i),
						     type(type_i),
						     bytes(bytes_i)
{
    // no body
    // ...knows the trouble I've seen
}

/*
// ~Value()
// Smart destructor which deallocates memory stored at *ptr by inferring the
// type from the 'type' field
//
// Arguments: None
// Returns: Nothing
// Modifies: The data pointed to by ptr is freed
Value::~Value()
{
    // if-else if-else blocks are necessary here because we need to do
    // instantiations
    if(type == VAL_TYPE_UINT8)
    {
	uint8_t* dptr = (uint8_t*)ptr;
	delete dptr;
    }
    else if(type == VAL_TYPE_UINT16)
    {
	uint16_t* dptr = (uint16_t*)ptr;
	delete dptr;
    }
    else if(type == VAL_TYPE_UINT32)
    {
	uint32_t* dptr = (uint32_t*)ptr;
	delete dptr;
    }
    else if(type == VAL_TYPE_INT8)
    {
	int8_t* dptr = (int8_t*)ptr;
	delete dptr;
    }
    else if(type == VAL_TYPE_INT16)
    {
	int16_t* dptr = (int16_t*)ptr;
	delete dptr;
    }
    else if(type == VAL_TYPE_INT32)
    {
	int32_t* dptr = (int32_t*)ptr;
	delete dptr;
    }
    else if(type == VAL_TYPE_STRING)
    {
	string* dptr = (string*)ptr;
	delete dptr;
    }
    else if(type == VAL_TYPE_CHAR_PTR)
    {
	char* dptr = (char*)ptr;
	delete[] dptr;
    }
}

// Value(Value const & origVal)
// Copy constructor
//
// Arguments: origVal, the value to copy construct from
// Returns: Nothing
// Modifies: Nothing; a hard copy is made of the pointed-to data
Value::Value(Value const & origVal) : type(origVal.type), bytes(origVal.bytes)
{
	if(type == VAL_TYPE_UINT8)
	    ptr = new uint8_t(*(uint8_t*)(origVal.ptr));
	else if(type == VAL_TYPE_UINT16)
	    ptr = new uint16_t(*(uint16_t*)(origVal.ptr));
	else if(type ==  VAL_TYPE_UINT32)
	    ptr = new uint32_t(*(uint32_t*)(origVal.ptr));
	else if(type ==  VAL_TYPE_INT8)
	    ptr = new int8_t(*(int8_t*)(origVal.ptr));
	else if(type ==  VAL_TYPE_INT16)
	    ptr = new int16_t(*(int16_t*)(origVal.ptr));
	else if(type ==  VAL_TYPE_INT32)
	    ptr = new int32_t(*(int32_t*)(origVal.ptr));
	else if(type ==  VAL_TYPE_STRING)
	    ptr = new string(*(string*)(origVal.ptr));
	else if(type == VAL_TYPE_CHAR_PTR)
	{
	    ptr = new char[bytes];
	    strncpy((char*)ptr, (char*)origVal.ptr, bytes);
	}
	else
	    ptr = NULL;
}

// Value const & operator=(Value const & origVal)
// Assignment operator
//
// Arguments: origVal, the value to assign
// Returns: A const reference to the assigned object
// Modifies: The old data is freed and a hard copy of new data is made
Value const & Value::operator=(Value const & origVal)
{
    if(this != &origVal) // self-assignment
    {
	type = origVal.type;
	bytes = origVal.bytes;
	if(type == VAL_TYPE_UINT8)
	{
	    uint8_t* dptr = (uint8_t*)ptr;
	    delete dptr;
	}
	else if(type == VAL_TYPE_UINT16)
	{
	    uint16_t* dptr = (uint16_t*)ptr;
	    delete dptr;
	}
	else if(type == VAL_TYPE_UINT32)
	{
	    uint32_t* dptr = (uint32_t*)ptr;
	    delete dptr;
	}
	else if(type == VAL_TYPE_INT8)
	{
	    int8_t* dptr = (int8_t*)ptr;
	    delete dptr;
	}
	else if(type == VAL_TYPE_INT16)
	{
	    int16_t* dptr = (int16_t*)ptr;
	    delete dptr;
	}
	else if(type == VAL_TYPE_INT32)
	{
	    int32_t* dptr = (int32_t*)ptr;
	    delete dptr;
	}
	else if(type == VAL_TYPE_STRING)
	{
	    string* dptr = (string*)ptr;
	    delete dptr;
	}
	else if(type == VAL_TYPE_CHAR_PTR)
	{
	    char* dptr = (char*)ptr;
	    delete[] dptr;
	}


	if(type == VAL_TYPE_UINT8)
	    ptr = new uint8_t(*(uint8_t*)(origVal.ptr));
	else if(type == VAL_TYPE_UINT16)
	    ptr = new uint16_t(*(uint16_t*)(origVal.ptr));
	else if(type ==  VAL_TYPE_UINT32)
	    ptr = new uint32_t(*(uint32_t*)(origVal.ptr));
	else if(type ==  VAL_TYPE_INT8)
	    ptr = new int8_t(*(int8_t*)(origVal.ptr));
	else if(type ==  VAL_TYPE_INT16)
	    ptr = new int16_t(*(int16_t*)(origVal.ptr));
	else if(type ==  VAL_TYPE_INT32)
	    ptr = new int32_t(*(int32_t*)(origVal.ptr));
	else if(type ==  VAL_TYPE_STRING)
	    ptr = new string(*(string*)(origVal.ptr));
	else if(type == VAL_TYPE_CHAR_PTR)
	{
	    ptr = new char[bytes];
	    strncpy((char*)ptr, (char*)origVal.ptr, bytes);
	}
	else
	    ptr = NULL;
    }

    return *this;
}
*/

// void* Ptr() const
// Return the value contained in the ptr field
//
// Arguments: none
// Returns: the address stored in *ptr
// Modifies: nothing
void* Value::Ptr() const
{
    return ptr;
}

// int Type() const
// Return the value contained in the type field
//
// Arguments: none
// Returns: the value stored in type
// Modifies: nothing
int Value::Type() const
{
    return type;
}

// int Bytes() const
// Return the value contained in the bytes field
//
// Arguments: none
// Returns: the value stored in bytes
// Modifies: nothing
int Value::Bytes() const
{
    return bytes;
}


//type conversion utility functions
uint32_t* Value::to_uint32(Value *v)
{
  assert( v->Type() == VAL_TYPE_UINT32);
  return (uint32_t *)(v->Ptr());
}

uint16_t* Value::to_uint16(Value *v)
{
  assert( v->Type() == VAL_TYPE_UINT16);
  return (uint16_t *)(v->Ptr());
}

uint8_t* Value::to_uint8(Value *v)
{
  assert( v->Type() == VAL_TYPE_UINT8);
  return (uint8_t *)(v->Ptr());
}


int32_t* Value::to_int32(Value *v)
{
  assert( v->Type() == VAL_TYPE_INT32);
  return (int32_t *)(v->Ptr());
}

int16_t* Value::to_int16(Value *v)
{
  assert( v->Type() == VAL_TYPE_INT16);
  return (int16_t *)(v->Ptr());
}

int8_t* Value::to_int8(Value *v)
{
  assert( v->Type() == VAL_TYPE_INT8);
  return (int8_t *)(v->Ptr());
}

char*  Value::to_char_ptr(Value *v)
{
  assert( v->Type() == VAL_TYPE_CHAR_PTR);
  return (char *)(v->Ptr());
}

string*  Value::to_string(Value *v)
{
  assert( v->Type() == VAL_TYPE_STRING);
  return (string *)(v->Ptr());
}

