// QSeg2.cpp: implementation of the QSeg2 class.
//
//////////////////////////////////////////////////////////////////////

#include "QSeg2.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QSeg2::QSeg2()
{
	init ();
}

QSeg2::~QSeg2()
{

}
void QSeg2::init()
{
	m_iReVision = 1;
	setNumberOfTraces(-1);
	setSamples(-1);
	setDataFormat(4);

	m_listFileKeys 
	<< "ACQUISITION_DATE"//: dd/mm/yyyy
	<< "ACQUISITION_TIME"//hh:mm:ss
	<< "CLIENT"
	<< "COMPANY"
	<< "GENERAL_CONSTANT"
	<< "INSTRUMENT"
	<< "JOB_ID"
	<< "OBSERVER"
	<< "PROCESSING_DATE"
	<< "PROCESSING_TIME"
	<< "TRACE_SORT"//AC_ACQUIED,CDP_GATHER,CDP_STACK,
					//COMMON_OFFSET,COMMON_RECEIVER,COMMON_SOURCE
	<< "UNITS" //FEET,METERS,INCHES,CENTRMETERS,NONE
	<< "NOTE"
	;
	m_listFileKeys 
	<< "ALIAS_FILTER"
	<< "AMPLITUDE_RECOVERY"
	<< "BAND_REJECT_FILTER"
	<< "CDP_NUMBER"
	<< "CDP_TRACE"
	<< "CHANNEL_NUMBER"
	<< "DATUM"
	<< "DELAY"
	<< "DIGITAL_BAND_REJECT_FILTER"
	<< "DIGITAL_HIGH_CUT_FILTER"
	<< "DIGITAL_LOW_CUT_FILTER"
	<< "END_OF_GROUP"
	<< "FIXED_GAIN"
	<< "HIGH_CUT_FILTER"
	<< "LINE_ID"
	<< "LOW_CUT_FILTER"
	<< "POLARITY"
	<< "RAW_RECORD"
	<< "RECEIVER"
	<< "RECEIVER_GEOMETRY"
	<< "RECEIVER_LOCATION"
	<< "RECEIVER_SPECS"
	<< "RECEIVER_STATION_NUMBER"
	<< "SAMPLE_INTERVAL"
	<< "SHOT_SEQUENCE_NUMBER"
	<< "SKEW"
	<< "SOURCE"
	<< "SOURCE_GEOMETRY"
	<< "SOURCE_LOCATION"
	<< "SOURCE_STATION_NUMBER"
	<< "STACK"
	<< "SATIC_CORRECTIONS"
	<< "TRACE_TYPE"
	<< "NOTE"
	;

}
int QSeg2::setNumberOfTraces(int number)
{
	m_iNumberOfTraces = number;
	return number;
}

int QSeg2::getNumberOfTraces()
{
	return m_iNumberOfTraces;
}
int QSeg2::setSamples(int samples)
{
	m_iSamples = samples;
	return samples;

}
int QSeg2::getSamples()
{
	return m_iSamples;
}
int QSeg2::setDataFormat(int format )
{
	m_iDataFormat = format;
	return format;
}
int QSeg2::getDataFormat()
{
	return m_iDataFormat;
}



bool QSeg2::IsBigEndian()
{
//	int wdsize;
        int i;
        //qSysInfo(&wdsize,&b);
        i = QSysInfo::ByteOrder;
        if(i == QSysInfo::BigEndian)
            return true;
        else
            return false;
}

void QSeg2::swap2(char *c)
{
	char c1,c2;
 
	if(IsBigEndian())
	{
		c1 = *c;
		c2 = *(c+1);
		*c = c2;
		*(c+1) = c1;

	}
}
void QSeg2::swap2(char *ch, int len)
{
	char *p;
	int i;
	if(IsBigEndian())
	{
		for(i = 0; i < len ; i++)
		{
			p = ch + 2*i;
			swap2(p);
		}
	}
}
void QSeg2::swap4(char *ch)
{
	char c1,c2,c3,c4;
	char *c;
	c = ch;
	if(IsBigEndian())
	{
		c1 = *c;
		c2 = *(c+1);
		c3 = *(c+2);
		c4 = *(c+3);

		*c = c4;
		*(c+1) = c3;
		*(c+2) = c2;
		*(c+3) = c1;
	}
}

void QSeg2::swap4(char *ch, int len)
{
	char *p;
	int i;
	if(IsBigEndian())
	{
		for(i = 0; i < len ; i++)
		{
			p = ch + 4*i;
			swap4(p);
		}
	}
}

void QSeg2::setPathName(QString name)
{
	m_strPathName = name;
}

QString QSeg2::getPathName()
{
	return m_strPathName;
}
int QSeg2::getInt(char *buf1)
{
	int i;
	char buf[4];
	memcpy(buf,buf1,4);
	swap4(buf);
	i = *((int*)buf);
	return i;

}
int QSeg2::getShort(char *buf1)
{
	int i;
	char buf[2];
	memcpy(buf,buf1,2);

	
	swap2(buf);
	i = *((short *)buf);
	return i;
}
float QSeg2::getFloat(char *buf1)
{
//	int i;
	float f;
	char buf[4];
	memcpy(buf,buf1,4);

	swap4((char *)buf);
	f =*((float *)buf);
	return f;
}
/* F8015_to_float - convert 20 bit binary demultiplexed data into floating numbers
 *
 * Credits:
 *      EOPG: Marc Schaming, Jean-Daniel Tissot
 *      SEP:  Stew Levin - fixed low-order bit error in conversion
 *            of negative values on 2's complement machines.
 *            Use ldexp() function instead of much slower value*pow(2,expo)
 *
 * Parameters:
 *    from   - input vector
 *    to     - output vector
 *    len    - number of packets of 4 floats in vectors
 *
 */
/*
 *
 * Format 8015 is a 10 byte per 4 words (2 1/2 bytes per word)
 * representation.  According to the SEG specifications, the
 * bit layout of the 10 bytes is:
 *
 *
 *  Bit       0     1     2     3     4     5     6     7
 *-----------------------------------------------------------
 * Byte 1    C3    C2    C1    C0    C3    C2    C1    C0    Exponents for
 * Byte 2    C3    C2    C1    C0    C3    C2    C1    C0    channels 1 thru 4
 *
 * Byte 3     S    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7   Channel 1
 * Byte 4    Q-8   Q-9   Q-10  Q-11  Q-12  Q-13  Q-14  Q-15
 * Byte 5     S    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7   Channel 2
 * Byte 6    Q-8   Q-9   Q-10  Q-11  Q-12  Q-13  Q-14  Q-15
 * Byte 7     S    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7   Channel 3
 * Byte 8    Q-8   Q-9   Q-10  Q-11  Q-12  Q-13  Q-14  Q-15
 * Byte 9     S    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7   Channel 4
 * Byte 10   Q-8   Q-9   Q-10  Q-11  Q-12  Q-13  Q-14  Q-15
 *
 * S=sign bit. - (One = negative number)
 * C=binary exponents. - This is a 4 bit positive binary exponent of 2
 *               CCCC
 *   written as 2     where CCCC can assume values of 0-15.  The four
 *   exponents are in channel number order for the four channels starting
 *   with channel one in bits 0-3 of Byte 1.
 * Q1-15-fraction. - This is a 15 bit one's complement binary fraction.
 *   The radix point is to the left of the most significant bit (Q-1)
 *                                  -1
 *   with the MSB being defined as 2 .  The sign and fraction can assume
 *                  -15       -15
 *   values from 1-2   to -1+2  .  Negative zero is invalid and must be
 *   converted to positive zero.
 *                                        CCCC    MP                   MP
 * Input signal = S.QQQQ,QQQQ,QQQQ,QQQ x 2    x 2    millivolts where 2
 *   is the value required to descale the data word to the recording
 *   system input level.  MP is defined in Byte 8 of each of the corre-
 *   sponding channel set descriptors in the scan type header.
 * Note that in utilizing this data recording method, the number of data
 *   channels per channel set must be exactly divisible by 4 in order to
 *   preserve the data grouping of this method.
 */


void QSeg2::f3ToFloat(short from[], float to[], int len)
{
 register int i;
 register short ex1_4;
 int expo;
 short fraction;

 for (i = 0; i < len; i += 4) {
      ex1_4 = GET_S(from);
 	expo = ((ex1_4 >> 12) & 15) - 15;
      fraction = GET_S(from);
	if (fraction < 0) fraction = -(~fraction);
 	*(to++) = ldexp((double) fraction, expo);

 	expo = ((ex1_4 >> 8) & 15) - 15;
      fraction = GET_S(from);
	if (fraction < 0) fraction = -(~fraction);
 	*(to++) = ldexp((double) fraction, expo);

 	expo = ((ex1_4 >> 4) & 15) - 15;
      fraction = GET_S(from);
	if (fraction < 0) fraction = -(~fraction);
 	*(to++) = ldexp((double) fraction, expo);

 	expo = (ex1_4 & 15) - 15;
      fraction = GET_S(from);
	if (fraction < 0) fraction = -(~fraction);
 	*(to++) = ldexp((double) fraction, expo);
 }
} 

int QSeg2::f1ToFloat(char *buf, float *tr, int len)
{
	int i,id;

	for(i = 0; i < len; i++)
	{
		id = getShort(buf + i*sizeof(short));
		tr[i] = id;
	}
	return 0;
}

int QSeg2::f2ToFloat(char *buf, float *tr, int len)
{
	int i,id;

	for(i = 0; i < len; i++)
	{
		id = getInt(buf + i*sizeof(int));
		tr[i] = id;
	}
	return 0;
}

int QSeg2::f4ToFloat(char *buf, float *tr, int len)
{
	int i;
	float f4;

	for(i = 0; i < len; i++)
	{
		f4 = getFloat(buf + i*sizeof(float));
		tr[i] = f4;
	}
	return 0;

}

int QSeg2::f5ToFloat(char *buf, float *tr, int len)
{
	// this is not right;
	int i;
	float f4;

	for(i = 0; i < len; i++)
	{
		f4 = getFloat(buf + i*sizeof(float));
		tr[i] = f4;
	}
	return 0;
}

 


