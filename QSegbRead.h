// QRead.h: interface for the QRead class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SEGBREAD_H
#define SEGBREAD_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/**<
---------------segd read interface---------------------
export interface:
        int InitTape(QString s);#QStringList list
	int ReadTrace(float *trace,int sams);
	int CloseTape();//close tape;

  	int GetLtr();
	int GetSi();
	int GetFormat();
	int GetTrs();
	int GetAuxes();

internal interface:
	
	int ReadRecord();
	int ConvertData(float *trace,int sams);
	int FillMainHeader();
	int GetFileInfo();   




*/

#include<qstring.h>
#include <math.h>
#include <QDebug>

#include"QTapeIO.h"
#include "QSegCommon.h"
#include"QDebug"
#define TIF8_BYTES 24
//e:\\segb\\FLD_BP8122710174.SEGB,2013
#if 0
    #define SEGD_DISK_OFFSET 0x4da34
    #define HEAD_GAP 22
#endif
//d:\\2006\\segb\\9003069.001,2016
#if 1
    #define SEGD_DISK_OFFSET 14
    #define HEAD_GAP 14
#endif

#define DEBUG 0

#include <QStringList>

class QSegbRead :public QSegCommon
{
public:

        QSegbRead();
        virtual ~QSegbRead();
/**
	Impormant information in scan code header 
*/
	typedef struct 
	{
		int iScanTypeNumber; //1:  bcd
		int iChSetNumber;    //2:  bcd
		int iStartTime;      //3-4: bin start time
		int iEndTime;        //5-6 bin end time
		float mp;            //7-8 ; 7:-3~-10,8:s,4~-2;
		int iChs;            //9-10 bcd channels in Set
		int iChType;         //11h bin   1:sesmic 9:aux	
		int iGain;           //12l;bin
		int iAF;             //13-14;bcd; Alias Filter Frequency
		int iAS;             //15-16;bcd; Alias Filter Slope
		int iLC;             //17-18;bcd; Low Cut Filter
		int iLS;             //19-20;bcd; Low Cut Filter Slope
		int iNF1;             //21-22;bcd; First Notch Filter
		int iNF2;             //23-24;bcd; Second Notch Filter
		int iNF3;             //25-26;bcd; Third Notch Filter
		int iVS;              //30;bin; Vertical Stack
		int iCAB;             //31;bin; Streamer No.
		int iARY;             //32;bin; Array Forming
		 

	}SEGD_SCANHD_INFO;
/**
	Impormant information in general header
*/
	typedef struct 
	{
		int iFileNo;          //1-2						#2 :1-3
		int iFormatCode;      //3-4     BCD
		int iAddGHD;          //12h;    BCD
		// add 2008.6.7
		int iManufacturerID;   //17      DCD

		int iBytesPerScan;    //20-22   BCD
		int iSi;              //23:bin/16
		int iPolarity;        //24:h4 bit
		int iFiletype ;       //26h;    8: normal, 2 test
	    int iLtr ;            //26L-27   	            #2// 15-17
		int iScanTypes;       //28;  bcd
		int iMaxChSets;       //29;  bcd				#2: 4-5
	// added this two 2007.10
		int iSkew;            //30:bsd
		int iExtended;        //31:bcd  				#2 6-7:bin
		int iExternal;        //32:bcd					#2 8-9: bin
	//#2--------------------
		float version;        //11-12,float
		int iTrailer;         //13-14
	//#3-----------------
		float fShotLine;	  //4-6:int   7-8 :fraction:signed
		float fShotPoint;	  //9-11:int 12-13:fraction:signed
		int iShotIndex;       //14
		int iPhaseControl;    //15
		int iTypeVibrator ;   //16
		int iPhaseAngle;      //17-18 : signed short
		int iSourceSetNO;     //20;

/*
		float fReceiverLine;  //hd ex: 1-3
		float fReceiverPoint; //4-6
		int iReceiverIndex;   //7
		int samples:          //8-10;
*/
	}SEGD_GHD_INFO;
	

        char m_tif8[4];
        qint64 m_pPre,m_pNext,m_pThis;
        bool m_bTif8;
        int m_iRFlag;


	SEGD_GHD_INFO m_infoGHD;
	SEGD_SCANHD_INFO m_pinfoSHD[100];
//	MAINHD_INFO m_infoMHD;




//	int m_iUnit;
// 	int m_iBytes;
	/**this read return bytes */
	int m_iRbytes;
	/**last read return bytes */
	int m_iLastRBytes;

	/**data   bytes */
	int m_iDataBytes;
        int m_iShotBytes;
        int m_iScanBytes;
	/**on data smaple use ? bytes */
	int m_fFormatBytes;
	/**mp for this shot */
	float m_fMP;
	/**all channels for this shot */
	int m_iTrsAll;
	/**seismic channels for this shot */
	int m_iTrs;
	/**aux channels for this shot */
	int m_iAux;
	/**all channal sets of all the scan types*/
	int m_iSHDs;  
	int m_iHeadBytes;// header bytes
	int m_iTrailBytes;// trailer bytes
	int m_iOffsetBytes;// offset of the general header from the segd data file;
	int m_iTraceGapBytes;//gap bytes between the neighbour traces
	int m_iShotsInFile;// shots in this segd file;
        int m_iShot,m_iFFID;

//counter of the channels whthin the shot;
	int m_ic;
	int m_ica;//assistant channel counter;
        int m_icShot;
	int m_iCurrentTrace;
	int m_icFile;//segd file counter;
	//format :8015,8022,8024,8042,8044,8048,8058,8036,8038;

	int m_piFormat[10];
	int m_iFormatNum;//7:
        QStringList m_listFiles;
        int m_iAllFiles;
        QString m_file;// current file;
        bool _1st;
        unsigned char syncCode[4];

	 
	void Init();
/**
set the offset (header from the begin of the segd data file
*/
	int SetOffset(int off);
/**
get mp
*/
	float GetMP(unsigned char *);

 //	int InitTape(int iunit);
	int InitTape(QString file); 
/**
@par function:
	 1:get filelist
	 2:pass the listfile to m_tp;
	 3:NextFile
@return :
	>0: bytes of the file head;\n
	-1: open err\n
	-2: read err\n
        -3: not a segd\n
*/
        int InitTape(QStringList filelist);
        /**
        Close tape
        */
	int CloseTape();//close tape;


/**
@par function:
	Readrecord  
	and get the status \n
ok:
	1:conver data to buf

eof:
	1:GetFileInfo()

eot:
	NextFile:
	return 
err:
	1:skip a record
	2: invalid the trace 0
	3: read next record
@return:

	>0 samples read
	0 double eof;compelete
	-1 read header error
	-2 read first record err
	-3 not a segd
	-4: eot
	-5: skip error;
	-6: dkiped;

//the status of the next record
	  OK:  read ok,
		the position of the tape is : after next record;
	  ERR: read trace err
		 the position of the tape is : befor the  ERR record;
		 if we use ReadTrace again,we output a invalid trace,and skip the err record,and read the next
	 HERR: read file header err
		the position of the tape is : before the  file header;
	   if we use ReadTrace again,we output a invalid trace,and skip the err record,and read the next

	  EOT: next record is EOT;read shuld be end
	       the position of the tape is ,after EOT,
	  EOF: next record is EOF; this record is the last of this file,
		 the position of the tape is ,after EOF
	 DEOF: double eof met ,we can end the read now;
	 EOTERR: eot met,no record can read now;
	 SERR: ship tape err;

*/
  //	int ReadTrace(float *trace,int sams);
        int ReadShot(float *trace,int sams);
	
/**
function:
	ReadRecord

	1: read a tape record
	2:set status:EOF,EOT,ERR,OK
return :
	bytes read
	0:EOT:complete
	<0 error;

*/
//	int ReadRecord();
	
/**
function:
	convert data
	1:Fillmainheader
	2:convert
return:
	0:ok;

*/
//	int ConvertData(float *trace,int sams);
	
/**
function:
	get information from the data header\n
	and fill main header
  
	1:get info from data header
	2:is the trace seismic data or Aux
	3: fill main header 

return :
	data header bytes(include extender bytes);
*/
//	int SetMainHeader();

/**
		close last file;if not the firet
		open newfile
		getfile info

  return 0:complete
		-1,-2:error;
		-3: not a segd err
		>0 ok;
	 
*/
	int NextFile();

//	int GetLtr();
//	int GetSi();
//	int GetFormat();

	int GetTrs();
	int GetAuxes();

	
// protected:
//	QTapeIO m_tp;
	int GetFileInfo();   
/** 
GetFile Infor
function:

	1:read header 
	2: fill m_infoGHD
	3: fill m_infoSHD
	4: is segd?? and set m_iLtr,m_iSI,m_iFormat;
	5: set m_iSHDs;

return:
	0 eof
	-1 read err;
	-2 not a segd;
	>0 ok bytes of the block
*/


#define GET_C(FROM) (*((FROM)++))//get next char
#define GET_S(FROM) (*((FROM)++))//get next short
#define GET_UC(FROM) (*((FROM)++))//get next unsined char

void F8058_to_float (short from[], float to[], int len);
void F8048_to_float (short from[], float to[], int len);
void F8044_to_float (short from[], float to[], int len);
void F8042_to_float (char from[], float to[], int len);
void F8024_to_float (short from[], float to[], int len);
void F8022_to_float (char from[], float to[], int len);
void F8015_to_float (short from[], float to[], int len);

void F0015_to_float (short from[], float to[], int len);
void F8036_to_float (unsigned char from[], float to[], int len);
void F8038_to_float (short from[], float to[], int len);

};

#endif // !defined(AFX_QSEGDREAD_H__7A1F0548_492F_45F2_B0DF_D9322898FFD8__INCLUDED_)
/*
2008.6.7:
   add manufacturerID, for some data
2008.6.27:
	int m_iTraceGapBytes;//gap bytes between the neighbour traces
	process the last trace it read bytes not the whole trace
*/
