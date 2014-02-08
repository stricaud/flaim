/**********************************************************
 *
 * libpacct_structs.cpp
 * file containing encoding/decoding for the FLAIM
 * libpacct parser
 * 
 * drafted by Xiaolin Luo
 * last modified May 30 16:12:34 CDT 2007
 *
 *********************************************************/

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include "libpacct_structs.h"


/* comp_t is a 16-bit "floating" point number with a 3-bit base 8
  exponent and a 13-bit fraction. */

//encode comp_t
comp_t encode_comp_t(unsigned long value)
{
    int exp = 0, rnd = 0;

     while (value > MAXFRACT) {
             rnd = value & (1 << (EXPSIZE - 1));     /* Round up */
             value >>= EXPSIZE;      /* Base 8 exponent == 3 bit shift. */                 
             exp++;
     }
         
     // If we need to round up, do it (and handle overflow correctly).        
     if (rnd && (++value > MAXFRACT)) {
            value >>= EXPSIZE;
            exp++;
     }
          
     // Clean it up and polish it off.         
     exp <<= MANTSIZE;            /* Shift the exponent into place */
     exp += value;                /* and add on the mantissa. */

     return (comp_t)exp;
}


//decode comp_t to unsigned long
unsigned long decode_comp_t(comp_t ct)
{
   int e;
   unsigned long f;
  
   e = (ct >> MANTSIZE) & 07;  // 3 bit base 8 exponent
   f =  ct & 017777;  //13 bit mantissa

   //printf("e = %d, f = %d \n", e, f);
  
   while ( e>0 ){
     f <<= EXPSIZE;
     e -= 1;
   }

   //printf("f = %d \n", f);

  return f;
}



/* comp2_t is 24-bit with 5-bit base 2 exponent and 20 bit fraction
   (leading 1 not stored) */

//encode comp2_t
comp2_t encode_comp2_t(unsigned long value)
{
     int exp, rnd;

     exp = (value > (MAXFRACT2>>1));
        rnd = 0;

      while (value > MAXFRACT2) {
            rnd = value & 1;
            value >>= 1;
            exp++;
       }

      // If we need to round up, do it and handle overflow correctly.         
      if (rnd && (++value > MAXFRACT2)) {
             value >>= 1;
             exp++;
      }

      if (exp > MAXEXP2) { 
            // Overflow. Return largest representable number instead. 
             return (1ul << (MANTSIZE2+EXPSIZE2-1)) - 1;
      } 
      else {
             return (value & (MAXFRACT2>>1)) | (exp << (MANTSIZE2-1));
      }
}


//decode comp2_t to unsigned long
unsigned long decode_comp2_t(comp2_t value)
{
   unsigned long f, e, shift;
      
   f = value & (MAXFRACT2 >> 1);
   e = value >> (MANTSIZE2 - 1);
   shift = 1;

  // printf("e = %d, f = %d \n", e, f);

   if (e != 0) {
     f |= 1 << (MANTSIZE2 - 1);
     shift <<= (e - 1);
   }

  // printf("f = %ld \n", (unsigned long) f * shift);

   return (unsigned long) f * shift;
}



//encode float
float encode_float(unsigned long value)
{
   unsigned long e = 0;
   unsigned long f = 0;
   unsigned long b = 1<<MANTSIZE3;
   
   while (value > 0){
      f =  value & 0x7fffffu;
      if (value >= b){
        value -=b ;
        e++;
      }
      else
        break;
   }

  //printf("e = %d, f= %d\n", e, f);

   return (float) (f | (e << MANTSIZE3));
}


//decode float to unsigned long
unsigned long decode_float(float ct)
{
   unsigned int e;
   unsigned long f;
   unsigned long b = 1 << MANTSIZE3;

   e = ((u_int32_t)ct >> MANTSIZE3) & 0xffu;  // using 2^23 as the base
   f =  (u_int32_t)ct & 0x7fffffu;

   //printf("e = %d, f = %d \n", e, f);
 
   b = b*e;
   f = b + f;
 
   //printf("value = %d \n", f);

   return f;
}


//convert big-endian to little endian, floating point
float invf( float f )
{
   union
   {
     float f;
     unsigned char b[4];
   } dat1, dat2;

   dat1.f = f;
   dat2.b[0] = dat1.b[3];
   dat2.b[1] = dat1.b[2];
   dat2.b[2] = dat1.b[1];
   dat2.b[3] = dat1.b[0];
  
   return dat2.f;
}


