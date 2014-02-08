/*****************************************************************************
 *
 * value_types.h
 * Header file with definitions for valid contents for the "type" field of the
 * Value class
 *
 * last modified by Greg Colombo
 * Mon Jul  3 14:21:14 GMT 2006
 *
 *****************************************************************************/

#ifndef _VALUE_TYPES_H
#define _VALUE_TYPES_H

// the following definitions are used to create constants that can be plugged
// into a type in the Value class. For example, you may instantiate a Value
// as follows:
//
// int myVar = 42;
// Value v = Value(&myVar, VAL_TYPE_INT, 4);
//
// Later on you can access the value as follows:
// if(v.Type() == VAL_TYPE_INT)
//   int* iptr = (int*)(Value.Ptr());
//
// The thing to remember is that a Value encapsulates a void*, so VAL_TYPE_INT
// means that the value encapsulates a void* version of a pointer to int, not
// an integer itself

#define VAL_TYPE_NULL -1

// unsigned integer types
#define VAL_TYPE_UINT8 0x00
#define VAL_TYPE_UINT16 0x01
#define VAL_TYPE_UINT32 0x02

// signed integer types
#define VAL_TYPE_INT8 0x10
#define VAL_TYPE_INT16 0x11
#define VAL_TYPE_INT32 0x12

// string types
#define VAL_TYPE_STRING 0x20

// byte buffer types
#define VAL_TYPE_CHAR_PTR 0x30
//FIXME: Resolve differences between the header files in the modules

// floating point types
#define VAL_TYPE_DOUBLE 0x40

#endif
