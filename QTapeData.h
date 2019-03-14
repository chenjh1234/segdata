// QTapeData.h: interface for the QTapeData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QTAPEDATA_H__61B5409F_5C91_4178_819C_A8EB310C45AB__INCLUDED_)
#define AFX_QTAPEDATA_H__61B5409F_5C91_4178_819C_A8EB310C45AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include <qstring.h>
#include<string.h>
#include<stdio.h>
#include<qtextcodec.h>
//copy:format
#define COPY_SEGY 0
#define COPY_IMAGE 1
//copymode
#define COPY_NEW 10
#define COPY_APPEND 11

#define SELECT_PAIR 3
// the button status:
#define SELECT_SHOT 0
#define SELECT_FILE 1
#define SELECT_RECORD 2
//tape format:
#define SEGDFMT 0
#define SEGYFMT 1
#define GRISYSFMT 2
#define CGGFMT 3
#define TARFMT 4
#define UNKNOW 8
// device mode:file tape
#define READ_MODE 0
#define WRITE_MODE 1
// copy status:
#define COPY_STOPED 0
#define COPY_COPYING 1
#define COPY_PAUSED 2

#include "QCommonDefine.h"

#define LINE_CHS 16 //tes read tape dume: characters of hex in a line

class QTapeData  
{
public:
	QString GetCH(QString ch);
	/** is the selecte range all 0*/
	bool IsParSelected();
	/** set the default parameters*/
	void SetCopyParDefault();
	/** do INIT*/
	void Init();
	QTapeData();
	virtual ~QTapeData();

	QString HexOut(unsigned char *,int bytes,int lines);
	void HexOutStr(char * , QString &s);
	void HexOutStrN(char * , QString &s);

// some chines word for button text:	 

	QTextCodec *m_ch;

	QString m_strChRead;
	QString m_strChWrite;
	QString m_strChPauseCopy;
	QString m_strChContinueCopy;
	QString m_strChStartCopy;
	QString m_strChAdvanced;
	QString m_strChNormal;

	QString m_strChSelectShot;
	QString m_strChSelectFile;
	QString m_strChSelectRecord;

	int m_iTapeBlockLen;
	unsigned char *m_pcBuf;
// Copy Parameters:
/** Select range */
	struct PAR_RANGE
	{
		int from[SELECT_PAIR];
		int to[SELECT_PAIR];
		int size;
		int files;
		int type;
	}m_ParSelectRange;
/** input sample rate*/
	int m_iParInputSI;
/** input tarce length*/
	int m_iParInputLTR;
/** copy format: COPY_SEGY,COPY_IMAGE*/
	int m_iCopyFormat;
/** copy mode: COPY_NEW,COPY_APPEND*/
	int m_iCopyMode;
/** output sample rate*/
	int m_iParOutputSI;
/** output tarce length*/
	int m_iParOutputLTR;
/** select button status :for SEGY:SHOT. fro Image: FILE,RECORD*/
	int m_iStatus;
//	int m_iParSelectType;

	int m_iTapeMode;//0,1,READ_MODE,WRITE_MODE
	int m_iFileMode;//0,1,READ_MODE,WRITE_MODE
	int m_iCopyStatus;//COPY_STOPED,COPY_COPYING,COPY_PAUSED;


//	bool m_bIsParSelected;
 

};

#endif // !defined(AFX_QTAPEDATA_H__61B5409F_5C91_4178_819C_A8EB310C45AB__INCLUDED_)
