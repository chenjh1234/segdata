// QSeg2.h: interface for the QSeg2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QSEG2_H__DA2DEBC8_2611_449E_BD56_9B2290E1DC15__INCLUDED_)
#define AFX_QSEG2_H__DA2DEBC8_2611_449E_BD56_9B2290E1DC15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <qstring.h>
#include <qstringlist.h>
#include <math.h>
#include<QSysInfo>


class QSeg2  
{
public:

#define GET_C(FROM) (*((FROM)++))
#define GET_S(FROM) (*((FROM)++))
#define GET_UC(FROM) (*((FROM)++))

	int f5ToFloat(char *buf,float *tr,int len);// is not right
	int f4ToFloat(char *buf,float *tr,int len);
	int f2ToFloat(char *buf,float *tr,int len);
	int f1ToFloat(char * buf,float * tr,int len);
	void f3ToFloat (short from[], float to[], int len);
	 

	QString getPathName();
	QSeg2();
	virtual ~QSeg2();

/**
	set the path that seg2 files will be located.
*/
	void setPathName(QString name);

//
	int setNumberOfTraces(int number);
	int getNumberOfTraces();
	int setSamples(int samples);
	int getSamples();
	int setDataFormat(int format );
	int getDataFormat();
//
	void swap4(char *ch,int len);
	void swap4(char *ch);
	void swap2(char *c);
	void swap2(char *ch,int len);
	bool IsBigEndian();
	void init();

	int getInt(char *c);
	int getShort(char *c);
	float getFloat(char *c);

        int	m_iReVision;
        int	m_iNumberOfTraces;
        int	m_iSizeOfTrPointerBlock;
        int	m_iSizeOfTrHeaderBlock;
        int	m_iSizeOfTrDataBlock;
        int	m_iSamples;
        int	m_iDataFormat;

	QString	m_strCurrentFileName;
	QString m_strFileString;
	QString m_strTraceString;
	QString m_strPathName;
	QString m_strErr;
	QStringList m_listFileKeys;
	QStringList m_listTraceKeys;


};

#endif // !defined(AFX_QSEG2_H__DA2DEBC8_2611_449E_BD56_9B2290E1DC15__INCLUDED_)
/**
-----------------------------
seg2 format:
------------
  file description block

  trace description block1
  data block1
  trace description block2
  data block2
  ......
  trace description block N
  data blockN
--------------------------------
file description block:
----------------------
	00-01: file descriptor block ID  : 553a 
	02-03: ReVision Number           :01
	04-05: M = size of Trace Pointer Sub-Block(in bytes)
	06-07: N = size of traces in this file
	08   : Size of String Terminator :01
	09   : First String Terminator Character :NULL
	10   : Second String Terminator Character :NULL
	11   : Size of Line Terminator :01
	12   : First Line Terminator Character :0a
	13   : Second Line Terminator Character :NULL
	14-31: Reserved.
	32-35: Pointer to Trace Descriptor Block 1
	36-39: Pointer to Trace Descriptor Block 2
	...
	32 + 4*(n-1) - 32 + 4*(n):
	     : Pointer to Trace Descriptor Block N
	32+M :string1 
			string2
			....
			string Z
--------------------------------------
Trace description block:
----------------------
	00-01: Trace description ID: 2244
	02-03: X = size of this block(inbytes)
	04-07: Y = size of corresponding Data Block(in bytes)
	08-11: NB = number of samples in data block
	12   : Data format code:
			01: 16-bit fixed point
			02:32-bit fixed point
			03: 20-bit floating point(SEGD)
			04: 32-bit floating point(IEEE standard)
			05: 64-bit floating point(IEEE standard)
	13-31: Reserved
	   32: strings1
			string 2
			....
			string Z
-----------------------------------------------

	 



*/
