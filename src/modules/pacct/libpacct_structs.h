/**********************************************************
 *
 * libpacct_structs.h
 * Header file containing structures for the FLAIM
 * libpacct parser
 * 
 * drafted by Xiaolin Luo
 * last modified Jun 22 20:47:34 CDT 2007
 *
 *********************************************************/

#ifndef _LIBPACCT_STRUCTS_H
#define _LIBPACCT_STRUCTS_H

#include <stdint.h>
#include <sys/types.h>


//type definition

#define __u8  u_int8_t
#define __u16 u_int16_t
#define __u32 u_int32_t
#define __u64 u_int64_t

#define __s8  int8_t
#define __s16 int16_t
#define __s32 int32_t
#define __s64 int64_t

typedef u_int16_t comp_t;
typedef u_int32_t comp2_t;


//Definitions of process accounting log version
#define ACCT_V0   0
#define ACCT_V1   1
#define ACCT_V2   2
#define ACCT_V3   3

#define ACCT_RCD_SIZE   64

#define ACCT_BIG_ENDIAN	     0x80    /* accounting file is big endian */
#define ACCT_LITTLE_ENDIAN   0x00    /* accounting file is little endian */

// accounting flags
#define AFORK		0x01	/* ... executed fork, but did not exec */
#define ASU		0x02	/* ... used super-user privileges */
#define ACOMPAT		0x04	/* ... used compatibility mode (VAX only not used) */
#define ACORE		0x08	/* ... dumped core */
#define AXSIG		0x10	/* ... was killed by a signal */

#define ACCT_COMM   16


//structure for pacct v0-2
struct acct_v02
{
	char		ac_flag;		/* Flags */
	char		ac_version;		/* Always set to ACCT_VERSION */
	u_int16_t       ac_uid16;		/* LSB of Real User ID */
	u_int16_t	ac_gid16;		/* LSB of Real Group ID */
	u_int16_t	ac_tty;			/* Control Terminal */
        u_int32_t	ac_btime;		/* Process Creation Time */
	comp_t		ac_utime;		/* User Time, in clock ticks */
	comp_t		ac_stime;		/* System Time, in clock ticks */
	comp_t		ac_etime;		/* Elapsed Time, in clock ticks */
	comp_t		ac_mem;	                /* Average Memory Usage ub clicks */
	comp_t		ac_io;			/* Chars Transferred by read/write*/
	comp_t		ac_rw;			/* Blocks Read or Written */
	comp_t		ac_minflt;		/* Minor Pagefaults */
	comp_t		ac_majflt;		/* Major Pagefaults */
	comp_t		ac_swaps;		/* Number of Swaps */
	u_int16_t	ac_ahz;			/* AHZ */
        u_int32_t	ac_exitcode;		/* Exitcode */
	char		ac_comm[ACCT_COMM + 1];	/* Command Name */
        u_int8_t	ac_etime_hi;		/* Elapsed Time MSB */
	u_int16_t	ac_etime_lo;		/* Elapsed Time LSB */
	u_int32_t	ac_uid;			/* Real User ID */
	u_int32_t	ac_gid;			/* Real Group ID */
};

typedef struct acct_v02 acct_v02_t;



//structure for pacct v3
struct acct_v3
{
	char		ac_flag;		/* Flags */
	char		ac_version;		/* Always set to ACCT_VERSION */
	u_int16_t       ac_tty;			/* Control Terminal */
	u_int32_t       ac_exitcode;		/* Exitcode */
	u_int32_t	ac_uid;			/* Real User ID */
	u_int32_t	ac_gid;			/* Real Group ID */
        u_int32_t	ac_pid;			/* Process ID */
	u_int32_t	ac_ppid;		/* Parent Process ID */
	u_int32_t	ac_btime;		/* Process Creation Time */
	float	        ac_etime;		/* Elapsed Time*/
	comp_t		ac_utime;		/* User Time */
	comp_t		ac_stime;		/* System Time */
	comp_t		ac_mem;			/* Average Memory Usage */
	comp_t		ac_io;			/* Chars Transferred */
	comp_t		ac_rw;			/* Blocks Read or Written */
	comp_t		ac_minflt;		/* Minor Pagefaults */
	comp_t		ac_majflt;		/* Major Pagefaults */
	comp_t		ac_swaps;		/* Number of Swaps */
	char		ac_comm[ACCT_COMM];	/* Command Name */
};

typedef struct acct_v3  acct_v3_t;


//temprary structure for pacct 
struct acct
{
	char		ac_flag;		/* Flags */
	char		ac_version;		/* Always set to ACCT_VERSION */
	u_int16_t	ac_tty;			/* Control Terminal */
        u_int32_t	ac_btime;		/* Process Creation Time */
	u_int32_t	ac_utime;		/* User Time, in clock ticks */
	u_int32_t	ac_stime;		/* System Time, in clock ticks */
	u_int32_t	ac_etime;		/* Elapsed Time, in clock ticks */
	u_int32_t	ac_mem;	                /* Average Memory Usage ub clicks */
	u_int32_t	ac_io;			/* Chars Transferred by read/write*/
	u_int32_t	ac_rw;			/* Blocks Read or Written */
	u_int32_t	ac_minflt;		/* Minor Pagefaults */
	u_int32_t	ac_majflt;		/* Major Pagefaults */
	u_int32_t	ac_swaps;		/* Number of Swaps */
        u_int32_t	ac_exitcode;		/* Exitcode */
        u_int32_t	ac_uid;			/* Real User ID */
	u_int32_t	ac_gid;			/* Real Group ID */
        u_int32_t	ac_pid;			/* Process ID */
	u_int32_t	ac_ppid;		/* Parent Process ID */
        u_int16_t	ac_ahz;			/* AHZ */
        char		ac_comm[ACCT_COMM+1];	/* Command Name */
};

typedef struct acct acct_t;


/* comp_t is a 16-bit "floating" point number with a 3-bit base 8
  exponent and a 13-bit fraction. */

#define MANTSIZE        13                      /* 13 bit mantissa. */
#define EXPSIZE         3                       /* Base 8 (3 bit) exponent. */
#define MAXFRACT        ((1 << MANTSIZE) - 1)   /* Maximum fractional value. */

//encode comp_t
comp_t encode_comp_t(unsigned long val);

//decode comp_t to unsigned long
unsigned long decode_comp_t(comp_t ct);


/* comp2_t is 24-bit with 5-bit base 2 exponent and 20 bit fraction
   (leading 1 not stored) */

#define MANTSIZE2       20                      /* 20 bit mantissa. */
#define EXPSIZE2        5                       /* 5 bit base 2 exponent. */
#define MAXFRACT2       ((1ul << MANTSIZE2) - 1) /* Maximum fractional value. */
#define MAXEXP2         ((1 <<EXPSIZE2) - 1)    /* Maximum exponent. */

//encode comp2_t
comp2_t encode_comp2_t(unsigned long val);

//decode comp2_t to unsigned long
unsigned long decode_comp2_t(comp2_t ct);


/* float is 32-bit with 8-bit base 2^23 exponent and 23 bit fraction */

#define MANTSIZE3       23                    /* 23 bit mantissa. */
#define COFSIZE3        8                     /* 8 bit base 23 coefficients. */

//encode float
float encode_float(unsigned long val);

//decode float to unsigned long
unsigned long decode_float(float ct);

//convert big-endian to little-endian, floating point
float invf( float f );

#endif // _LIBPACCT_STRUCTS_H
