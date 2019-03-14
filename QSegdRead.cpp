// QSegdRead.cpp: implementation of the QSegdRead class.
//
//////////////////////////////////////////////////////////////////////

#include "QSegdRead.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QSegdRead::QSegdRead() : QSegCommon()
{
   Init();
}

QSegdRead::~QSegdRead()
{

}
void QSegdRead::clearBuf()
{
    //	m_iIbytes = 65000;
   memset((void *)&m_infoMHD, 0, sizeof(MAINHD_INFO));
   memset((void *)&m_infoGHD, 0, sizeof(SEGD_GHD_INFO));
   for (int i = 0; i < MAX_CH_SETS; i++)
   {
      memset((void *)&m_pinfoSHD[i], 0, sizeof(SEGD_SCANHD_INFO));
   }

}
void QSegdRead::Init()
{
    clearBuf();


   m_piFormat[0] = 8015;
   m_piFormat[1] = 8022;
   m_piFormat[2] = 8024;
   m_piFormat[3] = 8042;
   m_piFormat[4] = 8044;
   m_piFormat[5] = 8048;
   m_piFormat[6] = 8058;
//2008.6.12 add 8036,8038
   m_piFormat[7] = 8036;
   m_piFormat[8] = 8038;
   m_piFormat[8] = 8088; // for SD30
   m_iVersion = 2;
   m_iSCANB_LEN = 32;
   //m_piFormat[10] = 8058;

   m_iFormatNum = 10;
   m_iLastRBytes = 0;
   m_iRbytes = 0;
   m_fMP = 0;
   m_fDeScale = 1;
   m_iDataHDBytes = 20;
   m_iDataBytes = 0;
   m_iHeadBytes = TAPE_BLOCK;
   m_iSamples = 0;
#if 1
   m_icTrShot = 0;
   m_ic = 0;
   m_ica = 0;
   m_icFile  = 0;
   m_icShot  = 0;
#endif
   m_iOffsetBytes = SEGD_DISK_OFFSET; // 4 * 1024 * 1024;
                                      //m_iOffsetBytes = 0;
   m_iTraceGapBytes = 0;
   m_iShotsInFile = 0;
   strcpy(m_tif8, "TIF8");
   m_bTif8 = false;
   m_pPre = 0;
   m_pNext = 0;
   m_pThis = 0;
   m_bFirst = true;
   setManufactory();

}
void QSegdRead::setManufactory()
{
   QMap<int, QString>  mapM;
   mapM[01] = "Alpine Geophysical Associates, Inc";
   mapM[02] = "Applied Magnetics Corporation";
   mapM[03] = "Western Geophysical Exploration Products";
   mapM[04] = "SIE, Inc.";
   mapM[05] = "Dyna-Tronics Mfg. Corporation";
   mapM[06] = "Electronic Instrumentation, Inc;";
   mapM[07] = "Halliburton Geophysical Services, Inc.;";
   mapM[8] = "Fortune Electronics, Inc.";
   mapM[9] = "Geo Space Corporation";
   mapM[10] = "Leach Corporation";
   mapM[11] = "Metrix Instrument Co";
   mapM[12] = "Redcor Corporation";
   mapM[13] = "Sercel ";
   mapM[14] = "Scientific Data Systems (SDS)";
   mapM[15] = "Texas Instruments, Inc.";
   mapM[17] = "GUS Manufacturing, Inc.";
   mapM[18] = "Input/Output, Inc.";
   mapM[19] = "Geco-Prakla";
   mapM[20] = "Fairfield Industries, Incorporated";
   mapM[22] = "Geco-Prakla";
   mapM[31] = "Japex Geoscience Institute";
   mapM[32] = "Halliburton Geophysical Services, Inc";
   mapM[33] = "Compuseis, Inc";
   mapM[34] = "Syntron, Inc.";
   mapM[35] = "Syntron Europe Ltd";
   mapM[36] = "Opseis	";
   mapM[39] = "Grant Geophysical";
   mapM[40] = "Geo-X";
   mapM[41] = "PGS Inc.";
   mapM[42] = "SeaMap UK Ltd.";
   mapM[43] = "Hydroscience";
   mapM[44] = "JSC";
   mapM[45] = "Fugro";
   mapM[46] = "ProFocus Systems ";
   mapManufactory = mapM;
   return;
}

//init tape===========================================================;
/**
    work:
    1: get m_iUnit;
    2: open tape
    3: GetfileInfo()
    4: read first trace;Set the Status;

@return 
    >0: bytes of the file head;
    -1: open err
    -2: read err
    -3: not a segd
*/
int QSegdRead::InitTape(QString file)
{
   int iret;
   QStringList list;
   list.append(file);
   iret =  InitTape(list);
   //qDebug()<< "trs In InitTape ="<< m_iTrs<<m_iAux<<m_iTrsAll;
   return iret;

}
#if 0
int QSegdRead::openTape(QString d)
{
   int iret;
   m_tp->setType(DEV_TAPE);
   if (!m_tp->open(d)) return OPENFILE_ERR; //cannot open
   iret = GetFileInfo(); // return header:
   if (iret <=0) return OPENFILE_ERR;
   return iret;
}
#endif
int QSegdRead::InitTape(QStringList filelist)
{
   int iret;
//	int iunit;
// get unit
//	m_iIUnit = iunit;
//open tape
   m_tp->setType(DEV_DISK);
   m_listFiles = filelist;
   m_iAllFiles = m_listFiles.size();
   m_icFile = 0;

   iret = NextFile();
   qDebug() << "offset = " << m_iOffsetBytes;
   qDebug() << "NextFile in initTape return =" << iret;

//2008.6.5:
// default :offset = 4*1024*1024;
// for support offset = 0
// read err -2:
// if the offset point to thr end of file,it will return read err -2
// update 2010.4.21,
//	if(iret == -3)
   if (iret == -3 || iret == -2) //-3:not a segd,-2 read err
   {
      SetOffset(0);
      m_tp->close();
      m_icFile = 0;
      iret = NextFile();
      qDebug() << "offset1 = " << m_iOffsetBytes;
      qDebug() << "NextFile in initTape return =" << iret;
   }
   qDebug() << "trs In InitTape =" << m_iTrs << m_iAux << m_iTrsAll;
   qDebug() << "end of initTape========================================\n";

   return iret;
}
/*
int QSegdRead::InitTape(QString file)
{
    QString line;
    QStringList list;
    QFile f(file);
    list.clear();
    if ( f.open( IO_ReadOnly ) ) 
    {
        QTextStream stream( &f );
        while ( !stream.atEnd() ) 
        {
            line = stream.readLine(); // line of text excluding '\n'
            list.append(line);
        }
        f.close();
    }
    else
        return -1;// cannot open file

    //m_listFile = list;
    return InitTape(list);
}
 
int QSegdRead::InitTape(int iunit)
{
    int iret;
//	int iunit;
// get unit
    m_iIUnit = iunit;
//open tape
    iret = m_tp->TapeOpen(iunit);
    if(iret !=0) return -1;//open tape err
    m_tp->TapeRewind( );
//    m_iTrsInFile = 0;
// read the file header;
    iret = GetFileInfo();
    if(iret == -1) return -2;//read err
    else if(iret == 0) return -2;//read err
    else if(iret == -2) return -3;// not segd
    else if(iret >0) 
    {
    
// read 1 trace
    ReadRecord();
//	m_iTrsInFile ++;
        return iret;
    }
    return 0;
} 
//======================================================================
    ReadRecord

    1: read a tape record
    2:set status:EOF,EOT,ERR,OK

@return :
    bytes read;
    0:EOT:complete
    <0 error;

*/

int QSegdRead::ReadRecord()
{
   int bytes;
   // ltr from the general header:

   setBytes();//befor Readtrace
   bytes = m_iTraceBytes;

   if (m_bTif8)
   {
      //qDebug() << "seek in read record=  " << m_pPre<<m_pThis<< m_pNext << m_ic<<m_ica;
      m_tp->seek(m_pNext);
      //read gap:
      bytes = 24;
      m_iRbytes = m_tp->read(m_pcBuf, bytes);
      m_iRFlag = getInt(m_pcBuf + 4);
      m_pPre = getInt64(m_pcBuf + 8);
      m_pThis = m_pNext;
      m_pNext = getInt64(m_pcBuf + 16);
      if (m_iRFlag != 0)
      { //file mark
         m_strStatus = "EOF";
         //qDebug() << "file mark in readRecord: shot ,ic,ica=" <<m_icShot <<m_ic << m_ica;
         qDebug() << "Shot:" << m_infoMHD.iShot << " Traces:" << m_ic << " Aux:" << m_ica;

         return m_iRbytes;

      }
      bytes = m_pNext - m_pThis;
   }
   //read data:
  

   m_iRbytes = m_tp->read(m_pcBuf, bytes);
    if (MY_DEBUG) 
    {
       qDebug() << "read bytes, rbytes = " << bytes << m_iRbytes ;
    }


   if (m_debug)
   {

      qDebug("bytes,dataDH,dataBytes,returnBytes = %d,%d,%d,%d\n", bytes, m_iDataHDBytes, m_iDataBytes, m_iRbytes);
   }

   if (m_iRbytes < 0)
   { //err
      m_strStatus = "ERR";
   }
   else if (m_iRbytes == bytes)
   { //ok
      m_strStatus = "OK";
   }
   else if (m_iRbytes == 0)
   { //end of this file
     //when read is not enough,we ignore the last read;
      if (m_tp->eofFlag) m_strStatus = "EOF";
      if (m_tp->eotFlag) m_strStatus = "EOT";
      return 0;
   }
   else //return is not enough;
   { //end of this file
     //when read is not enough,we got the last read;
     //m_strStatus = "OK";// 2008.8
      if (m_tp->getType() == DEV_DISK)
      {
         m_strStatus = "EOT";
      }
      else
      {
         m_strStatus = "OK";

      }

   }
   //m_ic ++;
//		m_iCurrentTrace = m_ic;
#if 0
   if (m_ic == m_iTrsAll)
   {
      m_strStatus = "EOF"; // never come to it;
                           //2008.11.3 :for trace
//			m_ic = 0;
//			m_ica = 0;
   }
#endif

   return m_iRbytes;
}
void QSegdRead::prGH()
{
   if (1)
   {
      qDebug("PR of File Infor GHeader ==================================================");
      qDebug() << "GH1 begin------------";
      qDebug("iFileNo = %d", m_infoGHD.iFileNo);
      qDebug("iFormatCode = %d", m_infoGHD.iFormatCode);
      qDebug("iManufacturerID = %d", m_infoGHD.iManufacturerID);
      qDebug() << mapManufactory[m_infoGHD.iManufacturerID];
      qDebug("iAddGHD = %d", m_infoGHD.iAddGHD);
      qDebug("iBytesPerScan = %d", m_infoGHD.iBytesPerScan);
      qDebug("iSi = %d", m_infoGHD.iSi);
      qDebug("iFiletype = %d", m_infoGHD.iFiletype);
      qDebug("iLtr  = %d", m_infoGHD.iLtr);
      qDebug("iScanTypes = %d", m_infoGHD.iScanTypes);
      qDebug("iMaxChSets = %d", m_infoGHD.iMaxChSets);
      //qDebug("Channel sets of ALL: = %d", m_iSHDs);
      qDebug("iSkew   = %d", m_infoGHD.iSkew);

      qDebug("extended : = %d", m_infoGHD.iExtended);
      qDebug("external : = %d", m_infoGHD.iExternal);
//2
      qDebug() << "GH2 begin----------";
      qDebug("version : = %d", m_infoGHD.iVersion);
      qDebug("iTrailer : = %d", m_infoGHD.iTrailer);
      qDebug() << "Shot Group= " << m_infoGHD.iShotGroup;

//3
//qDebug() << "GH3  begin========================";
      qDebug() << "GH3 begin version3--------";
      qDebug() << "time zero= " << m_infoGHD.iTime;
      qDebug() << "Shot Len= " << m_infoGHD.iShotLen;
      qDebug() << "Data Len= " << m_infoGHD.iDataLen;
      qDebug() << "Head Len= " << m_infoGHD.iHeadLen;

      qDebug() << "iExtendedRecordMode= " << m_infoGHD.iExtendedRecordMode;
      qDebug() << "iRelativeTimeMode= " << m_infoGHD.iRelativeTimeMode;

      qDebug() << "GH3 version<3:-------";

      qDebug("fShotLine : = %f", m_infoGHD.fShotLine);
      qDebug("fShotPoint : = %f", m_infoGHD.fShotPoint);
      qDebug("iShotIndex: = %d", m_infoGHD.iShotIndex);
      qDebug("iPhaseControl : = %d", m_infoGHD.iPhaseControl);
      qDebug("iTypeVibrator : = %d", m_infoGHD.iTypeVibrator);
      qDebug("iPhaseAngle : = %d", m_infoGHD.iPhaseAngle);
      qDebug("iSourceSetNO : = %d", m_infoGHD.iSourceSetNO);
   } //end of debug
}
void QSegdRead::prChSets()
{
   int i;
   qDebug() << "PR of File Infor channel sets =================" << m_iSHDs;

   for (i = 0; i < m_iSHDs; i++)
   {
      qDebug(" no=%d, scantype = %d. channel sets = %d,trs = %d, time = %d,type = %d",
             i,
             m_pinfoSHD[i].iScanTypeNumber,
             m_pinfoSHD[i].iChSetNumber,
             m_pinfoSHD[i].iChs,
             m_pinfoSHD[i].iEndTime,
             m_pinfoSHD[i].iChType);
      qDebug() << " iSample,iSi,iTHE,fDeScale,iGain ="<< m_pinfoSHD[i].iSamples << m_pinfoSHD[i].iSi << m_pinfoSHD[i].iTHE << m_pinfoSHD[i].fDeScale << m_pinfoSHD[i].iGain;
   }
   qDebug() << "trs,aux,alltrs =" << m_iTrs << m_iAux << m_iTrsAll;
   /*
int iScanTypeNumber; //1:  bcd
      int iChSetNumber;    //2:  bcd;#3: 3-4int
      int iStartTime;      //3-4: bin start time #3:5-8int,TF
      int iEndTime;        //5-6 bin end time  #3:9-12int,TE
      float mp;            //7-8 ; 7:-3~-10,8:s,4~-2;
      int iChs;            //9-10 bcd channels in Set
      int iChType;         //11h bin   1:sesmic 9:aux
      int iChTypeCode;     //#3:4 int1 0x10:seismic
      int iSample;        //#3:13-16  int4;NS,TE=TF+NS¡ÁSR. 
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
      */
}
int QSegdRead::decodeGH1(unsigned char *p)
{
   unsigned char *ip;
   ip = (unsigned char *)p;

   qDebug() << " GH1 start";

//get file number: //1-2
   if (ip[0] != 0xff) m_infoGHD.iFileNo = BCD2Int((unsigned char *)ip, 4, true);
   else m_infoGHD.iFileNo = -1;

//get format:3-4
   m_infoGHD.iFormatCode = BCD2Int(ip + 3 - 1, 4, true);

//get additional gen header:12h
   if (((ip[12 - 1]) & (0xF0)) != 0xF0) m_infoGHD.iAddGHD = getInt1H(ip + 12 - 1);
   else m_infoGHD.iAddGHD = -1; //version3
//		iManufacturerID: 17 ,add 2008.6.7
   m_infoGHD.iManufacturerID = BCD2Int(ip + 17 - 1, 2, true);
// add 2008.6:
// cjh this is not a formular:? but we flloow it 201610
//   if (m_infoGHD.iManufacturerID == 13) m_iTraceGapBytes = 4; //for 190
//if (m_infoGHD.iManufacturerID == 13) m_iTraceGapBytes = 4*20;// for houston data
//iManufacturerSerial: 18-19
   m_infoGHD.iManufacturerSerial = BCD2Int(ip + 18 - 1, 4, true);

//get bytes/scan:20-22=0;// this is no use i>SD20

   m_infoGHD.iBytesPerScan = BCD2Int(ip + 20 - 1, 6);
//get si:23,bin
   if (ip[23 - 1] != 0xff) m_infoGHD.iSi = (float)ip[23 - 1] / 16 * 1000; //increment 1/16
   else m_infoGHD.iSi = -1;
//int iPolarity;        //24h:4 bit
   m_infoGHD.iPolarity =  *(ip + 24 - 1) & (unsigned char)0xf0 >> 4;

//get file type:26h(Record Type)
   /*
       216   Test record       
       416   Parallel channel test       
       616   Direct channel test       
       816   Normal record       
       116   Other   */
   m_infoGHD.iFiletype = BCD2Int(ip + 26 - 1, 1); //8 is ok
// get ltr26l,27
   float f;
   int i1;
   if (int(ip[26 - 1]& 0x0f) != int(0x0f))
   {
      i1 = BCD2Int(ip + 26 - 1, 3, false);
      //m_infoGHD.iLtr = i1*100;//why *100, yes ok,see document
      // update 2007.10
      f = i1 * 0.1 * 1.024 * 1000; //increment 1.024 ,bcd:00.0
      m_infoGHD.iLtr = f;
   }
   else m_infoGHD.iLtr = -1;

//get scan types:28
   m_infoGHD.iScanTypes = BCD2Int(m_pcBuf + 28 - 1, 2, true);
//get (Chan Sets/Scan Type)  max ch sets:29

   if (ip[29 - 1] != 0xff) m_infoGHD.iMaxChSets = BCD2Int(ip + 29 - 1, 2, true);
   else m_infoGHD.iMaxChSets = -1;

//get Skew blocks:30
   if (ip[30 - 1] != 0xff) m_infoGHD.iSkew = BCD2Int(ip + 30 - 1, 2, true);
   else m_infoGHD.iSkew = -1;
//get extended header:31
   if (ip[31 - 1] != 0xff) m_infoGHD.iExtended = BCD2Int(ip + 31 - 1, 2, true);
   else m_infoGHD.iExtended = -1;

//get external header:32
   if (ip[32 - 1] != 0xff) m_infoGHD.iExternal = BCD2Int(ip + 32 - 1, 2, true);
   else m_infoGHD.iExternal = -1;
   qDebug() << "end of GH1";
   return 0;
}
int QSegdRead::decodeGH3(unsigned char *p)
{


   qDebug() << " GH3 start";

   if (m_infoGHD.iVersion < 3) //#3 //gor version <3
   {
      //shotline: integer :4-6:,7-8
      m_infoGHD.fShotLine = getSignedFloat3_2(p + 4 - 1);
      //shotpoint:integer :9-11,12-13
      m_infoGHD.fShotPoint = getSignedFloat3_2(p + 9 - 1);
      //shotpoint index: integer :14
      m_infoGHD.iShotIndex = *(p + 14 - 1);
      //int iPhaseControl;    //15
      m_infoGHD.iPhaseControl = *(p + 15 - 1);
      //	int iTypeVibrator ;   //16
      m_infoGHD.iTypeVibrator = *(p + 16 - 1);
      //	int iPhaseAngle;      //17-18 : signed short
      m_infoGHD.iPhaseAngle = getSignedShort(p + 17 - 1);
      //	int iSourceSetNO;     //20;
      m_infoGHD.iSourceSetNO = *(p + 20 - 1);
   }
   else // version3:
   {
//get Date time: //1-8 int8
      m_infoGHD.iTime = getInt8(p + 1 - 1);
//get ShotLen: //9-16 int 8
      m_infoGHD.iShotLen = getInt8(p + 9 - 1);
//get DataLen: //17-24 int8
      m_infoGHD.iDataLen = getInt8(p + 17 - 1);
//get HeadLen: //25-28 int4
      m_infoGHD.iHeadLen = getInt(p + 25 - 1);
//get ExRecMode: //29 int1
      m_infoGHD.iExtendedRecordMode =  p[29 - 1];
//get relativw Time Mode: //30 int1
      m_infoGHD.iRelativeTimeMode =  p[30 - 1];
   }
   qDebug() << "end of GH3";
   return 0;
}
int QSegdRead::decodeGH2(unsigned char *ip)
{

   //----------------------general header #2:-----------------------------------------------------
// check:
   //if (ip[32 - 1] != 2) qDebug() << "Error this is not GH2" << ip[32 - 1];
   qDebug() << " GH2 start ";
   //#2 segd version:11-12
   m_infoGHD.iVersion = ip[11 - 1];
   m_infoGHD.iSubVersion = ip[12 - 1];

//file number: #2:1-3,bin
   if (m_infoGHD.iFileNo == -1) m_infoGHD.iFileNo = getInt3(ip);
//Extended Channel Sets/Scan Type  #2:4-5,bin
   if (m_infoGHD.iMaxChSets  == -1) m_infoGHD.iMaxChSets = getShort(ip + 4 - 1);
// version2===============================:
   if (m_infoGHD.iVersion < 3)
   {
//get extended header: #2:6-7 bin 2
      if (m_infoGHD.iExtended  == -1) m_infoGHD.iExtended = getShort(ip + 6 - 1);
//get external  header: #2:8-9 bin2
      if (m_infoGHD.iExternal  == -1) m_infoGHD.iExternal = getShort(ip + 8 - 1);
//get trailer     : #2:13-14 :bin 2
      m_infoGHD.iTrailer = getShort(ip + 13 - 1);
//ltr:          #2 15-17,bin3
      if (m_infoGHD.iLtr == -1) //	m_infoGHD.iLtr = GetInt(m_pcBuf+32 +14-1);//	m_infoGHD.iLtr = m_infoGHD.iLtr & 0x00ffffff;
         m_infoGHD.iLtr = getInt3(ip + 15 - 1);
   }
// for version3.x:================================
   else
   {
//get extended header: #2:6-8 bin 3
      if (m_infoGHD.iExtended  == -1) m_infoGHD.iExtended = getInt3(ip + 6 - 1);
//get sk: #2:9-10 bin2
      if (m_infoGHD.iSkew  == -1) m_infoGHD.iSkew = getShort(ip + 9 - 1);
//get trailer     : #2:13-16 :int 3
      m_infoGHD.iTrailer = getInt3(ip + 13 - 1);
//ltr:         #2 17-20,bin4
      if (m_infoGHD.iLtr == -1) //	m_infoGHD.iLtr = GetInt(m_pcBuf+32 +14-1);//	m_infoGHD.iLtr = m_infoGHD.iLtr & 0x00ffffff;
         m_infoGHD.iLtr = getInt(ip + 17 - 1);
//shotGroup  #2:21-22 bin2
      if (m_infoGHD.iExternal  == -1) m_infoGHD.iShotGroup = getShort(ip + 21 - 1);
//addGHD  #2:23-24 bin2
      if (m_infoGHD.iAddGHD  == -1) m_infoGHD.iAddGHD = getShort(ip + 23 - 1);
//si:         #2 25-27,bin 3
      if (m_infoGHD.iSi == -1) //	m_infoGHD.iLtr = GetInt(m_pcBuf+32 +14-1);//	m_infoGHD.iLtr = m_infoGHD.iLtr & 0x00ffffff;
         m_infoGHD.iSi = getInt3(ip + 25 - 1);
//get external  header: #2:28-29 bin2
      if (m_infoGHD.iExternal  == -1) m_infoGHD.iExternal = getShort(ip + 28 - 1);
       
   }
   qDebug() << "end of GH2";
   return 0;
}
int QSegdRead::decodeChannelSets(unsigned char *p)
{
   int j, num;
   float mp;
   int begin;
   int sLen;
   //get the for para: scan type,channel sets,channel number,channel type;
   int chsetsBytes, loopEnd;
   int trsall, aux, trs, i;

   sLen = m_iSCANB_LEN;
   //begin = sLen + m_infoGHD.iAddGHD * sLen;
   chsetsBytes = m_infoGHD.iScanTypes * m_infoGHD.iMaxChSets * sLen;
   //loopEnd = begin + chsetsBytes;
   begin = 0;
   loopEnd = chsetsBytes;
   qDebug() << "decode channel sets begin,slen=" << sLen;

   j = 0;
   trs = 0;
   aux = 0;

   for (i = begin; i < loopEnd; i = i + sLen)
   {
      if (m_infoGHD.iVersion < 3)
      {

//scan type           1:bcd
         m_pinfoSHD[j].iScanTypeNumber = BCD2Int(p + j * sLen, 2, true);
//channel sets number 2:bcd FF //27,28 INT2,
         if ((*(p + j * sLen + 2 - 1)) != 0xff )
             m_pinfoSHD[j].iChSetNumber = BCD2Int(p + j * sLen + 2 - 1, 2, true);
         else//27,28 INT2,
             m_pinfoSHD[j].iChSetNumber = getShort(p + j * sLen + 27 - 1);
         
           qDebug() << "in chset   loop ,scan,set=" <<m_pinfoSHD[j].iScanTypeNumber<<  m_pinfoSHD[j].iChSetNumber;
//int iStartTime;      //3-4: bin start time
         m_pinfoSHD[j].iStartTime = getShort(p + j * sLen + 3 - 1) * 2; //increment 2//int iEndTime;
//5-6 bin end time
         m_pinfoSHD[j].iEndTime = getShort(p + j * sLen + 5 - 1) * 2; //increment 2
 //* mp//7-8 ; 7:-3~-10,8:s,4~-2;
         mp = GetMP(p + j * sLen + 7 - 1);
         m_pinfoSHD[j].mp = mp;
         if (m_pinfoSHD[j].iScanTypeNumber == 0 ||
             m_pinfoSHD[j].iChSetNumber == 0) break;

 //channel numbers 9-10
         m_pinfoSHD[j].iChs = BCD2Int(p + j * sLen + 9 - 1, 4, true);
//channel type 11h
         m_pinfoSHD[j].iChType = BCD2Int(p + j * sLen + 11 - 1, 1,true); //1:seis9:aux

         qDebug() << "in chset   loop " << j << m_pinfoSHD[j].iChSetNumber << m_pinfoSHD[j].iChType;


         //int iGain;           //12 low;bin
         m_pinfoSHD[j].iGain = *(p + j * sLen + 12 - 1) & (unsigned char)0x0f;
         //int iAF;             //13-14;bcd; Alias Filter Frequency
         m_pinfoSHD[j].iAF = BCD2Int(p + j * sLen + 13 - 1, 4);
         //int iAS;             //15-16;bcd; Alias Filter Slope
         m_pinfoSHD[j].iAS = BCD2Int(p + j * sLen + 15 - 1, 4);
         //int iLC;             //17-18;bcd; Low Cut Filter
         m_pinfoSHD[j].iLC = BCD2Int(p + j * sLen + 17 - 1, 4);
         //int iLS;             //19-20;bcd; Low Cut Filter Slope
         m_pinfoSHD[j].iLS = BCD2Int(p + j * sLen + 19 - 1, 4);
         //int iNF1;             //21-22;bcd; First Notch Filter
         m_pinfoSHD[j].iNF1 = BCD2Int(p + j * sLen + 21 - 1, 4);
         //int iNF2;             //23-24;bcd; Second Notch Filter
         m_pinfoSHD[j].iNF2 = BCD2Int(p + j * sLen + 23 - 1, 4);
         // iNF3;             //25-26;bcd; Third Notch Filter
         m_pinfoSHD[j].iNF3 = BCD2Int(p + j * sLen + 25 - 1, 4);  
         //27-28 ex chsets number 
          //iEFH; 29H  int1/2; Extended Header flag
         m_pinfoSHD[j].iEFH = getInt1H(p + j * sLen + 29 - 1);
         //iTHE; 29L   int1/2; Trace header extensions 
         m_pinfoSHD[j].iTHE = getInt1L(p + j * sLen + 29 - 1);

         //intint iVS;              //30;bin; Vertical Stack
         m_pinfoSHD[j].iVS = *(p + j * sLen + 30 - 1);
         //int iCAB;             //31;bin; Streamer No.
         m_pinfoSHD[j].iCAB = *(p + j * sLen + 31 - 1);
         //int iARY;             //32;bin; Array Forming
         m_pinfoSHD[j].iARY = *(p + j * sLen + 32 - 1);
      }
      else
      { //version 3:
//#30
//scan type           1:bcd
         m_pinfoSHD[j].iScanTypeNumber = BCD2Int(p + j * sLen + 1 - 1, 2, true);
         //channel sets number 2-3:int 3
         m_pinfoSHD[j].iChSetNumber = getShort(p + j * sLen + 2 - 1);
         //Channel type Identification 4:int1
         num = p[j * sLen + 4 - 1];
         if (num == 0x10)  //seis
            m_pinfoSHD[j].iChType = 1; //seis
         else m_pinfoSHD[j].iChType = 0; //aux

             if (m_pinfoSHD[j].iScanTypeNumber == 0 ||
             m_pinfoSHD[j].iChSetNumber == 0) break;


         //int iStartTime;      //5-8: bin start time (microsecond)
         m_pinfoSHD[j].iStartTime = getInt(p + j * sLen + 5 - 1);
         //end time   9-12 bin 4
         m_pinfoSHD[j].iEndTime = getInt(p + j * sLen + 9 - 1);
         //samples  13-16 bin4
         m_pinfoSHD[j].iSamples = getInt(p + j * sLen + 13 - 1);
         m_iSamples = m_pinfoSHD[j].iSamples;
         //Sample descale multiplication factor  17-20 Float 4
         m_pinfoSHD[j].fDeScale = getFloat(p + j * sLen + 17 - 1);
         //* mp//7-8 ; 7:-3~-10,8:s,4~-2;
#if 0
         mp = GetMP(p + j * sLen + 7 - 1);
         m_pinfoSHD[j].mp = mp;
         if (m_pinfoSHD[j].iScanTypeNumber == 0 ||
             m_pinfoSHD[j].iChSetNumber == 0) break;
#endif

         //channel numbers  21-23 INT3
         m_pinfoSHD[j].iChs = getInt3(p + j * sLen + 21 - 1);
         //SI 24-26 INT3
         m_pinfoSHD[j].iSi = getInt3(p + j * sLen + 24 - 1);
         //iARY; 27  int1; Array Forming
         m_pinfoSHD[j].iARY = *(p + j * sLen + 27 - 1);
         //iTHE; 28  int1; Trace header extensions
         m_pinfoSHD[j].iTHE = *(p + j * sLen + 28 - 1);
        
         //iEFH; 29H  int1; Extended Header flag
         m_pinfoSHD[j].iEFH = getInt1H(p + j * sLen + 29 - 1);
         //iGain; 29L
         m_pinfoSHD[j].iGain = getInt1L(p + j * sLen + 29 - 1);
         // iVS;   30 int1   Vertical Stack
         m_pinfoSHD[j].iVS = *(p + j * sLen + 30 - 1);
         //iCAB;   31  int1      Streamer cable number.
         m_pinfoSHD[j].iCAB = *(p + j * sLen + 31 - 1);

//#31:
// iAF;  33-36  float4, Alias filter frequency
         m_pinfoSHD[j].iAF = getFloat(p + j * sLen + 33 - 1);
         //int iLC;  37-40 float4; Low Cut Filter
         m_pinfoSHD[j].iLC = getFloat(p + j * sLen + 37 - 1);
         //int iAS;  41-44  float4; Alias Filter Slope
         m_pinfoSHD[j].iAS = getFloat(p + j * sLen + 40 - 1);
         //int iLS;  45-48  float4;     Low Cut Filter Slope
         m_pinfoSHD[j].iLS = getFloat(p + j * sLen + 45 - 1);
         //int iNF1; 49-52  float4; First Notch Filter
         m_pinfoSHD[j].iNF1 = getFloat(p + j * sLen + 49 - 1);
         //int iNF2; 53-56  float4;  Second Notch Filter
         m_pinfoSHD[j].iNF2 = getFloat(p + j * sLen + 53 - 1);
         // iNF3;    57-60  float4;  Third Notch Filter
         m_pinfoSHD[j].iNF3 = getFloat(p + j * sLen + 57 - 1);
//#32:
// FDL;    65-68  float4;  Filter delay
         m_pinfoSHD[j].iFDL = getFloat(p + j * sLen + 65 - 1);
         // DSC;    69-97 :27  float4;  Description
         memcpy(m_pinfoSHD[j].cDSC, p + j * sLen + 69 - 1, 27);
         

      } //end of i
      if (m_pinfoSHD[j].iChType == 1) trs += m_pinfoSHD[j].iChs;
      else aux += m_pinfoSHD[j].iChs;

      if(j == 0) m_iExHDs = m_pinfoSHD[j].iTHE;
      
      j++;// channel counter,i bytes counter 
   } //for i
   m_iSHDs = j;
   trsall = trs + aux;

   m_iTrs = trs;
   m_iAux = aux;
   m_iTrsAll = trsall;
   qDebug() << " end of decode Chstes trs,aux = " << trs << aux << m_iExHDs;

   return trsall;
}

/*======================================================================
                   GetFile Infor
work:
    1:read header 
    2: fill m_infoGHD
    3: fill m_infoSHD
    4: is segd?? and set m_iLtr,m_iSI,m_iFormat;
    5: set m_iSHDs;

@return
    0 eof
    -1 read err;
    -2 not a segd;
    >0 ok bytes of the block
*/
int QSegdRead::GetFileInfo()
{
   int  iret, bytes;
   int i, ip, id;
//	m_iTrsInFile = 0;
   QString str;
   // char *c,*c1;
   bytes = m_iHeadBytes;
   // qDebug() << "bytes in fileinfo1 = " <<m_pPre<<m_pThis<<m_pNext <<bytes;
   if (m_bTif8)
   { // not the 1st time enter
     //qDebug() << "seek in file info = "<<m_pNext;
      m_tp->seek(m_pNext);
      bytes = 24;
      iret = m_tp->read(m_pcBuf, bytes);
      //qDebug() << "read ret = "<< iret<<bytes;
      if (iret <= 0)
      {
         return -1;
      }
      m_iRFlag = getInt(m_pcBuf + 4);
      m_pPre = getInt64(m_pcBuf + 8);
      m_pThis = m_pNext;
      m_pNext = getInt64(m_pcBuf + 16);
      bytes = m_pNext - m_pThis;
      if (m_iRFlag != 0)
      { //file mark
         m_strStatus = "DEOF";
         qDebug() << "two file mark in readRecord" << m_icShot << m_ic << m_ica;
         return -1;
      }

   }
   else
   {
       clearBuf();
   }
   // maybe 1st
   iret = m_tp->read(m_pcBuf, bytes);
   //qDebug() << "read tape =" << iret;
   if (iret == 0) return iret;
   else if (iret < 0) return -1;
   else if (iret > 0)
   {
      if (!strncmp((char *)m_pcBuf, (char *)m_tif8, 4))
      { // first time enter:
         m_bTif8 = true;
         qDebug() << "==========================this is tif8===================";
         m_iRFlag = getInt(m_pcBuf + 4);
         m_pPre = getInt64(m_pcBuf + 8);
         m_pThis = m_pNext;
         m_pNext = getInt64(m_pcBuf + 16);
         // qDebug() << "bytes in fileinfo = " <<m_pPre<<m_pThis<<m_pNext;

         memcpy(m_pcBuf, m_pcBuf + 24, bytes - 24);
         m_iTraceGapBytes = 24;
      }
//gh1:
      decodeGH1(m_pcBuf);
      if (MY_DEBUG) 
      {
          qDebug() << "-----after decode GH1---:";
          prGH();
      }

      if (!isSegd()) return -2;
      //gh2:
      if (m_infoGHD.iAddGHD != 0) // -1 >0 ok
         decodeGH2(m_pcBuf + GHB_LEN);
      
      //gh3
      if (MY_DEBUG) 
      {
          qDebug() << "-----after decode GH2---:";
          prGH();
      }
      
      if ((m_infoGHD.iVersion < 3) && (m_infoGHD.iAddGHD >= 2))  decodeGH3(m_pcBuf + 2 * GHB_LEN); //
      else //version 3:
      {
         for (i = 0; i < m_infoGHD.iAddGHD - 2; i++)
         {
            ip =   GHB_LEN * 2 + i * GHB_LEN;
            id = *(m_pcBuf + ip + 32 - 1);
            switch (id)
            {
            case 0x03:
               decodeGH3(m_pcBuf + ip); //
               break;
            default:
               qDebug("-----decode GHD idx, no.,id = %d, %d, %X", i, i + 3, id);
               break;

            }
         }
      } // end of GHD
      //qDebug() << "33333333333333333333";
      
      qDebug() << "-----after decode GH3---:";
      prGH();
 //channel sets:
      m_iVersion = m_infoGHD.iVersion;
      if (m_infoGHD.iVersion >= 3) m_iSCANB_LEN = 96;
      else
          m_iSCANB_LEN = 32;


      ip =   GHB_LEN + m_infoGHD.iAddGHD * GHB_LEN;
      qDebug() << "-------start decode channelsets:  ip = " << ip;
      i = decodeChannelSets(m_pcBuf + ip);
      prChSets();

//============================ end of genHeader chsets ========================================
//--------------------------------------------is segd format?? check---------------
//is segd format:


      m_fFormatBytes = float(m_infoGHD.iFormatCode - m_infoGHD.iFormatCode / 10 * 10) / 2.0;

//----------some important varies
      if (m_infoGHD.iSi <= 0) m_infoGHD.iSi = m_iOSi;
      if (m_infoGHD.iLtr <= 0) m_infoGHD.iLtr = m_iOLtr;
      
      m_iLTR = m_infoGHD.iLtr;
      if (m_iVersion >=3)  m_iLTR = m_infoGHD.iLtr/1000;
      m_iSI = m_infoGHD.iSi;
      m_iFormat = m_infoGHD.iFormatCode;
      //if (m_iSI != 0) m_iSamples = m_iLTR * 1000 / m_iSI;
//----------------------------------------------------------
//qDebug() << "in fileinf";
// the scan type and the channel sets header
//==============================================scan type and the channel sets header===================

      m_ic = 0;
      m_ica = 0;
      m_icTrShot =0;

      int headBytes, scanBytes;
      headBytes = (1	+ m_infoGHD.iAddGHD
                   // + m_infoGHD.iScanTypes * m_infoGHD.iMaxChSets
                   + m_infoGHD.iSkew
                   + m_infoGHD.iExtended
                   + m_infoGHD.iExternal) * GHB_LEN;
      scanBytes =  m_infoGHD.iScanTypes * m_infoGHD.iMaxChSets * m_iSCANB_LEN;

      // cjh 201610, the gap not added to fileHeader,but to trace data;
      m_iHeadBytes = headBytes + scanBytes; // + m_iTraceGapBytes;
      m_iTrailBytes = m_infoGHD.iTrailer * GHB_LEN;
      if (m_infoGHD.iHeadLen !=0)  m_iHeadBytes = m_infoGHD.iHeadLen;

      //getExHD();//next trace hd 10;

      //setBytes();//getinfo
      get1stTrPos();


      qDebug() << "head Bytes = " << headBytes;
      qDebug() << "scan Bytes = " << scanBytes;
      qDebug() << "m_iHeadBytes = " << m_iHeadBytes;
      qDebug() << "m_iDataHDBytes = " << m_iDataHDBytes;
      qDebug() << "m_iDataBytes = " << m_iDataBytes;
      qDebug() << "m_iTraceBytes = " << m_iTraceBytes;
      qDebug() << QString("hex = %1").arg(m_iHeadBytes, 10, 16);
      qDebug() << "Gap Bytes of trace   = " << m_iTraceGapBytes;
      qDebug() << "trail Bytes    = " << m_iTrailBytes;
      qDebug() << "m_fFormatBytes = " << m_fFormatBytes;
      qDebug("File Infor end ==================================================\n");

   } //end of iret>0;


   return iret;
}
int QSegdRead::get1stTrPos()
{
     unsigned char *p;
     int iSamples;
     iSamples = 0;
    if (m_iVersion < 3 )  
    {
        p = m_pcBuf + m_iHeadBytes;
            // extended header :10 ;int1
            if (m_iExHDs == 0)  
                m_iExHDs = *(p + 10 - 1); //10
            if (m_iExHDs >0)  
                iSamples = getInt3(p + 20 + 8 - 1);          
            
    }
    #if 0
    else
    {
        iSamples = getInt(p + 20+ 25 - 1);
    }
    #endif



    if(iSamples > 0) m_iSamples = iSamples;
    qDebug() << "get1stTrPos exHD,m_iSamples,iSamples = " << m_iExHDs << m_iSamples<<iSamples;

    setBytes();// in get1stTrShot
    m_tp->seek(m_iOffsetBytes + m_iHeadBytes); // repoiter:
    qDebug() <<"seek first trace:offset, hdBytes =" << m_iOffsetBytes << m_iHeadBytes;
    
    return 0;

}
int QSegdRead::curChSetsIdx()
{
   int i, chsets, iret,ic;
   chsets = m_infoGHD.iMaxChSets; // number of channel sets for each scan type
   iret = -1;
   ic = 0;
   for (i = 0; i < chsets; i++)
   {
      ic += m_pinfoSHD[i].iChs; //channels in sets
      if (m_icTrShot < ic)
      { 
          iret  = i;
          break;
      }   
   }
   return iret;
}
int QSegdRead::curScan()
{
   int i,iret,chsets;
   chsets = m_infoGHD.iMaxChSets; // number of channel sets for each scan type
   i = curChSetsIdx();
   if (i >=0 && i < chsets) 
   {
      
       iret = m_pinfoSHD[i].iScanTypeNumber;
       // qDebug() <<"get Scan" <<  i << m_pinfoSHD[i].iScanTypeNumber<<iret;
   }
   else
       iret = -1;

   return iret;

}
int QSegdRead::curChSets()
{
   int i,iret,chsets;
   chsets = m_infoGHD.iMaxChSets; // number of channel sets for each scan type
   i = curChSetsIdx();
   if (i >=0 && i < chsets) 
   {
      
       iret = m_pinfoSHD[i].iChSetNumber;
       // qDebug() <<"get ch number" <<  i << m_pinfoSHD[i].iChSetNumber<<iret;
   }
   else
       iret = -1;

   return iret;

}
int QSegdRead::setBytes()
{
   int   chsets, ltr, i,iExHDs,iSamples;
   chsets = m_infoGHD.iMaxChSets; // number of channel sets for each scan type
 
   ltr = 0;
   iSamples = 0;
   i = curChSetsIdx();
   if (i >=0 && i < chsets)  
   {
         iExHDs = m_pinfoSHD[i].iTHE;
         if (m_pinfoSHD[i].iEndTime != 0) ltr = m_pinfoSHD[i].iEndTime;
         if (m_pinfoSHD[i].mp != 0) m_fMP = m_pinfoSHD[i].mp;
         if (m_pinfoSHD[i].fDeScale != 0) m_fDeScale = m_pinfoSHD[i].fDeScale;
         if (m_pinfoSHD[i].iSamples != 0) iSamples = m_pinfoSHD[i].iSamples;
         if (m_pinfoSHD[i].iSi != 0) m_iSI= m_pinfoSHD[i].iSi;
         if (m_pinfoSHD[i].iChType!= 0) m_iChType = m_pinfoSHD[i].iChType;
         m_fDeScale = m_pinfoSHD[i].fDeScale;  //v30
   }
   else
   {
       qDebug() << "Wrong chset idx=========== " << i;
       return -1;
   }
         
   if (iExHDs !=0)  m_iExHDs = iExHDs;//
   if (iSamples !=0)  m_iSamples = iSamples;//

// add 2008.6.7 for func = 13 ,have one extra sample(4 byte)
// for Sercel 2008.6.27 for m_iTraceGapBytes
   if (MY_DEBUG)  
        qDebug() << "ltr ,m_iSamples,iSample,si = " << ltr << m_iSamples << iSamples << m_infoGHD.iSi;
   if (ltr == 0)  ltr = m_infoGHD.iLtr;
 
   int sam;
   sam = 0;
   if (m_iSamples != 0) 
   {
       sam = m_iSamples;
   }
   else 
   {
       sam = ltr * 1000 / m_infoGHD.iSi ;
   }

   if (sam >1000000) sam = sam /1000;// ltr is microsecond

   m_iSamples = sam;

   m_iDataHDBytes =  20 + m_iExHDs * 32;
   m_iDataBytes = sam * m_fFormatBytes; //+ m_iTraceGapBytes;//201610 deleted
   m_iTraceBytes = m_iDataHDBytes + m_iDataBytes + m_iTraceGapBytes;
   if (MY_DEBUG) 
       qDebug() << "setBytes() : ltr,m_iDataHDBytes,sam,si,exHDs" << ltr <<  m_iDataHDBytes << sam << m_infoGHD.iSi <<m_iExHDs ;
   return 0;
}
bool QSegdRead::isSegd()
{
   int flag = -1;
   // check format code:
   for (int i = 0; i < m_iFormatNum; i++)
   {
      if (m_infoGHD.iFormatCode == m_piFormat[i]) flag = 1;
   }

   if (flag == -1) return false; //not a segd,format code is not in range
   else return true;
}
int QSegdRead::GetFileInfo1()
{
   int i, iret, i1, bytes;
//	m_iTrsInFile = 0;
   QString str;
   // char *c,*c1;
   bytes = m_iHeadBytes;
   // qDebug() << "bytes in fileinfo1 = " <<m_pPre<<m_pThis<<m_pNext <<bytes;
   if (m_bTif8)
   { // not the 1st time enter
     //qDebug() << "seek in file info = "<<m_pNext;
      m_tp->seek(m_pNext);
      bytes = 24;
      iret = m_tp->read(m_pcBuf, bytes);
      //qDebug() << "read ret = "<< iret<<bytes;
      if (iret <= 0)
      {
         return -1;
      }
      m_iRFlag = getInt(m_pcBuf + 4);
      m_pPre = getInt64(m_pcBuf + 8);
      m_pThis = m_pNext;
      m_pNext = getInt64(m_pcBuf + 16);
      bytes = m_pNext - m_pThis;
      if (m_iRFlag != 0)
      { //file mark
         m_strStatus = "DEOF";
         qDebug() << "two file mark in readRecord" << m_icShot << m_ic << m_ica;
         return -1;
      }

   }
   // maybe 1st
   iret = m_tp->read(m_pcBuf, bytes);
   //qDebug() << "read tape =" << iret;
   if (iret == 0) return iret;
   else if (iret < 0) return -1;
   else if (iret > 0)
   {
      if (!strncmp((char *)m_pcBuf, (char *)m_tif8, 4))
      { // first time enter:
         m_bTif8 = true;
         qDebug() << "==========================this is tif8===================";
         m_iRFlag = getInt(m_pcBuf + 4);
         m_pPre = getInt64(m_pcBuf + 8);
         m_pThis = m_pNext;
         m_pNext = getInt64(m_pcBuf + 16);
         // qDebug() << "bytes in fileinfo = " <<m_pPre<<m_pThis<<m_pNext;

         memcpy(m_pcBuf, m_pcBuf + 24, bytes - 24);
         m_iTraceGapBytes = 24;
      }
      //========================================gen Header====================
//get file number: //1-2
      if (m_pcBuf[0] != 0xff) m_infoGHD.iFileNo = BCD2Int(m_pcBuf, 4);
      else m_infoGHD.iFileNo = -1;

//get format:3-4
      m_infoGHD.iFormatCode = BCD2Int(m_pcBuf + 3 - 1, 4);

//get additional gen header:12h
      m_infoGHD.iAddGHD = BCD2Int(m_pcBuf + 12 - 1, 1);
//		iManufacturerID: 17 ,add 2008.6.7
      m_infoGHD.iManufacturerID = BCD2Int(m_pcBuf + 17 - 1, 2);
// add 2008.6:
// cjh this is not a formular:? but we flloow it 201610
      if (m_infoGHD.iManufacturerID == 13) m_iTraceGapBytes = 4; //for 190
                                                                 //if (m_infoGHD.iManufacturerID == 13) m_iTraceGapBytes = 4*20;// for houston data

//get bytes/scan:20-22=0;
      m_infoGHD.iBytesPerScan = BCD2Int(m_pcBuf + 20 - 1, 6);
//get si:23,bin
      m_infoGHD.iSi = (float)m_pcBuf[23 - 1] / 16 * 1000; //increment 1/16
//int iPolarity;        //24h:4 bit
      m_infoGHD.iPolarity =  *(m_pcBuf + 24 - 1) & (unsigned char)0xf0 >> 4;

//get file type:26h(Record Type)
      m_infoGHD.iFiletype = BCD2Int(m_pcBuf + 26 - 1, 1);
// get ltr26l,27
      float f;
      if (int(m_pcBuf[26 - 1]& 0x0f) != int(0x0f))
      {
         i1 = BCD2Int(m_pcBuf + 26 - 1, 3, false);
         //m_infoGHD.iLtr = i1*100;//why *100, yes ok,see document
         // update 2007.10
         f = i1 * 0.1 * 1.024 * 1000; //increment 1.024 ,bcd:00.0
         m_infoGHD.iLtr = f;
      }
      else m_infoGHD.iLtr = -1;

//get scan types:28
      m_infoGHD.iScanTypes = BCD2Int(m_pcBuf + 28 - 1, 2, true);
//get (Chan Sets/Scan Type)  max ch sets:29

      if (m_pcBuf[29 - 1] != 0xff) m_infoGHD.iMaxChSets = BCD2Int(m_pcBuf + 29 - 1, 2, true);
      else m_infoGHD.iMaxChSets = -1;

//get Skew blocks:30
      m_infoGHD.iSkew = BCD2Int(m_pcBuf + 30 - 1, 2, true);
//get extended header:31
      if (m_pcBuf[31 - 1] != 0xff) m_infoGHD.iExtended = BCD2Int(m_pcBuf + 31 - 1, 2, true);
      else m_infoGHD.iExtended = -1;

//get external header:32
      if (m_pcBuf[32 - 1] != 0xff) m_infoGHD.iExternal = BCD2Int(m_pcBuf + 32 - 1, 2, true);
      else m_infoGHD.iExternal = -1;

//============================ end of genHeader========================================
//--------------------------------------------is segd format?? check---------------
//is segd format??
      int flag = -1;
      // check format code:
      for (i = 0; i < m_iFormatNum; i++)
      {
         if (m_infoGHD.iFormatCode == m_piFormat[i]) flag = 1;
      }

      if (flag == -1) return -2; //not a segd,format code is not in range
      m_fFormatBytes = float(m_infoGHD.iFormatCode - m_infoGHD.iFormatCode / 10 * 10) / 2.0;
      //============================  genHeader 2========================================
//----------------------general header #2:-----------------------------------------------------
//  general header #2:

      //file number: #2:1-3,bin
      unsigned char *p;
      p = m_pcBuf + 32;
      if (m_infoGHD.iAddGHD > 0)
      {

         if (m_infoGHD.iFileNo == -1)
         {
            //m_infoGHD.iFileNo = BCD2Int(m_pcBuf+32,6,true);
            m_infoGHD.iFileNo = getInt3(p);
         }


         //Extended Channel Sets/Scan Type  #2:4-5,bin
         if (m_infoGHD.iMaxChSets  == -1)
         {
            m_infoGHD.iMaxChSets = getShort(p + 4 - 1);
         }

//get extended header: #2:6-7 bin
         if (m_infoGHD.iExtended  == -1)
         {
            m_infoGHD.iExtended = getShort(p + 6 - 1);
         }

//get external  header: #2:8-9 bin
         if (m_infoGHD.iExternal  == -1)
         {
            m_infoGHD.iExternal = getShort(p + 8 - 1);
         }


//get segd version: #2:11-12 :
         m_infoGHD.iVersion = *(p + 11 - 1);

//get trailer     : #2:13-14 :
         m_infoGHD.iTrailer = getShort(p + 13 - 1);

         //ltr:          #2 15-17,bin
         if (m_infoGHD.iLtr == -1)
         {
            //	m_infoGHD.iLtr = GetInt(m_pcBuf+32 +14-1);
            //	m_infoGHD.iLtr = m_infoGHD.iLtr & 0x00ffffff;
            m_infoGHD.iLtr = getInt3(p + 15 - 1);
         }

      }
      //============================  genHeader 3========================================
//-------------------get shot line point in header #3--------------
      p = m_pcBuf + 32 * 2;

      if (m_infoGHD.iAddGHD > 1) //#3
      {

         //shotline: integer :4-6:,7-8
         m_infoGHD.fShotLine = getSignedFloat3_2(p + 4 - 1);
         //shotpoint:integer :9-11,12-13
         m_infoGHD.fShotPoint = getSignedFloat3_2(p + 9 - 1);
         //shotpoint index: integer :14
         m_infoGHD.iShotIndex = *(p + 14 - 1);
         //int iPhaseControl;    //15
         m_infoGHD.iPhaseControl = *(p + 15 - 1);
         //	int iTypeVibrator ;   //16
         m_infoGHD.iTypeVibrator = *(p + 16 - 1);
         //	int iPhaseAngle;      //17-18 : signed short
         m_infoGHD.iPhaseAngle = getSignedShort(p + 17 - 1);
         //	int iSourceSetNO;     //20;
         m_infoGHD.iSourceSetNO = *(p + 20 - 1);

      }
      //============================  end of genHeader 3========================================
//--------------------------------
      if (m_infoGHD.iSi <= 0) m_infoGHD.iSi = m_iOSi;
      if (m_infoGHD.iLtr <= 0) m_infoGHD.iLtr = m_iOLtr;

      m_iLTR = m_infoGHD.iLtr;// getinfor1
      m_iSI = m_infoGHD.iSi;
      m_iFormat = m_infoGHD.iFormatCode;
      // if (m_iSI != 0) m_iSamples = m_iLTR * 1000 / m_iSI;
//----------------------------------------------------------
//qDebug() << "in fileinf";
// the scan type and the channel sets header
//==============================================scan type and the channel sets header===================
      int j;
      float mp;
      int begin;
      //get the for para: scan type,channel sets,channel number,channel type;
      int chsetsBytes, loopEnd;
      int trsall, aux, trs;

      begin = 32 + m_infoGHD.iAddGHD * 32;
      chsetsBytes = m_infoGHD.iScanTypes * m_infoGHD.iMaxChSets * 32;
      loopEnd = begin + chsetsBytes;

      p =  m_pcBuf + begin;
      j = 0;
      trs = 0;
      aux = 0;

      for (i = begin; i < loopEnd; i = i + 32)
      {
         //scan type           1:bcd
         m_pinfoSHD[j].iScanTypeNumber = BCD2Int(p + j * 32, 2, true);
         //channel sets number 2:bcd
         m_pinfoSHD[j].iChSetNumber = BCD2Int(p + j * 32 + 2 - 1, 2, true);
         //int iStartTime;      //3-4: bin start time
         m_pinfoSHD[j].iStartTime = getShort(p + j * 32 + 3 - 1) * 2; //increment 2
                                                                      //int iEndTime;        //5-6 bin end time
         m_pinfoSHD[j].iEndTime = getShort(p + j * 32 + 5 - 1) * 2; //increment 2

         //* mp//7-8 ; 7:-3~-10,8:s,4~-2;
         mp = GetMP(p + j * 32 + 7 - 1);
         m_pinfoSHD[j].mp = mp;

         if (m_pinfoSHD[j].iScanTypeNumber == 0 ||
             m_pinfoSHD[j].iChSetNumber == 0)
         {
            //j++;
            break;
         }
         //channel numbers 9-10
         m_pinfoSHD[j].iChs = BCD2Int(m_pcBuf + begin + j * 32 + 8, 4, true);
         //channel type 11h
         m_pinfoSHD[j].iChType = BCD2Int(m_pcBuf + begin + j * 32 + 11 - 1, 1); //1:seis9:aux
         if (m_pinfoSHD[j].iChType == 1) trs += m_pinfoSHD[j].iChs;
         else aux += m_pinfoSHD[j].iChs;

         //int iGain;           //12l;bin
         m_pinfoSHD[j].iGain = *(m_pcBuf + begin + j * 32 + 12 - 1) & (unsigned char)0x0f;
         //int iAF;             //13-14;bcd; Alias Filter Frequency
         m_pinfoSHD[j].iAF = BCD2Int(m_pcBuf + begin + j * 32 + 13 - 1, 4);
         //int iAS;             //15-16;bcd; Alias Filter Slope
         m_pinfoSHD[j].iAS = BCD2Int(m_pcBuf + begin + j * 32 + 15 - 1, 4);
         //int iLC;             //17-18;bcd; Low Cut Filter
         m_pinfoSHD[j].iLC = BCD2Int(m_pcBuf + begin + j * 32 + 17 - 1, 4);
         //int iLS;             //19-20;bcd; Low Cut Filter Slope
         m_pinfoSHD[j].iLS = BCD2Int(m_pcBuf + begin + j * 32 + 19 - 1, 4);
         //int iNF1;             //21-22;bcd; First Notch Filter
         m_pinfoSHD[j].iNF1 = BCD2Int(m_pcBuf + begin + j * 32 + 21 - 1, 4);
         //int iNF2;             //23-24;bcd; Second Notch Filter
         m_pinfoSHD[j].iNF2 = BCD2Int(m_pcBuf + begin + j * 32 + 23 - 1, 4);
         // iNF3;             //25-26;bcd; Third Notch Filter
         m_pinfoSHD[j].iNF3 = BCD2Int(m_pcBuf + begin + j * 32 + 25 - 1, 4);
         //intint iVS;              //30;bin; Vertical Stack
         m_pinfoSHD[j].iVS = *(m_pcBuf + begin + j * 32 + 30 - 1);
         //int iCAB;             //31;bin; Streamer No.
         m_pinfoSHD[j].iCAB = *(m_pcBuf + begin + j * 32 + 31 - 1);
         //int iARY;             //32;bin; Array Forming
         m_pinfoSHD[j].iARY = *(m_pcBuf + begin + j * 32 + 32 - 1);

         j++;
      }
      //==============================================end of scan type and the channel sets header===================
      m_iSHDs = j;

      trsall = trs + aux;
      m_iTrs = trs;
      m_iAux = aux;
      m_iTrsAll = trsall;
      m_ic = 0;
      m_ica = 0;



      //debug:
      if (1)
      {
         qDebug("File Infor begin ==================================================\n");
         qDebug("iFileNo = %d", m_infoGHD.iFileNo);
         qDebug("iFormatCode = %d", m_infoGHD.iFormatCode);
         qDebug("iManufacturerID = %d", m_infoGHD.iManufacturerID);
         qDebug() << mapManufactory[m_infoGHD.iManufacturerID];
         qDebug("iAddGHD = %d", m_infoGHD.iAddGHD);
         qDebug("iBytesPerScan = %d", m_infoGHD.iBytesPerScan);
         qDebug("iSi = %d", m_infoGHD.iSi);
         qDebug("iFiletype = %d", m_infoGHD.iFiletype);
         qDebug("iLtr  = %d", m_infoGHD.iLtr);
         qDebug("iScanTypes = %d", m_infoGHD.iScanTypes);
         qDebug("iMaxChSets; = %d", m_infoGHD.iMaxChSets);
         qDebug("Channel sets table heads: = %d", m_iSHDs);
         qDebug("extended : = %d", m_infoGHD.iExtended);
         qDebug("external : = %d", m_infoGHD.iExternal);
//2
         qDebug("version : = %d", m_infoGHD.iVersion);
         qDebug("iTrailer : = %d", m_infoGHD.iTrailer);
//3

         qDebug("fShotLine : = %f", m_infoGHD.fShotLine);
         qDebug("fShotPoint : = %f", m_infoGHD.fShotPoint);
         qDebug("iShotIndex: = %d", m_infoGHD.iShotIndex);
         qDebug("iPhaseControl : = %d", m_infoGHD.iPhaseControl);
         qDebug("iTypeVibrator : = %d", m_infoGHD.iTypeVibrator);
         qDebug("iPhaseAngle : = %d", m_infoGHD.iPhaseAngle);
         qDebug("iSourceSetNO : = %d", m_infoGHD.iSourceSetNO);


         for (i = 0; i < m_iSHDs; i++)
         {
            qDebug(" no=%d, scantype = %d. channel sets = %d,trs = %d, time = %d,type = %d",
                   i,
                   m_pinfoSHD[i].iScanTypeNumber,
                   m_pinfoSHD[i].iChSetNumber,
                   m_pinfoSHD[i].iChs,
                   m_pinfoSHD[i].iEndTime,
                   m_pinfoSHD[i].iChType);
         }
         qDebug() << "trs,aux,alltrs =" << m_iTrs << m_iAux << m_iTrsAll;
      } //end of debug

      int headbytes;
      headbytes = (1	+ m_infoGHD.iAddGHD
                   + m_infoGHD.iScanTypes * m_infoGHD.iMaxChSets
                   + m_infoGHD.iSkew
                   + m_infoGHD.iExtended
                   + m_infoGHD.iExternal) * 32;
      // cjh 201610, the gap not added to fileHeader,but to trace data;
      m_iHeadBytes = headbytes; // + m_iTraceGapBytes;
      m_iTrailBytes = m_infoGHD.iTrailer * 32;

      qDebug() << "head Bytes = " << headbytes;
      qDebug() << QString("hex = %1").arg(m_iHeadBytes, 10, 16);
      qDebug() << "Gat Bytes of trace   = " << m_iTraceGapBytes;
      qDebug() << "trail Bytes    = " << m_iTrailBytes;
      qDebug("File Infor end ==================================================\n");

   } //end of iret>0;


   return iret;
}
//============================================================================
/**
ReadTrace

    1: getstatus;

ok:
    1:conver data to buf
    2:read next trace
eof:
    1:GetFileInfo()
    2:read a trace;
    return
eot:
    nextfile
    return 
err:
    1:skip a record
    2: invalid the trace 0
    3: read next record

@return:

    >0 samples read
    0 double eof
    -1 read header error
    -2 read first record err
    -3 not a segd
    -4: eot
    -5: skip error;
    -6: skiped

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
int QSegdRead::readShot(float *buf, int *head)
{
   int i, iby, sams, iret;
   QString str;
   int headLen;
   float *f;
   f = (float *)head;
   headLen =  SEGD_HEAD_LEN;
   sams = m_iSamples;
   i = 0;
   //sams = m_iLtr*1000/m_iSi;
   qDebug() <<  "sams =" << sams << headLen << buf;
   while (1)
   {
      iby =  readTrace(buf + sams * i, head + headLen * i);
      str = getStatus();
      //qDebug() << "In readShot loop ,i=" << i<< iby << str;
      if (str != "OK") break;
      if (iby <= 0) break;
      i++;
      if (i >= m_iTrsAll)
      {
         qDebug() << "In readShot loop >= Trs of shot=" << i << iby << str <<  m_iTrsAll;
         str = "EOF"; // this is DEV_DISK
         break; // end of this shot
      }
      //qDebug() << "read trace =" <<i << str;
   }
   // qDebug() << "read shot =" << i << str <<  head[0] << head[1];
   qDebug() << "read shot1 =" << i << str <<   f[0] << f[1] << f[21];
//
   if (str == "EOF" || str == "DEOF")
   {
      iret = GetFileInfo();
      if (iret > 0) str == "EOF"; // normal get next file info
      else if (iret == 0) // end of this file ,need next file;
      {
         iret = NextFile();
         if (iret == 0) setStatus("EOT"); // no extra file
         else if (iret > 0) setStatus("EOF"); // yes next file begin
         else setStatus("ERR"); // err
      }
      else setStatus("ERR");
   }
   else if (str == "EOT" || str == "EOTERR") // next file:
   {
      iret = NextFile();
      if (iret == 0) setStatus("EOT");
      else if (iret > 0) setStatus("EOF");
      else setStatus("ERR");
   }
   else if (str == "ERR" || str == "SERR")
   {
      setStatus("ERR");
   }
   return i;
}
int QSegdRead::readTrace(float *buf, int *head)
{
   int  sams,ir;
   QString str, str1;
   
   ReadRecord();
   str = getStatus();
   if (str == "OK")
   {
      setHeader(head);
      sams = m_iSamples;
      ir = ConvertData(buf, sams);

   }
   m_icTrShot ++; // in readTrace after  ReadRecord(); convertData setHeader 
   return ir;
}

int QSegdRead::ReadTrace(float *buf, int sams)// donot use this func
{
   int iret;
   QString str, str1;
   //int iskiped = 0;
// read a block
record:
   ReadRecord();
   m_icTrShot ++; // in ReadTrace after  ReadRecord();
 

   str = getStatus();
   //if(str == "OK" )//for tape
   //if(str == "OK" || str == "EOF")//for diskfile
   //2008.8
   //if(str == "OK" || str == "EOF"||str == "EOT")//for diskfile
   if (str == "OK") //for diskfile tif8 format
   {
      iret = ConvertData(buf, sams);
   }
   //else //if tape
   if (str == "EOF" || str == "DEOF") // no DEOF infact
   {


      // first trace of the next shot;
      //m_iHeadBytes = m_pNext - m_pThis;
      iret = GetFileInfo();
      if (m_strStatus == "DEOF") //
      {
         return 0;
      }
      m_icShot++; // end of shot
      if (iret == -1)
      {
         m_strStatus = "DEOF"; // no file:
         return -1;
      }
      else if (iret == 0)
      {
         //2008.5.12: for disk file:
         //is the file end,for read 0 byte when GetFileInfo()
         //double eof end of the tape
         //m_strStatus = "DEOF";
         iret = NextFile();
         setStatus("EOT"); //
         return iret; //==0, complete ,no file can read
                      // >0,eot;
      }
      else if (iret == -2) // not segd
      {
         m_strStatus = "NSEGD";
         return -3;
      }
      goto record;

   } //end of eof
   else if (str == "EOT" || str == "EOTERR")
   {
      //open next segd file:
      iret = NextFile();
      //m_strStatus = "EOTERR"	;
      //return -4;
      setStatus("EOT");
      return iret; //==0,complete
   }
   else if (str == "ERR" || str == "SERR")
   {
      return -5; // diskfile;
      /*
      i = m_tp->TapeFileForword(m_iIUnit,1);
      if(i !=0)//skip err
      {
          m_strStatus = "SERR";
          return -5;
      }
      //main head
      SetMainHeader();
      m_infoMHD.iTr ++;
      m_infoMHD.iValid = 0;
      iskiped = 1;
      //continue read:
      */
   }

   return sams;
}

/*
convert data

  1:Fillmainheader
  2:convert
  return 0;

*/
int QSegdRead::ConvertData(float *buf, int sams)
{
   int len, len1,ir;

   ir = SetMainHeader();
   if (ir <0)  return  ir;

//	convert
   len = m_iSamples;
   if (len == 0)  
       if (m_iSI != 0) 
           len = m_iLTR * 1000 / m_iSI;

   if (len > sams) len = sams;
   //    else len = sams;


   len1 = m_iRbytes;


   switch (m_iFormat)
   {
   case 8015:
      swap2(m_pcBuf + m_iDataHDBytes, len1 - m_iDataHDBytes);
      F8015_to_float((short *)(m_pcBuf + m_iDataHDBytes), buf, len);
      break;
   case 8022:

      F8022_to_float((char *)m_pcBuf + m_iDataHDBytes, buf, len);
      break;
   case 8024:
      swap2(m_pcBuf + m_iDataHDBytes, len1 - m_iDataHDBytes);
      F8024_to_float((short *)(m_pcBuf + m_iDataHDBytes), buf, len);
      break;
   case 8042:
      F8042_to_float((char *)(m_pcBuf + m_iDataHDBytes), buf, len);
      break;
   case 8044:
      swap2(m_pcBuf + m_iDataHDBytes, len1 - m_iDataHDBytes);
      F8044_to_float((short *)(m_pcBuf + m_iDataHDBytes), buf, len);
      break;
   case 8048:
      swap2(m_pcBuf + m_iDataHDBytes, len1 - m_iDataHDBytes);
      F8048_to_float((short *)(m_pcBuf + m_iDataHDBytes), buf, len);
      break;
   case 8058:
      swap2(m_pcBuf + m_iDataHDBytes, len1 - m_iDataHDBytes);
      F8058_to_float((short *)(m_pcBuf + m_iDataHDBytes), buf, len);
      break;
   case 8036:
      //2008.6.12 add 8036,8038
      F8036_to_float((unsigned char *)(m_pcBuf + m_iDataHDBytes), buf, len);
      //F8036_to_float (unsigned char from[], float to[], int len)
      break;
   case 8038:
      swap2(m_pcBuf + m_iDataHDBytes, len1 - m_iDataHDBytes);
      F8038_to_float((short *)(m_pcBuf + m_iDataHDBytes), buf, len);
      break;
   }

// mp processing: for_mp
   int i;
   if(m_fDeScale ==0) m_fDeScale =1;
   for (i = 0; i < len; i++)  
   {     
           buf[i] = buf[i] * pow(2, m_fMP) * m_fDeScale;    
   }
   if (MY_DEBUG) 
       qDebug() << " mp,dscal = " << m_fMP << m_fDeScale << m_icTrShot<<len <<sams;


   return sams;

}
int QSegdRead::setHeader(int *ihead)
{
   int i;
   float *head;
   head = (float *)ihead;
   i = 0;
   head[i] = m_infoMHD.iShot; i++; //0
   head[i] = m_infoMHD.iTr; i++; //1
   head[i] = m_infoMHD.iChannel; i++; //2
   head[i] = m_infoMHD.iType; i++; //1:seis 0:aux//3
   head[i] = m_infoMHD.iCMP; i++; //4
   head[i] = m_infoMHD.iLtr; i++; //5
   head[i] =  m_infoMHD.iOffset; i++; //6

   head[i] = m_infoMHD.iSi; i++; //7
                                 //qDebug() << m_infoMHD.iSi;
   head[i] = m_infoMHD.iValid; i++; //8
   head[i] = m_infoMHD.fShotLine; i++; //9
   head[i] = m_infoMHD.fShotPoint; i++; //10
   head[i] = m_infoMHD.iShotIndex; i++; //11
   head[i] = m_infoMHD.fReceiverLine; i++; //12
   head[i] = m_infoMHD.fReceiverPoint; i++; //13
   head[i] = m_infoMHD.iReceiverIndex; i++; //14
   head[i] = m_infoMHD.iTrsAll; i++; //15
   head[i] = m_infoMHD.iTrs; i++; //16
   head[i] = m_infoMHD.iAux; i++; //17
                                  //qDebug() << "SHot tr si= " <<head[0] << head[1] << head[7];
   return 0;
}

int QSegdRead::decodeTH(unsigned char *ip)
{

   int iFile;
   int iScan;
   int iCHSets;
   int iTr;
   int iExHDs;
   float fStartTime, fSkew;
   unsigned char *p;
 
   p = ip;
   if (MY_DEBUG) 
       qDebug() << " TH start";
   //first header:---------20bytes-----------------------------------

   // file: 1-2 ; bcd4 #E =18-20
   if (p[0] != 0xff) iFile = BCD2Int(p + 1 - 1, 4, true); //1-2
   else iFile = -1;
   // scan type: 3;bcd2
   iScan = BCD2Int(p + 3 - 1, 2, true);     //3
                                            // channel sets: 4;bcd2 #E = 16-17
   if (p[4 - 1] != 0xff) iCHSets = BCD2Int(p + 4 - 1, 2, true);   //4
   else iCHSets = -1;
   // channel trace  number 5-6 ;bcd  #e=22-24
   if (p[5 - 1] != 0xff && p[6 - 1] != 0xff) iTr = BCD2Int(p + 5 - 1, 4, true);
   else iTr = -1;
   //startTime :7-9// 7-9:int2f1;  first time word
   fStartTime = getFloat2_1(p + 7 - 1);
   // extended header :10 ;int1
   iExHDs = *(p + 10 - 1); //10
                           //11: bin sample skew
   fSkew = getFloat1(p + 11 - 1);
   //12: bin, trace edit
   //13-15:bin; time break window
   //16-17: bin;extended channel set number
   if (iCHSets == -1)  iCHSets = getShort(p + 16 - 1);
   // 18-20: bin extended file number:18-20
   if (iFile == -1) iFile = getInt3(p + 18 - 1); //18-20
// find the trace type,mp, m_fDeScale
//fill main header;
   m_infoMHD.iShot = iFile;
   m_infoMHD.iChannel = iTr;
   m_infoMHD.fStartTime =  fStartTime;
   m_infoMHD.fSkew =  fSkew;
   m_iScan = iScan;
   m_iCHSets = iCHSets;
   m_iExHDs = iExHDs;
   return 0;
}
int QSegdRead::decodeTHE(unsigned char *ip)
{
   int i;
   // int iSensorMove,m_iSensorType;
   float fRLine, fRPoint, iRIndex;
   int iSamples;


   unsigned char *p, *pp;
   p = ip;
   if (MY_DEBUG) 
       qDebug() << " THED start";
   //receiver line: 1-3: int or 11-15: 3int+2fraction
   // receiver Line://1-3 int3      // 2006.4: ||-->&&

   if (*p == 0xff && *(p + 1) == 0xff && *(p + 2) == 0xff)
   {
      // 11-15:int4
      fRLine = getSignedFloat3_2(p + 11 - 1);
   }
   else //1-3;
   {
      i = getSignedInt3(p + 1 - 1);
      fRLine = i;
   }
   //receiver point4-6: int or 16-20: 3int+2fraction
   pp = p + 4 - 1; //4-6
   if (*pp == 0xff && *(pp + 1) == 0xff && *(pp + 2) == 0xff)
   {
      // 16-20 int3f2
      fRPoint = getSignedFloat3_2(p + 16);
   }
   else //4-6;int3
   {
      i = getSignedInt3(p);
      //2006.4:fRLine->fRPoint
      fRPoint = i;
   }
   //iReceiver point Index: 7:bin
   iRIndex = *((char *)p  + 7 - 1);

   //isamples: 8-10 :bin3
   if (m_iVersion < 3)  iSamples = getInt3(p + 8 - 1);
   else //versin3:
   {    //sams = 25-28 int4
      iSamples = getInt(p + 25 - 1);

      //trace = 22-24: int3
      if (m_infoMHD.iChannel == -1) m_infoMHD.iChannel = getInt3(p + 22 - 1);
   }
   //qDebug() << "ETH decode=  iSample ,ver= " << iSamples<< m_iVersion;

   //11-15:receiver line(extened)
   //16-20:receiver point(extened)
   //21:bin;sensor type
   //22-24 V3
   //25-28 isample:V3
   //29 isample:V3
   m_iSensorType = *(p + 21 - 1);
   m_iSensorMove = *(p + 29 - 1);

   m_infoMHD.iSams = iSamples;
   m_iSamples = iSamples;
   m_infoMHD.fReceiverLine = fRLine;
   m_infoMHD.fReceiverPoint = fRPoint;
   m_infoMHD.iReceiverIndex = iRIndex;

   return 0;
}
int QSegdRead::SetMainHeader()
{
   int i, id;
   unsigned char *p, *ip;
   p = m_pcBuf;
   //HD1
   decodeTH(p);
   //EHD1
   if (m_iVersion < 3)  decodeTHE(p + 20);
   else
   {
      for (i = 0; i < m_iExHDs; i++)
      {
         ip =   p + 20 + i * GHB_LEN;
         id = *(ip + 32 - 1);
         switch (id)
         {
         case 0x40:
            decodeTHE(ip); //
            break;
         default:
            //qDebug("-----decode THD idx, no.,id = %d, %d, %X", i, i + 1, id);
            break;

         }
      }
   }
//type ,mp,descale:

   int iType = -1;
   iType = m_iChType;
   if (iType != 1) iType = 0;

   //cjh: here we can do some extra info:to covert segd to segy
//HD bytes:
//m_iDataHDBytes =  20 + m_iExHDs * 32;
//m_iCurrentTrace:
//2008.10.24: counter the seismic and auxiliary individually
    
   if (iType == 1) //seismic
   {
      m_ic++;
      m_iCurrentTrace = m_ic;
   }
   else //auxiliary:
   {
      m_ica++;
      m_iCurrentTrace = m_ica;
   }

//fill main header;
//     m_infoMHD.iShot = iFile;
   m_infoMHD.iTr = m_iCurrentTrace;
   m_infoMHD.iChannel = m_icTrShot +1;
   m_infoMHD.iType = iType; //1:seis 0:aux
   m_infoMHD.iCMP = 0;
   m_infoMHD.iLtr = m_iLTR;
   m_infoMHD.iOffset = 0;
   m_infoMHD.iSi = m_iSI;
   //qDebug() << m_iSI;//ok
   m_infoMHD.iValid = 1; //ok;
   m_infoMHD.fShotLine = m_infoGHD.fShotLine;
   m_infoMHD.fShotPoint = m_infoGHD.fShotPoint;
   m_infoMHD.iShotIndex = m_infoGHD.iShotIndex;
   //     m_infoMHD.fReceiverLine = fRLine;
   //     m_infoMHD.fReceiverPoint = fRPoint;
   //    m_infoMHD.iReceiverIndex = iRIndex;

   m_infoMHD.iTrsAll = m_iTrsAll;
   m_infoMHD.iTrs = m_iTrs;
   m_infoMHD.iAux = m_iAux;
   if(MY_DEBUG)
       prTHD();
   // we test if scan number match GHsets scriptions m_iScan is decode in this trace heaer.
   // we donot test CHsets because chsets is not in order of GHsets scriptions; example:606 Data version 3 ID = 50;INOVA Geophysical, Inc
   if (curScan() != m_iScan) 
   {
       qDebug() << "=========CH number Wrong,inHD,inSETS,shot,its=" << m_iCHSets << curChSets()<< m_infoMHD.iShot << m_infoMHD.iChannel;
       return -1;
   }
   
   return 0;
}
void QSegdRead::prTHD()
{
   qDebug() << " THD========================== " << m_infoMHD.iShot << m_infoMHD.iChannel ;
   qDebug() << " shot = " << m_infoMHD.iShot;
   qDebug() << " iTr = " << m_infoMHD.iTr;
   qDebug() << " iChannel = " << m_infoMHD.iChannel;
   qDebug() << " scan = " << m_iScan;
   qDebug() << " chsets = " << m_iCHSets;
   qDebug() << " m_iExHDs = " << m_iExHDs;

   qDebug() << " iType = " << m_infoMHD.iType;
   qDebug() << " mp = " << m_fMP << m_fDeScale;
   qDebug() << " iLtr = " << m_infoMHD.iLtr;
   qDebug() << " iSmas = " << m_infoMHD.iSams;
   qDebug() << " iSi = " << m_infoMHD.iSi;
   qDebug() << " m_iDataHDBytes = " << m_iDataHDBytes;

}
int QSegdRead::SetMainHeader1()
{
   int i;
   int iFile;
   int iScan;
   int iCHsets;
   int iTr;
   int iExHDs;
   float fRLine, fRPoint, iRIndex;
   int iSamples;
   float fStartTime, fSkew;
   int iSensorType;
   unsigned char *p;
   p = m_pcBuf;


   //int len = m_iRbytes;

//first header:---------20bytes-----------------------------------

   // file: 1-2 ; bcd4 #E =18-20
   if (p[0] != 0xff) iFile = BCD2Int(p + 1 - 1, 4, true); //1-2
   else iFile = -1;
   // scan type: 3;bcd2
   iScan = BCD2Int(p + 3 - 1, 2, true);     //3
                                            // channel sets: 4;bcd2 #E = 16-17
   if (p[4 - 1] != 0xff) iCHsets = BCD2Int(p + 4 - 1, 2, true);   //4
   else iCHsets = -1;

   // channel trace  number 5-6 ;bcd  #e=22-24
   if (p[5 - 1] != 0xff && p[6 - 1] != 0xff) iTr = BCD2Int(p + 5 - 1, 4, true);
   else iTr = -1;
   //startTime :7-9// 7-9:int2f1;  first time word
   fStartTime = getFloat2_1(p + 7 - 1);
   // extended header :10 ;int1
   iExHDs = *(p + 10 - 1); //10
                           //11: bin sample skew
   fSkew = getFloat1(p + 11 - 1);
   //12: bin, trace edit
   //13-15:bin; time break window
   //16-17: bin;extended channel set number
   if (iCHsets = -1)  iCHsets = getShort(p + 16 - 1);
   // 18-20: bin extended file number:18-20
   if (iFile == -1) iFile = getInt3(p + 18 - 1); //18-20
// find the trace type,mp, m_fDeScale

   int iType = -1;
   for (i = 0; i < m_iSHDs; i++)
   {
      if (m_pinfoSHD[i].iScanTypeNumber == iScan &&
          m_pinfoSHD[i].iChSetNumber    ==  iCHsets)
      {
         iType = m_pinfoSHD[i].iChType;
         //for_mp
         m_fMP  = m_pinfoSHD[i].mp;
         m_fDeScale = m_pinfoSHD[i].fDeScale;  //v30
         break;
      }
   }
   if (iType != 1) iType = 0;
   //cjh: here we can do some extra info:to covert segd to segy
   m_iDataHDBytes =  20 + iExHDs * 32;
//extended header ----------------32---------------------------------

   if (iExHDs > 0)
   {
      //receiver line: 1-3: int or 11-15: 3int+2fraction
      unsigned char *p;
      p = m_pcBuf + 20  - 1 + 1; //1-3
                                 // 2006.4: ||-->&&
      if (*p == 0xff && *(p + 1) == 0xff && *(p + 2) == 0xff)
      {
         p = m_pcBuf + 20  - 1 + 11; // 11-15
         fRLine = getSignedFloat3_2(p);
      }
      else //1-3;
      {
         i = getSignedInt3(p);
         fRLine = i;
      }
      //receiver point4-6: int or 16-20: 3int+2fraction
      p = m_pcBuf + 20  - 1 + 4; //4-6
      if (*p == 0xff && *(p + 1) == 0xff && *(p + 2) == 0xff)
      {
         p = m_pcBuf + 20  - 1 + 16; // 16-20
         fRPoint = getSignedFloat3_2(p);
      }
      else //4-6;
      {
         i = getSignedInt3(p);
         //2006.4:fRLine->fRPoint
         fRPoint = i;
      }
      //iReceiver point Index: 7:bin
      p = m_pcBuf + 20  - 1 + 7; //7
      iRIndex = *((char *)p);
      //isamples: 8-10 :bin
      p = m_pcBuf + 20  - 1 + 8; //8-10
      iSamples = getInt3(p);

      //11-15:receiver line(extened)
      //16-20:receiver point(extened)
      //21:bin;sensor type
      p = m_pcBuf + 20  - 1 + 21; //21
      iSensorType = (unsigned char)*p;

   }
   //2008.10.24: counter the seismic and auxiliary individually
   if (iType == 1) //seismic
   {
      m_ic++;
      m_iCurrentTrace = m_ic - m_ica;
   }
   else //auxiliary:
   {
      m_ica++;
      m_iCurrentTrace = m_ica;
   }

//fill main header;
   m_infoMHD.iShot = iFile;
   m_infoMHD.iTr = m_iCurrentTrace;
   m_infoMHD.iChannel = iTr;
   m_infoMHD.iType = iType; //1:seis 0:aux
   m_infoMHD.iCMP = 0;
   m_infoMHD.iLtr = m_iLTR;
   m_infoMHD.iOffset = 0;
   m_infoMHD.iSi = m_iSI;
   //qDebug() << m_iSI;//ok
   m_infoMHD.iValid = 1; //ok;
   m_infoMHD.fShotLine = m_infoGHD.fShotLine;
   m_infoMHD.fShotPoint = m_infoGHD.fShotPoint;
   m_infoMHD.iShotIndex = m_infoGHD.iShotIndex;
   m_infoMHD.fReceiverLine = fRLine;
   m_infoMHD.fReceiverPoint = fRPoint;
   m_infoMHD.iReceiverIndex = iRIndex;
   m_infoMHD.iTrsAll = m_iTrsAll;
   m_infoMHD.iTrs = m_iTrs;
   m_infoMHD.iAux = m_iAux;


   if (m_debug) qDebug("data header info:file = %d,tr= %d,type = %d, si = %d,ltr = %d,%d,headers = %d\n",
                       iFile, m_ic, iTr, iType, m_iSI, m_iLTR, m_iDataHDBytes);


   return 0;
}
int QSegdRead::SetOffset(int off)
{
   m_iOffsetBytes = off;
   return 0;
}
int QSegdRead::CloseTape()
{
   m_tp->rewind();
   m_tp->unload();
   qDebug() << "in CloseTape";
   return m_tp->close();
}



int QSegdRead::GetTrs()
{
   int i;
   int id = 0;
   for (i = 0; i < m_iSHDs; i++)
   {
      id = id + m_pinfoSHD[i].iChs;
   }
   return id;
}
int QSegdRead::GetAuxes()
{
   int i;
   int id = 0;
   for (i = 0; i < m_iSHDs; i++)
   {
      if (m_pinfoSHD[i].iChType != 1) id = id + m_pinfoSHD[i].iChs;
   }
   return id;
}



/* F0015_to_float - convert 20 bit binary multiplexed data into floating numbers
 *
 * Credits:
 *      EOPG: Marc Schaming, Jean-Daniel Tissot
 *      SEP:  Stew Levin - fixed low-order bit error in conversion
 *            of negative values on 2's complement machines.
 *            Use ldexp() function instead of much slower value*pow(2,expo)
 *      SEP:  Adapted F8015 to F0015 conversion
 *
 *
 * Parameters:
 *    from   - input vector
 *    to     - output vector
 *    len    - number of packets of 4 floats in vectors
 *
 */
/*
 *
 * Format 0015 is a 10 byte per 4 words (2 1/2 bytes per word)
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
 * Byte 4    Q-8   Q-9   Q-10  Q-11  Q-12  Q-13  Q-14   0
 * Byte 5     S    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7   Channel 2
 * Byte 6    Q-8   Q-9   Q-10  Q-11  Q-12  Q-13  Q-14   0
 * Byte 7     S    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7   Channel 3
 * Byte 8    Q-8   Q-9   Q-10  Q-11  Q-12  Q-13  Q-14   0
 * Byte 9     S    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7   Channel 4
 * Byte 10   Q-8   Q-9   Q-10  Q-11  Q-12  Q-13  Q-14   0
 *
 * S=sign bit. - (One = negative number)
 * C=binary exponents. - This is a 4 bit positive binary exponent of 2
 *               CCCC
 *   written as 2     where CCCC can assume values of 0-15.  The four
 *   exponents are in channel number order for the four channels starting
 *   with channel one in bits 0-3 of Byte 1.
 * Q1-14-fraction. - This is a 14 bit one's complement binary fraction.
 *   The radix point is to the left of the most significant bit (Q-1)
 *                                  -1
 *   with the MSB being defined as 2 .  The sign and fraction can assume
 *                  -14       -14
 *   values from 1-2   to -1+2  .  Note that bit 7 of the second byte
 *   of each sample must be zero in order to guarantee the uniqueness of
 *   the start of scan.  Negative zero is invalid and must be converted
 *   to positive zero.
 *                                       CCCC    MP                   MP
 * Input signal = S.QQQQ,QQQQ,QQQQ,QQ x 2    x 2    millivolts where 2
 *   is the value required to descale the data word to the recording
 *   system input level.  MP is defined in Byte 8 of each of the corre-
 *   sponding channel set descriptors in the scan type header.
 * Note that in utilizing this data recording method, the number of data
 *   channels per channel set must be exactly divisible by 4 in order to
 *   preserve the data grouping of this method.
 */


void QSegdRead::F0015_to_float(short from[], float to[], int len)
{
   register int i;
   register short ex1_4;
   int expo;
   short fraction;

   for (i = 0; i < len; i += 4)
   {
      ex1_4 = GET_S(from);
      expo = ((ex1_4 >> 12) & 0x0F) - 15;
      fraction = GET_S(from);
      if (fraction < 0) fraction = -((~fraction) & (~1));
      *(to++) = ldexp((double)fraction, expo);

      expo = ((ex1_4 >> 8) & 0x0F) - 15;
      fraction = GET_S(from);
      if (fraction < 0) fraction = -((~fraction) & (~1));
      *(to++) = ldexp((double)fraction, expo);

      expo = ((ex1_4 >> 4) & 0x0F) - 15;
      fraction = GET_S(from);
      if (fraction < 0) fraction = -((~fraction) & (~1));
      *(to++) = ldexp((double)fraction, expo);

      expo = (ex1_4 & 0x0F) - 15;
      fraction = GET_S(from);
      if (fraction < 0) fraction = -((~fraction) & (~1));
      *(to++) = ldexp((double)fraction, expo);
   }
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


void QSegdRead::F8015_to_float(short from[], float to[], int len)
{
   register int i;
   register short ex1_4;
   int expo;
   short fraction;

   for (i = 0; i < len; i += 4)
   {
      ex1_4 = GET_S(from);
      expo = ((ex1_4 >> 12) & 15) - 15;
      fraction = GET_S(from);
      if (fraction < 0) fraction = -(~fraction);
      *(to++) = ldexp((double)fraction, expo);

      expo = ((ex1_4 >> 8) & 15) - 15;
      fraction = GET_S(from);
      if (fraction < 0) fraction = -(~fraction);
      *(to++) = ldexp((double)fraction, expo);

      expo = ((ex1_4 >> 4) & 15) - 15;
      fraction = GET_S(from);
      if (fraction < 0) fraction = -(~fraction);
      *(to++) = ldexp((double)fraction, expo);

      expo = (ex1_4 & 15) - 15;
      fraction = GET_S(from);
      if (fraction < 0) fraction = -(~fraction);
      *(to++) = ldexp((double)fraction, expo);
   }
}

/* F8022_to_float - convert 8 bit quaternary demultiplexed data into floating numbers
 *
 * Credits:
 *      SEP:  Stew Levin
 *
 * Parameters:
 *    from   - input sfio unit
 *    to     - output vector
 *    len    - number of packets of 4 floats in vectors
 *
 */
/*
 *
 * Format 8022 is a 1 byte per word representation.
 * According to the SEG specifications, the bit
 * layout of the byte is:
 *
 *
 *  Bit       0     1     2     3     4     5     6     7
 *-----------------------------------------------------------
 * Byte 1     S    C2    C1    C0    Q-1   Q-2   Q-3   Q-4
 *
 * S=sign bit. - (One = negative number)
 * C=quaternary exponent. - This is a 3 bit positive binary exponent of 4
 *               CCC
 *   written as 4    where CCC can assume values of 0-7.
 * Q1-4-fraction. - This is a 4 bit one's complement binary fraction.
 *   The radix point is to the left of the most significant bit (Q-1)
 *                                  -1
 *   with the MSB being defined as 2 .  The fraction can have values
 *           -4        -4
 *   from 1-2   to -1+2  .  Negative zero is invalid and must be
 *   converted to positive zero.
 *                          CCC   MP                   MP
 * Input signal = S.QQQQ x 4   x 2   millivolts where 2    is the
 *   value required to descale the data word to the recording system
 *   input level.  MP is defined in Byte 8 of each of the corre-
 *   sponding channel set descriptors in the scan type header.
 */


void QSegdRead::F8022_to_float(char from[], float to[], int len)
{
   register int i;
   register int ex1_4;
   int expo;
   short fraction;

   for (i = 0; i < len; i++)
   {
      ex1_4 = GET_C(from);
      expo = ((ex1_4 >> 3) & 14) - 4;
      fraction = ex1_4 & 15;
      if (ex1_4 & 128) fraction = -(15 ^ fraction);
      *(to++) = ldexp((double)fraction, expo);
   }
}

/* F8024_to_float - convert 16 bit quaternary demultiplexed data into floating numbers
 *
 * Credits:
 *      SEP:  Stew Levin
 *
 * Parameters:
 *    from   - input sfio unit
 *    to     - output vector
 *    len    - number of packets of 4 floats in vectors
 *
 */
/*
 *
 * Format 8024 is a 2 byte per word representation.
 * According to the SEG specifications, the bit
 * layout of the bytes is:
 *
 *
 *  Bit       0     1     2     3     4     5     6     7
 *-----------------------------------------------------------
 * Byte 1     S    C2    C1    C0    Q-1   Q-2   Q-3   Q-4
 * Byte 2    Q-5   Q-6   Q-7   Q-8   Q-9   Q-10  Q-11  Q-12
 *
 * S=sign bit. - (One = negative number)
 * C=quaternary exponent. - This is a 3 bit positive binary exponent of 4
 *               CCC
 *   written as 4    where CCC can assume values of 0-7.
 * Q1-12-fraction. - This is a 12 bit one's complement binary fraction.
 *   The radix point is to the left of the most significant bit (Q-1)
 *                                  -1
 *   with the MSB being defined as 2 .  The fraction can have values
 *           -12        -12
 *   from 1-2    to -1+2   .  Negative zero is invalid and must be
 *   converted to positive zero.
 *                                    CCC   MP                   MP
 * Input signal = S.QQQQ,QQQQ,QQQQ x 4   x 2   millivolts where 2  
 *   is the value required to descale the data word to the recording
 *   system input level.  MP is defined in Byte 8 of each of the corre-
 *   sponding channel set descriptors in the scan type header.
 */


void QSegdRead::F8024_to_float(short from[], float to[], int len)
{
   register int i;
   register int ex1_4;
   int expo;
   short fraction;

   for (i = 0; i < len; i++)
   {
      ex1_4 = GET_S(from);
      expo = ((ex1_4 >> 11) & 14) - 12;
      fraction = ex1_4 & 4095;
      if (ex1_4 & 32768) fraction = -(4095 ^ fraction);
      *(to++) = ldexp((double)fraction, expo);
   }
}

/* F8036_to_float - convert 24 bit quaternary demultiplexed data into floating numbers
 *
 * Credits:
 *      SEP:  Stew Levin
 *
 * Parameters:
 *    from   - input sfio unit
 *    to     - output vector
 *    len    - number of packets of 4 floats in vectors
 *
 */
/*
 *
 * Format 8036 is a 3 byte per word representation.
 * According to the SEG specifications, the bit
 * layout of the bytes is:
 *
 *
 *  Bit       0     1     2     3     4     5     6     7
 *-----------------------------------------------------------
 * Byte 1    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7   Q-8
 * Byte 2    Q-9   Q-10  Q-11  Q-12  Q-13  Q-14  Q-15  Q-16
 * Byte 3    Q-17  Q-18  Q-19  Q-20  Q-21  Q-22  Q-23  Q-24
 *
 * Q1-24-integer. - This is a 24 bit two's complement binary integer.
 *                         MP                   MP
 * Input signal = Q...Q x 2   millivolts where 2  
 *   is the value required to descale the data word to the recording
 *   system input level.  MP is defined in Byte 8 of each of the corre-
 *   sponding channel set descriptors in the scan type header.
 */


void QSegdRead::F8036_to_float(unsigned char from[], float to[], int len)
{
   register int i;
   register long int ival;

   for (i = 0; i < len; i++)
   {
      ival = GET_UC(from);
      ival <<= 8; ival |= GET_UC(from);
      ival <<= 8; ival |= GET_UC(from);
      if (ival > 8388607) ival -= 16777216;
      *(to++) = (float)ival;
   }
}

/* F8038_to_float - convert 32 bit quaternary demultiplexed data into floating numbers
 *
 * Credits:
 *      SEP:  Stew Levin
 *
 * Parameters:
 *    from   - input sfio unit
 *    to     - output vector
 *    len    - number of packets of 4 floats in vectors
 *
 */
/*
 *
 * Format 8038 is a 4 byte per word representation.
 * According to the SEG specifications, the bit
 * layout of the bytes is:
 *
 *
 *  Bit       0     1     2     3     4     5     6     7
 *-----------------------------------------------------------
 * Byte 1    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7   Q-8
 * Byte 2    Q-9   Q-10  Q-11  Q-12  Q-13  Q-14  Q-15  Q-16
 * Byte 3    Q-17  Q-18  Q-19  Q-20  Q-21  Q-22  Q-23  Q-24
 * Byte 4    Q-25  Q-26  Q-27  Q-28  Q-29  Q-30  Q-31  Q-32
 *
 * Q1-32-fraction. - This is a 32 bit two's complement binary integer.
 *                         MP                   MP
 * Input signal = Q...Q x 2   millivolts where 2  
 *   is the value required to descale the data word to the recording
 *   system input level.  MP is defined in Byte 8 of each of the corre-
 */
/* Note this conversion routine assumes the target architecture is
 * already 2's complement.
 */


void QSegdRead::F8038_to_float(short from[], float to[], int len)
{
   int i;
   long int ex1_4;
   long int ex2_4;
   long int value;

   for (i = 0; i < len; i++)
   {
      ex1_4 = GET_S(from);
      ex2_4 = GET_S(from);
      value = (ex1_4 << 16) | (ex2_4 & 65535);
      *(to++) = (float)value;
   }
}

/* F8042_to_float - convert 8 bit hexadecimal demultiplexed data into floating numbers
 *
 * Credits:
 *      SEP:  Stew Levin
 *
 * Parameters:
 *    from   - input sfio unit
 *    to     - output vector
 *    len    - number of floats in vector
 *
 */
/*
 *
 * Format 8042 is a 1 byte per word representation.
 * According to the SEG specifications, the bit
 * layout of the byte is:
 *
 *
 *  Bit       0     1     2     3     4     5     6     7
 *-----------------------------------------------------------
 * Byte 1     S    C1    C0    Q-1   Q-2   Q-3   Q-4   Q-5
 *
 * S=sign bit. - (One = negative number)
 * C=hexadecimal exponent. - This is a 2 bit positive binary exponent of 16
 *                CC
 *   written as 16    where CC can assume values of 0-3.
 * Q1-5-fraction. - This is a 5 bit positive binary fraction.
 *   The radix point is to the left of the most significant bit (Q-1)
 *                                  -1
 *   with the MSB being defined as 2 .  The fraction can have values
 *           -5        -5
 *   from 1-2   to -1+2  .  
 *                             CC    MP                   MP
 * Input signal = S.QQQQ,Q x 16   x 2   millivolts where 2    is the
 *   value required to descale the data word to the recording system
 *   input level.  MP is defined in Byte 8 of each of the corre-
 *   sponding channel set descriptors in the scan type header.
 */

void QSegdRead::F8042_to_float(char from[], float to[], int len)

{
   register int i;
   register int ex1_4;
   int expo;
   short fraction;

   for (i = 0; i < len; i++)
   {
      ex1_4 = GET_C(from);
      expo = ((ex1_4 >> 3) & 12) - 5;
      fraction = ex1_4 & 31;
      if (ex1_4 & 128) fraction = -fraction;
      *(to++) = ldexp((double)fraction, expo);
   }
}

/* F8044_to_float - convert 16 bit hexadecimal demultiplexed data into floating numbers
 *
 * Credits:
 *      SEP:  Stew Levin
 *
 * Parameters:
 *    from   - input sfio unit
 *    to     - output vector
 *    len    - number of floats in vector
 *
 */
/*
 *
 * Format 8044 is a 2 byte per word representation.
 * According to the SEG specifications, the bit
 * layout of the bytes is:
 *
 *
 *  Bit       0     1     2     3     4     5     6     7
 *-----------------------------------------------------------
 * Byte 1     S    C1    C0    Q-1   Q-2   Q-3   Q-4   Q-5
 * Byte 2    Q-6   Q-7   Q-8   Q-9   Q-10  Q-11  Q-12  Q-13
 *
 * S=sign bit. - (One = negative number)
 * C=hexadecimal exponent. - This is a 2 bit positive binary exponent of 16
 *                CC
 *   written as 16    where CC can assume values of 0-3.
 * Q1-13-fraction. - This is a 13 bit positive binary fraction.
 *   The radix point is to the left of the most significant bit (Q-1)
 *                                  -1
 *   with the MSB being defined as 2 .  The fraction can have values
 *           -13        -13
 *   from 1-2    to -1+2   .  
 *                                       CC    MP                   MP
 * Input signal = S.QQQQ,QQQQ,QQQQ,Q x 16   x 2   millivolts where 2    
 *   is the value required to descale the data word to the recording system
 *   input level.  MP is defined in Byte 8 of each of the corre-
 *   sponding channel set descriptors in the scan type header.
 */


void QSegdRead::F8044_to_float(short from[], float to[], int len)

{
   register int i;
   register int ex1_4;
   int expo;
   short fraction;

   for (i = 0; i < len; i++)
   {
      ex1_4 = GET_S(from);
      expo = ((ex1_4 >> 11) & 12) - 13;
      fraction = ex1_4 & 8191;
      if (ex1_4 & 32768) fraction = -fraction;
      *(to++) = ldexp((double)fraction, expo);
   }
}

/* F8048_to_float - convert 32 bit hexadecimal demultiplexed data into floating numbers
 *
 * Credits:
 *      SEP:  Stew Levin
 *
 * Parameters:
 *    from   - input sfio unit
 *    to     - output vector
 *    len    - number of floats in vector
 *
 */
/*
 *
 * Format 8048 is a 4 byte per word representation.
 * According to the SEG specifications, the bit
 * layout of the bytes is:
 *
 *
 *  Bit       0     1     2     3     4     5     6     7
 *-----------------------------------------------------------
 * Byte 1     S    C6    C5    C4    C3    C2    C1    C0 
 * Byte 2    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7   Q-8 
 * Byte 3    Q-9   Q-10  Q-11  Q-12  Q-13  Q-14  Q-15  Q-16
 * Byte 4    Q-17  Q-18  Q-19  Q-20  Q-21  Q-22  Q-23  0
 *
 * S=sign bit. - (One = negative number)
 * C=hexadecimal exponent. - This is a binary exponent of 16
 *                (CCCCCCC-64)
 *   written as 16             where CC can assume values of 0-127.
 * Q1-23-fraction. - This is a 23 bit positive binary fraction.
 *   The radix point is to the left of the most significant bit (Q-1)
 *                                  -1
 *   with the MSB being defined as 2 .  The sign and fraction can have
 *                 -23        -23
 *   values from 1-2    to -1+2   .  
 *                                   C-64    MP                   MP
 * Input signal = S.QQQQ,...,QQQ x 16     x 2   millivolts where 2    
 *   is the value required to descale the data word to the recording system
 *   input level.  MP is defined in Byte 8 of each of the corre-
 *   sponding channel set descriptors in the scan type header.
 *   The data recording method has more than sufficient range to
 *   handle the dynamic range of a typical seismic system.  Thus, MP
 *   may not be needed to account for any scaling and may be recorded
 *   as zero.
 */


void QSegdRead::F8048_to_float(short from[], float to[], int len)

{
   register int i;
   register int ex1_4;
   int expo;
   long int fraction;

   for (i = 0; i < len; i++)
   {
      ex1_4 = GET_S(from);
      expo = ((ex1_4 >> 6) & 508) - (24 + 256);
      fraction = ex1_4 & 255;
      fraction <<= 16; fraction |= (GET_S(from) & 65535);
      if (ex1_4 & 32768) fraction = -fraction;
      *(to++) = ldexp((double)fraction, expo);
   }
}

/* F8058_to_float - convert 32 bit IEEE float demultiplexed data into floating numbers
 *
 * Credits:
 *      SEP:  Stew Levin
 *
 * Parameters:
 *    from   - input sfio unit
 *    to     - output vector
 *    len    - number of floats in vector
 *
 */
/*
 *
 * Format 8058 is a 4 byte per word representation.
 * According to the SEG specifications, the bit
 * layout of the bytes is:
 *
 *
 *  Bit       0     1     2     3     4     5     6     7
 *-----------------------------------------------------------
 * Byte 1     S    C7    C6    C5    C4    C3    C2    C1 
 * Byte 2    C0    Q-1   Q-2   Q-3   Q-4   Q-5   Q-6   Q-7
 * Byte 3    Q-8   Q-9   Q-10  Q-11  Q-12  Q-13  Q-14  Q-15
 * Byte 4    Q-16  Q-17  Q-18  Q-19  Q-20  Q-21  Q-22  Q-23
 *
 * S=sign bit. - (One = negative number)
 * C=exponent. - This is a excess-127 binary exponent of 2
 *               (CCCCCCCC-127)
 *   written as 2               where CC can assume values of 0-255.
 * Q1-23-fraction. - This is a 23 bit positive binary fraction.
 *   The radix point is to the left of the most significant bit (Q-1)
 *                                  -1
 *   with the MSB being defined as 2 .  With the exceptions noted below:
 *
 *                    S                    C-127    MP                   MP
 * Input signal = (-1) x 1.QQQQ,...,QQQ x 2     x 2   millivolts where 2    
 *   is the value required to descale the data word to the recording system
 *   input level.  MP is defined in Byte 8 of each of the corre-
 *   sponding channel set descriptors in the scan type header.
 *   The data recording method has more than sufficient range to
 *   handle the dynamic range of a typical seismic system.  Thus, MP
 *   may not be needed to account for any scaling and may be recorded
 *   as zero.
 *
 * Exceptions:
 *
 * If C=0 then
 *                    S                    -126     MP
 * Input signal = (-1) x 0.QQQQ,...,QQQ x 2     x 2   millivolts
 *
 * If C=255 and Q=0, then
 *                    S 
 * Input signal = (-1) x infinity  (overflow)
 *
 * If C=255 and Q!=0, then
 *                      
 * Input signal = NaN  (Not-a-Number)
 */


void QSegdRead::F8058_to_float(short from[], float to[], int len)

{
   register int i;
   register int ex1_4, ex2_4;
   int expo;
   long int fraction;

   for (i = 0; i < len; i++)
   {
      ex1_4 = GET_S(from);
      ex2_4 = GET_S(from);
      expo = ((ex1_4 >> 7) & 255);
      fraction = ex1_4 & 127;
      fraction <<= 16; fraction |= (ex2_4 & 65535);
      if (expo) fraction |= 8388608;
      else fraction <<= 1;
      if (ex1_4 & 32768) fraction = -fraction;
      *(to++) = ldexp((double)fraction, expo - (23 + 127));
   }
}


float QSegdRead::GetMP(unsigned char *mp)
{
   float f7, f8, ff;
   int s;
   int i, i7, i8;
   unsigned char ch;
   strncpy((char *)&ch, (char *)mp, 1);

// byte 7th:
   f7 = 0;
   i7 = -3;
   for (i = 0; i < 8; i++)
   {
      ff = 0;
      if ((ch & (unsigned char)((float)pow((float)2, (float)7 - i))) != 0)
      {
         ff = pow((float)2, (float)i7 - i);
         f7 = f7 + ff;
      }
   }

// byte 8th:
   strncpy((char *)&ch, (char *)mp + 1, 1);
   f8 = 0;
   i8 = 4;
   for (i = 0; i < 8 - 1; i++)
   {
      ff = 0;
      if ((ch & (unsigned char)(pow((float)2, (float)7 - 1 - i))) != 0)
      {
         ff = pow((float)2, (float)i8 - i);
         f8 = f8 + ff;
      }
   }

//sign:
   s  = 0;
   if ((ch & (unsigned char)(pow((float)2, (float)7))) != 0) s = 1;
// final:
   if (s == 0) ff = f8 + f7;
   else ff = -1 * (f7 + f8);

   return ff;
}

int QSegdRead::NextFile()
{
   qint64 fsize;
   int shots;
   QString str;

   //qDebug("NextFile ==== %d,%d\n", m_icFile,m_tp->m_iDiskFiles);
// not the first time:
   if (m_icFile != 0)
   {
      m_tp->close();
      if (m_icFile >= m_iAllFiles) return 0; // complete
   }

// next file :first file is 0;

   m_file =  m_listFiles[m_icFile];
   m_icFile++;
   // qDebug("Filename ==== %s\n", m_file.ascii());
// open

   if (!m_tp->open(m_file)) return -1; //cannot open
// offset
   m_tp->seek(m_iOffsetBytes);

   qDebug() << "this in Nextfile offset = "   << m_iOffsetBytes;

// read the file header;
   int iret;
   iret = GetFileInfo();
   //qDebug()<< "trs In Next File1 ="<< m_iTrs<<m_iAux<<m_iTrsAll;
   if (iret == -1) return -2; //read err
   else if (iret == 0) return -2; //read err
   else if (iret == -2) return -3; // not segd
   else if (iret > 0)
   {
      // first time read ,we donot know the real data header bytes;
      //the data bytes;
      if (m_tp->getType() == DEV_DISK)
      {
         if (m_icFile <= 1) // first file:??
         { // for all file ,not just first segd file,avoid the data header is not the same

            if (!m_bTif8) // read a trace for header  to the first trace: after GetFileInfo
            {
              // m_tp->seek(m_iOffsetBytes + m_iHeadBytes); //
              // ReadRecord(); //read
              // SetMainHeader(); //find the real data_header,next will read the whole trace;to see if head = 10 or more;
              // m_tp->seek(m_iOffsetBytes + m_iHeadBytes); // repoiter:
              // getExHD();
            }
         }
         //qDebug()<< "trs In Next File1 ="<< m_iTrs<<m_iAux<<m_iTrsAll;
         m_ic = 0;
         m_ica = 0;
// shots in the file
         fsize = m_tp->size();
         shots = (fsize - m_iOffsetBytes) /
            (m_iHeadBytes + m_iTrailBytes +
             (m_iDataHDBytes + m_iDataBytes + m_iTraceGapBytes) * m_iTrsAll);
         m_iShotsInFile = shots; // + 1;
                                 //return iret;
      } //end of disk:
      else
      {
         fsize = 0;
         m_iShotsInFile = 1;
         //return iret;
      }
      qDebug() << "file size =" << fsize;
      qDebug() << "offset =" << m_iOffsetBytes << QString("%1").arg(m_iOffsetBytes, 10, 16);
      qDebug() << "m_iTrsAll of shot=" << m_iTrsAll;
      qDebug() << "m_iHeadBytes =" << m_iHeadBytes;
      qDebug() << "m_iTrailBytes =" << m_iTrailBytes;
      qDebug() << "m_iDataHDBytes =" << m_iDataHDBytes;
      qDebug() << "m_iDataBytes =" << m_iDataBytes;
      qDebug() << "m_iTraceGapBytes =" << m_iTraceGapBytes;

      qDebug() << "m_iShotsInFile" << m_iShotsInFile;

      str  = QString("hex = All,+headByte = %1,%2").arg(m_iDataBytes + m_iDataHDBytes + m_iTraceGapBytes, 10, 16)
         .arg(m_iDataBytes + m_iDataHDBytes + m_iTraceGapBytes + m_iHeadBytes, 10, 16);
      qDebug() << "disk file:dataHDBytes , DataBytes,gap = " << m_iDataHDBytes << m_iDataBytes << m_iTraceGapBytes;
      qDebug() << str;
      //return -3;
      return iret;
   }
   else return -4;

}


