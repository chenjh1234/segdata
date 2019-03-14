// QSegCommon.h: interface for the QSegCommon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QSEGCOMMON_H__62E9411E_5268_4179_90F3_8F43942870C3__INCLUDED_)
#define AFX_QSEGCOMMON_H__62E9411E_5268_4179_90F3_8F43942870C3__INCLUDED_


#include <QString>
#include"QTapeIO.h"
#include<math.h>
#include <QDebug>
//#define TAPE_BLOCK 128000
 
class QSegCommon :public QObject
{
public:

   QSegCommon();
   virtual ~QSegCommon();

   dataIO m_tape,*m_tp,*m_tpo;
   unsigned char m_pcBuf[TAPE_BLOCK];  //buf used when read tape

   /**
   Impormant information in data header
   */
   typedef struct
   {
      int iShot ;//9-12,13-16
      int iFfid;//9-12
      int iTr;//13-16
      int iChannel;//13-16
      int iType; //1:ok,2:aux
      int iCMP,iCMPTr;// 21-24,25-28
      int iLine;//189-192
      int iLineY;//193-196
      int iLagA,iLagB;//105-106,107-108

     
      int iScaleXy,iScaleEle,iScaleSt;//71-72,69-70,125-126;

      int iLtr;
      int iSi;
      int iSams;//17-18;15-16
      int iOffset;//37-40
      int iValid; //1:ok 0:invalid
// scale xy:
      int iCMPX,iCMPY;//181-184,185-188
      int iShotX,iShotY;//73-76,77-79
      int iReceiverX,iReceiverY;//81-84,85-88
// scale lp
      int iStaticsShot,iStaticsReceiver,iStaticsSum;//99-100,101-102,103-104
      int iUpholeShot,iUpholeReceiver;//95-96,97-98
      int iDelay;//

      int iEleShot,iEleReceiver,iEleDReceiver,iEleDShot;//45-48,41-44,53-56,57-60
      int iDepthShot;//49-52
      int iWaterShot,iWaterReceiver;//61-64,65-68

      int iMuteS,iMuteE;//111-112,113-114
      //segd?
      float fStartTime;
      float fSkew;

// not used:
#if 1
      float fShotLine;//77-78
      float fShotPoint;//73-76
      float fReceiverLine;//81-84
      float fReceiverPoint;//85-88
#endif
      int   iReceiverIndex;
      int   iShotIndex;//

      int iTrsAll; // all traces in shot
      int iTrs; // seismic traces in shot
      int iAux; //aux traces in shot
   }MAINHD_INFO;

   MAINHD_INFO m_infoMHD;

   int m_iSamples;        //samples of the output of the trace
   int m_iFormat;
   int m_iSI; //us
   int m_iLTR;
   int m_iEndian;

   int m_iIUnit;
   int m_iOUnit; 

   float m_iOSi;
   int m_iOLtr;
   int m_debug;


   QString m_strStatus;

   virtual void Init() {
   };
   void Init0();

   /** get the Endian of the omputer sun:1,pc = 0*/
   int getEndian();
   /** get trace length in ms   */
   int getLtr();
   int setLtr(int ltr);
   /** get the sample rate in us*/
   int getSi();
   int setSi(int si);
   /** get  samples of the trace   */
   int getSamples();
   /** set  samples of the trace   */
   int setSamples(int sams);
   /** get the data format*/
   int getFormat();
   int setFormat(int fmt);
   /** get status of last read */
   QString getStatus();
   /** report status after read */
   void setStatus(QString s);

   //QDiskIO * GetTape();
   //int SetTape(QDiskIO *tp);
/**
    DCB2Int
    @para:
        buf:  pointer 
        i :numbers of the digital(half byte), 
        flag: true:from first halt of the start byte
             false:from last halt of the start byte
*/
   int BCD2Int(unsigned char *buf, int i, bool flag = true);


   /** get signed int 3 bytes*/
   int getSignedInt3(unsigned char *);
   /** signed int 2 bytes*/
   int getSignedShort(unsigned char *);
   /** unsigned int 3 bytes */
   int getInt3(unsigned char *);
   /** 5 bytes: 3 bytes integer,2 bytes fraction */
   float getSignedFloat3_2(unsigned char *);  
    /** get float   */
   float getFloat(void *buf);
   float getFloat1(unsigned char *buf);
      /** 1 bytes integer ,one bytes frction*/
   float getFloat1_1(unsigned char  *p);
   float getFloat2_1(unsigned char  *p);
 
   /** get int   */
   int getInt(unsigned char *buf);
   int getInt1H(unsigned char *buf);
   int getInt1L(unsigned char *buf);
   qint64 getInt64(unsigned char *buf);
   qint64 getInt8(unsigned char *buf);
   /** get short   */
   int getShort(unsigned char *buf);


   /** set int  */
   int setInt(int *buf, int id);
   /** set short   */
   int setShort(short *buf, short id);

   /**swap each  next two bytes */
   void swap2(unsigned char *buf, int len);
   /**swap each  next four bytes */
   void swap4(unsigned char *buf, int len);
   void swap8(unsigned char *buf, int len);
   /**float to ibm*/
   void float_to_ibm(int from[], int to[], int n);
};

#endif // !defined(AFX_QSEGCOMMON_H__62E9411E_5268_4179_90F3_8F43942870C3__INCLUDED_)
