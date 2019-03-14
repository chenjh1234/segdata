// QSegdRead.h: interface for the QSegdRead class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QSEGDREAD_H__7A1F0548_492F_45F2_B0DF_D9322898FFD8__INCLUDED_)
#define AFX_QSEGDREAD_H__7A1F0548_492F_45F2_B0DF_D9322898FFD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/**<
---------------segd read interface--------------------- 
version3: 2019.3 
 
export interface:
    int InitTape(QString s);# or int InitTape(QStringList s);
    int readShot(float *trace,int *head);
 
    int readTrace(float *trace,int *head);
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



// m_iTraceGapBytes :4, 80 not sure
*/

#include<qstring.h>
#include <math.h>

#include"QTapeIO.h"
#include "QSegCommon.h"
#include <QMap>
#include"QDebug"
#define TIF8_BYTES 24
#define SEGD_HEAD_LEN 20
#define TRACE_GAP 4 
#define TRACE_GAP1 4*20 
#define SEGD_DISK_OFFSET 4*1024*1024
#define SEGD_DISK_OFFSET1 0
#define MAX_CH_SETS  100
#define GHB_LEN   32
#define MY_DEBUG 0

#include <QStringList>

class QSegdRead : public QSegCommon
{
public:


   QSegdRead();
   virtual ~QSegdRead();
/**
    Impormant information in scan code header 
*/
   typedef struct
   {
      int iScanTypeNumber; //1:  bcd
      int iChSetNumber;    //2:  bcd;#3: 3-4int
      int iStartTime;      //3-4: bin start time #3:5-8int,TF
      int iEndTime;        //5-6 bin end time  #3:9-12int,TE
      float mp;            //7-8 ; 7:-3~-10,8:s,4~-2;
      int iChs;            //9-10 bcd channels in Set
      int iChType;         //11h bin   1:sesmic 9:aux
      int iChTypeCode;     //#3:4 int1 0x10:seismic
      int iSamples;        //#3:13-16  int4;NS,TE=TF+NS¡ÁSR. 
      int iSi;            //#3:24-26  int3;US
      float fDeScale;        //#3:17-20  float4; mybe MP
      int iTHE;            //#3:28;int1,trace heaser extension
      int iEFH;            //#3:29H;  int1H (0-3);   Extended Header flag        
      int iGain;           //12l;bin //#3:29L;  int1H(4-8);   Extended Header flag 
      int iAF;             //13-14;bcd; Alias Filter Frequency #3 :33-36 float4
      int iAS;             //15-16;bcd; Alias Filter Slope     #3:41-44 float
      int iLC;             //17-18;bcd; Low Cut Filter         #3:37-40 float
      int iLS;             //19-20;bcd; Low Cut Filter Slope   #3:45-48 float
      int iNF1;             //21-22;bcd; First Notch Filter    #3,49-52 float 
      int iNF2;             //23-24;bcd; Second Notch Filter   #3,53-56 float 
      int iNF3;             //25-26;bcd; Third Notch Filter    #3,57-60 float 
      int iVS;              //30;bin; Vertical Stack           #3,30  int1
      int iCAB;             //31;bin; Streamer No.             #3,31 int1
      int iARY;             //32;bin; Array Forming            #3,27
      int iFPH;             //#3=61,int1 Filter phase 
      int iPHU;             //#3=62, int1  Physical unit 
      int iFDL;           //#3 65-68 float4, Filter delay 
      char cDSC[27];        //#3 69-95 char[27],  Description
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
      int iManufacturerID;   //17      BCD
      int iManufacturerSerial;   //18-19      BCD

      int iBytesPerScan;    //20-22   BCD
      int iSi;              //23:bin/16  #2:25-27
      int iPolarity;        //24:h4 bit
      int iFiletype;       //26h;    8: normal, 2 test
      int iLtr;            //26L-27   	#2// 15-17  //17-20(V3)
      int iScanTypes;       //28;  bcd
      int iMaxChSets;       //29;  bcd				#2: 4-5
                            // added this two 2007.10
      int iSkew;            //30:bsd
      int iExtended;        //31:bcd  				#2 6-7:bin
      int iExternal;        //32:bcd					#2 8-9: bin
                            //#2 --------------------
      int iVersion;         //11 int
      int iSubVersion;      //12
      int iTrailer;         //13-14
                            //#3------version <3-----------
      float fShotLine;      //4-6:int   7-8 :fraction:signed
      float fShotPoint;     //9-11:int 12-13:fraction:signed
      int iShotIndex;       //14
      int iPhaseControl;    //15
      int iTypeVibrator;   //16
      int iPhaseAngle;      //17-18 : signed short
      int iSourceSetNO;     //20;

     
      // verson3: 
      int iShotGroup; //#2
      qint64 iTime; // #3 =1-8
      qint64 iShotLen,iDataLen;//#3=9-16,17-24
      int iHeadLen; //#3 =25-28
      int iExtendedRecordMode; //#3=29
      int iRelativeTimeMode; //#3=30

/*
        float fReceiverLine;  //hd ex: 1-3
        float fReceiverPoint; //4-6
        int iReceiverIndex;   //7
        int samples:          //8-10;
*/
   }SEGD_GHD_INFO;


   char m_tif8[4];
   qint64 m_pPre,m_pNext,m_pThis;//tif8 used
   bool m_bTif8;
   int m_iRFlag;


   int m_iVersion;//3.0

   SEGD_GHD_INFO m_infoGHD;
   SEGD_SCANHD_INFO m_pinfoSHD[MAX_CH_SETS];
//	MAINHD_INFO m_infoMHD;


//	int m_iUnit;
// 	int m_iBytes;
   /**this read return bytes */
   int m_iRbytes;
   /**last read return bytes */
   int m_iLastRBytes;
   /**data header bytes */
   int m_iDataHDBytes;
   /**data   bytes */
   int m_iDataBytes;
   /**on data smaple use ? bytes */
   int m_fFormatBytes;
   /**mp for this shot */
   float m_fMP;
   float m_fDeScale;// version3 
   /**all channels for this shot */
   int m_iTrsAll;
   /**seismic channels for this shot */
   int m_iTrs;
   /**aux channels for this shot */
   int m_iAux;
   /**all channal sets of all the scan types*/
   int m_iSHDs;
   int m_iScan,m_iCHSets,m_iExHDs ,m_iSensorType,m_iSensorMove;//for current trace decoded;

   int m_iSCANB_LEN;
   int m_iHeadBytes; // file header bytes
   int m_iTrailBytes; // trailer bytes
   int m_iOffsetBytes; // offset of the general header from the segd data file;
   int m_iTraceGapBytes; //gap bytes between the neighbour traces
   int m_iShotsInFile; // shots in this segd file;
   int m_iTraceBytes; //read bytes for this trace ;
   int m_iChType;// ch type in this trace in  shot

//counter of the channels whthin the shot;
   #if 1
   int m_icTrShot;// for read bytes calculatel
   int m_ic; //count seismic trace in shot for setHD
   int m_ica; //assistant channel counter; for setHD
   int m_icShot;// counter for shot
   int m_iCurrentTrace;// counter for seismic or aux
   int m_icFile; //segd file counter;
                 //format :8015,8022,8024,8042,8044,8048,8058,8036,8038;
   #endif
   int m_piFormat[10];
   int m_iFormatNum; //7:
   QStringList m_listFiles;
   int m_iAllFiles;
   QString m_file; // current file;
   bool m_bFirst;
   QMap<int,QString> mapManufactory;


   void Init();
   void clearBuf();
   /**set the offset (header from the begin of the segd data file*/
   int SetOffset(int off);
   void setManufactory();
   /**get mp*/
   float GetMP(unsigned char *);

   //	int InitTape(int iunit);
   int InitTape(QString file);
   int InitTape(QStringList filelist);
/**<
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

   int CloseTape(); //close tape;
/**<
Close tape
*/

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
   int ReadTrace(float *trace, int sams);// read record if eof get fileinfo or next file,and read next trace in next shot 
   int readTrace(float *trace, int *head);// read record if eof get fileinfo or next file
   int readShot(float *trace,int *head ); // return trs; str = EOF,EOT,ERR

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
   int ReadRecord();


/**
function:
    convert data
    1:Fillmainheader
    2:convert
return:
    0:ok;

*/
   int ConvertData(float *trace, int sams);

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
   int SetMainHeader();
   int SetMainHeader1();
   int setHeader(int *head);

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
   int GetFileInfo1();
   int decodeGH1(unsigned char *ip);
   int decodeGH2(unsigned char *ip);
   int decodeGH3(unsigned char *ip);
   int decodeChannelSets(unsigned char *p);
   int decodeTH(unsigned char *ip);
   int decodeTHE(unsigned char *ip);
   bool isSegd();
   void prGH();
   void prChSets();
   void prTHD();
   int GetFileInfo();
   bool isSegdD();
   int setBytes();
   //int getExHD();
   int get1stTrPos();
   int curChSets();
   int curChSetsIdx();
   int curScan();
 
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

   void F8058_to_float(short from[], float to[], int len);
   void F8048_to_float(short from[], float to[], int len);
   void F8044_to_float(short from[], float to[], int len);
   void F8042_to_float(char from[], float to[], int len);
   void F8024_to_float(short from[], float to[], int len);
   void F8022_to_float(char from[], float to[], int len);
   void F8015_to_float(short from[], float to[], int len);

   void F0015_to_float(short from[], float to[], int len);
   void F8036_to_float(unsigned char from[], float to[], int len);
   void F8038_to_float(short from[], float to[], int len);

};

#endif // !defined(AFX_QSEGDREAD_H__7A1F0548_492F_45F2_B0DF_D9322898FFD8__INCLUDED_)
/*
2008.6.7:
   add manufacturerID, for some data
2008.6.27:
    int m_iTraceGapBytes;//gap bytes between the neighbour traces
    process the last trace it read bytes not the whole trace
2019.1:version3 
 
Channel type Identification (one byte, unsigned binary).:
0016    Unused 
1016    Seis 
1116 Electromagnetic (EM) 
2016    Time break 
2116    Clock timebreak 
2216    Field timebreak 
3016    Up hole 
4016    Water break 
5016    Time counter 
6016    External Data 
6116    Acoustic range measurement 
6216    Acoustic reference measured (correlation reference) 
6316    Acoustic reference nominal (correlation reference) 
7016 Other 
8016    Signature/unfiltered 
9016    Signature/filtered 
9116    Source signature/unfiltered 
9216    Source signature/filtered 
9316    Source signature/estimated 
9416    Source signature/measured 
9516    Source base plate 
9616    Source reference sweep 
9716    Source other 
9816    Source reference pilot 
9916    Source mass 
9A16    Source excitation 
9B16    Source valve 
9C16    Source overload 
A016    Auxiliary Data Trailer (no longer used) 
B016    True reference sweep (correlation reference) 
B116    Radio reference sweep 
B216    Radio similarity signal 
B316    Wireline reference sweep 
C016    Depth 
C116    Wind 
C216    Current 
C316    Voltage 
C416    Velocity 
C516    Acceleration 
C616    Pressure 
C716    Tension 
C816    Tilt measurement 
C916  Angle measurement 
F016    Calibration trace (time series)  
 
*/

