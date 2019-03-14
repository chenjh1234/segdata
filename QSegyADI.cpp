// QSegyADI.cpp: implementation of the QSegyADI class.
//
//////////////////////////////////////////////////////////////////////
//#include <QApplication>
#include "QSegyADI.h"
#include <QObject>
#define   SHOT  3 
#define   CMP   6 
#define   LINE   48 
#define   LINEC  49 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//QString QSegyADI::m_key = "";
segyThread::segyThread():QThread()
{
}
void segyThread::run()
{
    QSegyADI sgy;
   //  qDebug() << " thread run file,hd=" << _file << _hd;
    _ret = sgy.createIdxFile(_file,_hd);
  //  qDebug() << "end of thread run";
    //emit sigDown();
}; 
       
QSegyADI::QSegyADI() : QSegCommon()
{
   init();
}

QSegyADI::~QSegyADI()
{
   // qDebug() << " dddddddddddd11111";
   if (m_pIdx != NULL) delete[] m_pIdx;
    //qDebug() << " dddddddddddd222222222222222";

}
void QSegyADI::init()
{

   setWriteFormat(5);
   m_strFilename = " ";
   m_strFilenameIdx = " ";
   m_pIdx = NULL;
   m_iReelCounter = 0;
   m_iLineCounter = 0;
   m_iCurGather = 0;
   m_ic = 0;
   m_iIdxHD =0;
   m_iMaxTrsOfGather =1;
  // th = new segyThread();

   connect(&th, SIGNAL(finished()),
           this, SLOT(slotIFile()));
//   connect(&th, SIGNAL(sigThreadDown()),
//           this, SLOT(slotIFile()));

}
void QSegyADI::slotIFile()
{
    #if 1
    qDebug() << " slot  creat idx down ret= " << th.getRet();
    //return;
    if (th.getRet() >=0) 
    {
        //closeRead();
        //openRead(th.getFile());
        getIdxInfo();      
    }
    emit sigThreadDown();
    #endif
}
void QSegyADI::createThIdxFile(QString filen, int h)
{
    #if 1
    th.setFile(filen,h);
    qDebug() << "connected ============================";
    th.start();
    qDebug() << "start wait";
    th.wait();
    qDebug() << "after wait";
   #endif
}
int QSegyADI::createIdxFile(QString filen, int hd)
{
   int i, j;
   m_strFilename = filen;
   m_strFilenameIdx = filen + ".idx";
   QFile file;
   //open file
   file.setFileName(m_strFilenameIdx);
   if (file.exists())
   {
      return 0;
      file.remove();
      file.setFileName(m_strFilenameIdx);
   }


   i = openRead(filen);
   if (i < 0) return -2;

   if (!file.open(QIODevice::ReadWrite)) return -1; //cannot open idx
                                                    //read abd write
   i = hd;
   file.write((char *)&i, sizeof(int)); //hd
   file.write((char *)&i, sizeof(int)); // all groups

   INDEX_INFO pidx;
   int ichd, ihd, ioldhd, icchd, iccchd, idxhd;

   ichd = 0; // trace counter in the gather
   icchd = 0; //gather counter
   iccchd = 0; //trace counter in all;
   idxhd = 0;
   qDebug() << "all trace = " << m_iAllTraces;

   unsigned char head[SEGY_HEAD_BYTES + m_iBytes];
   for (i = 0; i < m_iAllTraces; i++)
   {
      // qDebug() << "read trace i+++++++++++++ " <<  i<< m_iBytes;
      //j = readTrace(m_piHead, (float *)m_pcBuf)// we cannot use m_pcBuf;
      //j = readTrace(m_piHead, (float *)buf);
      j= m_tp->read(head, SEGY_HEAD_BYTES + m_iBytes);
     // qDebug() << "read trace i+++++++++++ " << i << j;
      if (j < SEGY_HEAD_BYTES + m_iBytes) break;
      //ihd = m_piHead[hd - 1];
      ihd = getHeaderWord(hd, (int *)head);
	  //qDebug("i =ihd,hd=%d,%d,%d\n",i,ihd,hd); 
      if (ihd <=0)  break;
      if (i == 0) ioldhd = ihd;
      if (ihd != ioldhd)
      {
         // v1.02 update segy idx file
        //  qDebug() << "hd =  " << ihd << ioldhd;
         pidx.idx = idxhd;
         pidx.num = ichd;
         pidx.grp = ioldhd;
         file.write((char *)&pidx, sizeof(INDEX_INFO));
         //qDebug("write index=idx,num,grp = %d,%d,%d,%d,%d\n",idxhd,ichd,ioldhd,ihd,ioldhd);

         //	if(icchd == 30)
         //   	qDebug("s-idx, shot ,num ,idx=%d,%d,%d,%d,%d,%d\n",icchd,ioldhd,ichd,i,phead[7-1],phead[3-1]);
         icchd++;
         ichd = 0;
         idxhd = iccchd;

      }
      //if(i >90) exit(0);
      //qDebug("i, shot ,num ,idx=%d,%d,%d,%d,%d,%d\n",i,ihd,ichd,icchd,phead[7-1],phead[3-1]);
      ioldhd = ihd;
      ichd++; //counter in a gather
      iccchd++; // counter in file
   }
   //qDebug() << "hdend =  " << ihd << ioldhd;
   // last gather:
   pidx.idx = idxhd;
   pidx.num = ichd;
   pidx.grp = ioldhd;
   //qDebug("write index,last =idx,num,grp = %d,%d,%d\n",idxhd,ichd,ioldhd);
   file.write((char *)&pidx, sizeof(INDEX_INFO));
   //qDebug("file--size,at=%d,%d\n",file.size(),file.at());
   icchd++;
   // write the all gathers to the second int
   i = icchd;
   int ipp = file.pos();
   file.flush();
   file.seek(4);
   file.write((char *)&i, sizeof(int));
   //file.flush()
   file.seek(ipp);
   qDebug()  << "close write index,size= "<<file.size()<< ipp;
//	exit(1);
//close
   file.close();
   closeRead();
   return 0;
}
int QSegyADI::getIdxInfo()
{
   int i, hd, allgroups;

   QFile file;
   m_bIdxFile = false;
   //open file
   file.setFileName(m_strFilenameIdx);
   if (!file.open(QIODevice::ReadOnly)) return -1; //cannot open idx
                                                   //read and write
   file.read((char *)&i, sizeof(int)); //hd
   hd = i;
   file.read((char *)&i, sizeof(int)); // all groups
   allgroups = i;

   if (hd < 1 || hd > SEGY_HEAD_WORDS) return -1;
   if (allgroups < 1 || hd > SEGY_MAX_GROUPS) return -1;

 
   if (m_pIdx != NULL) delete[] m_pIdx;
   m_pIdx = new INDEX_INFO[allgroups + 10];
   int max = 0;
   for (i = 0; i < allgroups; i++)
   {
      file.read((char *)&m_pIdx[i], sizeof(INDEX_INFO));
      //qDebug("fileindex info = %d,%d,%d,%d\n",i,m_pIdx[i].idx,m_pIdx[i].num,m_pIdx[i].grp);
      if (max < m_pIdx[i].num) max = m_pIdx[i].num;
   }
//	qDebug("max = %d",max);
   m_iMaxTrsOfGather = max;
   m_iAllGathers = allgroups;
   m_iIdxHD = hd;
   file.close();
   m_iCurrentIdx = 0;
   qDebug() << "get index info start===========";
   qDebug() << "m_iMaxTrsOfGather = " << m_iMaxTrsOfGather;
   qDebug() << "m_iAllGather = " << m_iAllGathers;
   qDebug() << "m_iIdxHD = " << m_iIdxHD;
   qDebug() << "get index info end=============";
   m_bIdxFile = true;
   return 0;
}
QString  QSegyADI::getKey()
{
   QString str;
   str = "";
   if (m_iIdxHD == SHOT) str = "shot";
   if (m_iIdxHD == CMP) str = "cmp";
   if (m_iIdxHD == LINE) str = "line";
   if (m_iIdxHD == LINEC) str = "cross line";
   return str;
}
int QSegyADI::testGather(QString filen)
{
 
   int i,num,j;
 //  int SHOT,CMP,LINE,LINEC;
   int gather;
   num = 50;

   i = openRead(filen);
  // qDebug() << "ret openFile in testFather: " << i;
   if (i < 0) return -1;
   if (num > i) num = i;
   gather = -1;
  //read 50 traces:
    
   int shot,cmp,line,linec;
   int shot1,cmp1,line1,linec1;

   for (i = 0; i < num; i++)
   {
       qDebug() << i << m_piHead <<m_pcBuf ;
      j = readTrace(m_piHead, (float *)m_pcBuf);
       qDebug() << j;
      if (j <= 0) break;
      shot = getInt((unsigned char *)&m_piHead[ SHOT - 1]);
      cmp = getInt((unsigned char *)&m_piHead[CMP - 1]);
      line = getInt((unsigned char *)&m_piHead[LINE - 1]);
      linec = getInt((unsigned char *)&m_piHead[LINEC - 1]);
      qDebug() << "testdata =" << i << shot << cmp << line << linec;
      // next 2 trace header eaqual, we consider it is the gather header
      if (i >0)// not first;
      {
         if (shot == shot1 && shot >=0)
         {
            gather = SHOT;
           
            break;
         }
         if (cmp == cmp1 && cmp >=0)
         {
            gather = CMP;
         
            break;
         }
         if (line == line1 )
         {
            if (cmp == cmp1 + 1)
            {
               gather = LINE;
          
            }
            break;
         }
         if (linec == linec1 && linec >=0)
         {
            gather = LINEC;
       
            break;
         }
      }
      shot1 = shot;
      cmp1 = cmp;
      line1 = line;
      linec1 = linec;
	 
   }
   closeRead();
   
   m_iIdxHD = gather;
   return gather;
}

int QSegyADI::openReadFile(QString filename)
{
  // qDebug() << "openReadFile = " << filename;
    int id,idx;
    id = testGather(filename);
    qDebug() << "testgather ok " << id;
    if (id < 0 ) return -1; 

    idx = openRead(filename); 
    qDebug() << "indexfile gatherHD=" << m_bIdxFile << id;
    if (!m_bIdxFile) 
    {
        //createThIdxFile(filename,id);
         closeRead();
         createIdxFile(filename,id);
          idx = openRead(filename); 
    }
    return idx; 
}
 
int QSegyADI::openRead(QString filename)
{
   int i;

   char filen[200];
   strcpy(filen, filename.toUtf8().data());

   m_strFilename = filename;
   m_strFilenameIdx = filename + ".idx";
//	m_bDiskFlag = TRUE;


   QFile file;
   file.setFileName(m_strFilenameIdx);
   if (file.exists())
   {
      qDebug() << "file IDX = " << m_strFilenameIdx;
      getIdxInfo() ;
      #if 0 // rmmove to :getIdxInfo() ;
      if (getIdxInfo() == 0)
      {
         m_bIdxFile = true;
      }
      else
      {
         m_bIdxFile = false;
      }
      #endif
   }
   else
   {
      m_bIdxFile = false;
   }

   i = m_tp->open(filename);
   if (i < 0) return -1;
   //3200
   i = m_tp->read(m_pBuf3200, 3200);
   if (i != 3200) return -2;
   //400
   i = m_tp->read(m_pBuf400, 400);
   if (i != 400) return -3;

   // get 400 information
   //qDebug() << "get file info start ";
   i = getFileInfo();
  // qDebug() << "ret of getFileInfo in OpenFIle =" << i;
   if (i != 0) return -4;

   //return 0;


   long len;
   len = m_tp->size();

   m_iAllTraces = (len - 3600) / (SEGY_HEAD_BYTES + m_iBytes);
   qDebug() << " m_iAllTraces = " << m_iAllTraces;
   qDebug() << " m_iBytes of tracedata = " << m_iBytes;
   qDebug() << " SEGY_HEAD_BYTES = " << SEGY_HEAD_BYTES;
   qDebug() << " end of open read =========================";

   return m_iAllTraces;
}
/*
int QSegyADI::openRead(int iunit)
{
    int i;
//	m_bDiskFlag = FALSE;
    m_iIUnit = iunit;

    i = m_tp->open(iunit);

    if(i <0)
        return -1;
//3200
        i = m_tp->read(m_pBuf3200,TAPE_BLOCK);
    if(i != 3200)
        return -2;
//400
        i = m_tp->read(m_pBuf400,TAPE_BLOCK);
    if(i != 400)
        return -3;
// get 400 information

        i = getFileInfo();
    if(i !=0) return -4;
 
    return 0;
}
*/
int QSegyADI::openWrite(QString filename)
{
   int i;

   char filen[200];
   strcpy(filen, filename.toUtf8().data());
   i = m_tpo->open(filename, 1);
   if (i >= 0) return 0;
   else return i;

}
int QSegyADI::set3200()
{
   return set3200(m_pBuf3200);
}

int QSegyADI::set3200(unsigned char *buf)
{
   QString str = "C 1 CLIENT                  COMPANY             CREW NO.                        ";
   str += "C 2 LINE          AREA                          MAP ID.                         ";
   str += "C 3 REEL NO.         DAY-START OF REEL       YEAR      OBSERVER                 ";
   str += "C 4 INSTRUMENT: MFG          MODEL              SERIAL NO.                      ";
   str += "C 5 DATA TRACES/RECORD    AUXILIARY TRACES/RECORD       CDP FOLD                ";
   str += "C 6 SAMPLE INTERVAL     MS  SAMPLES/TRACE     BITS/IN     BYTES/SAMPLE          ";
   str += "C 7 RECORDING FORMAT     FORMAT THIS REEL      MEASUREMENT SYSTEM               ";
   str += "C 8 SAMPLE CODE: FLOATING PT   FIXED PT   FIXED PT-GAIN    CORELATED            ";
   str += "C 9 GAIN   TYPE: FIXED    BINARY     FLOATING POINT     OTHER                   ";
   str += "C10 FILTERS: ALIAS     HZ  NOTCH     HZ     BAND     HZ    SLOPE                ";
   str += "C11 SOURCE: TYPE           NUMBER/POINT   POINT INTERVAL                        ";
   str += "C12     PATTERN                     LENGTH         WIDTH                        ";
   str += "C13 SWEEP: START     HZ  END     HZ  LENGTH     MS  CHANNEL NO.    TYPE         ";
   str += "C14 TAPER: START LENGTH     MS  END LENGTH     MS  TYPE                         ";
   str += "C15 SPREAD: OFFSET        MAX DISTANCE         GROUP INTERVAL                   ";
   str += "C16 GEOPHONES: PER GROUP    SPACING     FREQUENCY     MFG      MODEL            ";
   str += "C17    PATTERN              LENGTH         WIDTH                                ";
   str += "C18 TRACES SORTED BY: RECORD      CDP     OTHER                                 ";
   str += "C19 MAP PROJECTION          ZONE ID        COORDINATE UNITS                     ";
   str += "C20 BEGINNING SHOTPOINT (OBSERVER'S):                                           ";
   str += "C21 ENDING SHOTPOINT    (OBSERVER'S):                                           ";
   str += "C22 DIRECTION OF LINE:                                                          ";
   str += "C23 COUNTY               STATE              SURVEY NO.                          ";
   str += "C24                                                                             ";
   str += "C25                                                                             ";
   str += "C26                                                                             ";
   str += "C27                                                                             ";
   str += "C28                                                                             ";
   str += "C29                                                                             ";
   str += "C30                                                                             ";
   str += "C31                                                                             ";
   str += "C32                                                                             ";
   str += "C33                                                                             ";
   str += "C34                                                                             ";
   str += "C35                                                                             ";
   str += "C36                                                                             ";
   str += "C37                                                                             ";
   str += "C38                                                                             ";
   str += "C39                                                                             ";
   str += "C40                                                                             ";

//	QMessageBox::information(0,"len", QString::number(str.length()));
   strncpy((char *)buf, str.toUtf8().data(), 3200);
   return 0;
}

//

//read
/*
0:ok
-1: read head error
*/
int QSegyADI::readTrace(int *head, float *buf)
{
   int iby, i;
   QString str;
   unsigned int *iip;
   iip = (unsigned int*)m_pcBuf;

  // iby = m_iBytes;
//read header:
   //i = m_tp->read(m_pcBuf, SEGY_HEAD_BYTES + iby);
  //  qDebug() << "start tp_read = ";
   i = m_tp->read(m_pcBuf, SEGY_HEAD_BYTES);
  // qDebug() << "tp_read ret = "<<i;
   if (i < 0) return i;
   else if (i == 0)
   {
      setStatus("EOF");
      return 0;
   }
   setMainHeader();
   // qDebug() << "start memcpy head = "<< head;
   memcpy(head, m_pcBuf, SEGY_HEAD_BYTES);

   iby = m_iBytes;

  // qDebug() << "start tp_read = ";
   //reade samples:
   i = m_tp->read(m_pcBuf+SEGY_HEAD_BYTES , iby);
  // qDebug() << "tp_read ret = "<<i;
   if (i < 0) return i;
   else if (i == 0)
   {
      setStatus("EOF");
      return 0;
   }
   else
   { // ok;
      setStatus("");
      // qDebug("ppppp2 = %d,%x,%x,%x,%x---%x,%x,%x",i,m_pcBuf[0],m_pcBuf[1],m_pcBuf[2],m_pcBuf[3],m_pcBuf[240],m_pcBuf[241],m_pcBuf[242]);     
/// here we got a error ,i donot know why: copy a crazy data ,240byte looped data buf[0] = buf[60] = buf[120]???
   //   if (i - SEGY_HEAD_BYTES  > 0) 
    //      memcpy(m_pcBuf, m_pcBuf + SEGY_HEAD_BYTES, i - SEGY_HEAD_BYTES);
      // qDebug("ppppp3 = %d,%x,%x,%x,%x---%x,%x,%x",i,m_pcBuf[0],m_pcBuf[1],m_pcBuf[2],m_pcBuf[3],m_pcBuf[240],m_pcBuf[241],m_pcBuf[242]);
   }
   unsigned char *mpp;
   mpp = m_pcBuf + SEGY_HEAD_BYTES;
   str = getStatus();
   int sams;
   sams = m_iSamples;
   if (str != "EOF") switch (m_iFormat)
      {
      case 1:
         ibm_to_float((int *)mpp, (int *)buf, sams, m_iEndian);
        // qDebug("ppppp4 = %d,%x,%x,%x,%x---%x,%x,%x",i,m_pcBuf[0],m_pcBuf[1],m_pcBuf[2],m_pcBuf[3],m_pcBuf[240],m_pcBuf[241],m_pcBuf[242]);
         break;
      case 2:
         int_to_float((int *)mpp, buf, sams, m_iEndian);
         break;
      case 3:
         short_to_float((short *)mpp, buf, sams, m_iEndian);
         break;
      case 4:
         int_to_float((int *)mpp, buf, sams, m_iEndian);
         break;
      case 5: //ieee
         ieee_to_float((float *)mpp, buf, sams, m_iEndian);
         break;
      }
 
   // qDebug("ppppp = %08X, %08X,%x,%x,%x,%x",iip[0],iip[60],m_pcBuf[0],m_pcBuf[1],m_pcBuf[2],m_pcBuf[3]);
   // qDebug()<< "format = " <<m_tp->pos() <<  i <<i - SEGY_HEAD_BYTES<< m_iFormat << m_iSamples << buf[0] << buf[1] << buf[60] << buf[61] ;

   return m_iSamples;
}
int QSegyADI::readGather(int *head, float *buf)
{
/*	return :trs:ok
        -1: errorhead[61]
    */
   int i,trs;
   trs =  m_infoGHD.iTrs + m_infoGHD.iAuxs;
   qDebug() << " QSegyADI::readGather trs = " << trs << head << buf<< m_iSamples;
   for (i = 0; i < trs; i++)
   {
      if (readTrace(head + i * SEGY_HEAD_WORDS, buf + i * m_iSamples) != m_iSamples) 
      {
          qDebug() << "read gather error trs in shot" << i << trs;
          return -1;
      }
   }
    qDebug() << "read gather ok =" <<trs;
   return trs;
}
int QSegyADI::readTraces(int *head, float *buf, int trs)
{
/*	return :0:ok
        -1: errorhead[61]
    */
   int i;
   for (i = 0; i < trs; i++)
   {
      //qDebug() << "inloop = " << i << trs << m_iSamples <<head << buf;
      if (readTrace(head + i * SEGY_HEAD_WORDS, buf + i * m_iSamples) != m_iSamples) return -1;
   }
   return 0;
}
/*
0: OK
-1: read error;
*/
int QSegyADI::write3200(unsigned char *buf)
{
   int i;
   //i = write(m_iUnitO,buf,3200);

   i = m_tpo->write(buf, 3200);
   if (i != 3200) return -1;
   return 0;
}
int QSegyADI::write400(unsigned char *buf)
{
   int i;
   //i = write(m_iUnitO,buf,400);

   i = m_tpo->write(buf, 400);
   if (i != 400) return -1;
   return 0;
}
int QSegyADI::write3200()
{
   int i;
   i = m_tpo->write(m_pBuf3200, 3200);
   if (i != 3200) return -1;
   return 0;
}
int QSegyADI::write400()
{
   int i;
   i = m_tpo->write(m_pBuf400, 400);
   if (i != 400) return -1;
   return 0;
}
int QSegyADI::writeTrace(int *head, float *buf)
{
   int i;

//head
   memcpy(m_pcBuf, head, SEGY_HEAD_BYTES); //donot swap ok,//head[0]
   i = m_tpo->write(m_pcBuf, SEGY_HEAD_BYTES);
   if (i != SEGY_HEAD_BYTES) return -1;

   //qDebug() << "after head,format = "<<m_iWriteFormat;
//traces
//use format 2:
   if (m_iWriteFormat == 2)
   {
      int *ibuf;
      ibuf = new int[m_iSamples];
      for (i = 0; i < m_iSamples; i++)
      {
         ibuf[i] = buf[i];
      }


      memcpy(m_pcBuf, ibuf, m_iSamples * sizeof(int));

      swap4(m_pcBuf, m_iSamples * sizeof(int));

      //i = write(m_iUnitO,m_pBuf,m_iSamples*sizeof(int));
      // qDebug() << "after swap = "<<m_iWriteFormat;

      i = m_tpo->write(m_pcBuf, m_iSamples * sizeof(int));

      if (i != int(m_iSamples *sizeof(int))) return -2;

      delete[] ibuf;

   }
   if (m_iWriteFormat == 5)
   {
     // qDebug() << "bf copy  i= "<<m_pcBuf<<m_iSamples*sizeof(float)<<m_iSamples;

      memcpy(m_pcBuf, buf, m_iSamples * sizeof(float));
      //qDebug() << "after copy  i= "<<m_pcBuf;
      swap4(m_pcBuf, m_iSamples * sizeof(float));
      //qDebug() << "after swap= "<<m_iWriteFormat;

      //i = write(m_iUnitO,m_pBuf,m_iSamples*sizeof(float));

      i = m_tpo->write(m_pcBuf, m_iSamples * sizeof(float));

      if (i != int(m_iSamples *sizeof(float))) return -3;
   }

   if (m_debug) for (i = 500; i < 500 + 20; i++) qDebug("i = %d,sample write = %f\n", i, buf[i]);

   m_iReelCounter++;
   m_iLineCounter++;

   return 0;
}
int QSegyADI::writeTraces(int *head, float *buf, int trs)
{
   int i;
   for (i = 0; i < trs; i++)
   {
      if (writeTrace(head + i * SEGY_HEAD_WORDS, buf + i * m_iSamples) != 0) return -1;
   }
   return 0;
}
int QSegyADI::getHeaderWord(int *head)
{

   return getInt((unsigned char *)head);
     
}
int QSegyADI::getHeaderWord(int hd,int *head)
{
   int *ip;
   ip = head + hd - 1;

   return getInt((unsigned char *)ip);   
}
/*
0: OK
-1: read error;
*/
int QSegyADI::closeRead()
{
   m_tp->close();
   return 0;
}
void QSegyADI::setWriteFormat(int f)
{
   if (f == 2 || f == 5) m_iWriteFormat = f;
   else m_iWriteFormat = 2;

}
int QSegyADI::closeWrite()
{

   m_tpo->close();
   return 0;
}
int QSegyADI::getBuf400(unsigned char *buf)
{
   memcpy(m_pBuf400, buf, 400);
   return 0;
}
/*
int QSegyADI::getFileInfo()
{
//start1:
         

//get job no //1-4
                        m_infoGHD.iJobNo = getInt(m_pBuf400);
//get line no //5-8
                        m_infoGHD.iLineNo = getInt(m_pBuf400 + 4);
//get reel no //9-12
                        m_infoGHD.iReelNo = getInt(m_pBuf400 + 8);
//get all trs/shot //13-14
                        m_infoGHD.iTrs = getShort(m_pBuf400 + 12);
//get aux trs/shot  //15-16
                        m_infoGHD.iAuxs = getShort(m_pBuf400 + 14);
//get si  //17 -18
                        m_infoGHD.iSi = getShort(m_pBuf400 + 16);
//get osi  //19-20
                        m_infoGHD.iOSi = getShort(m_pBuf400 + 18);
//get sams  //21-22
                        m_infoGHD.iSams = getShort(m_pBuf400 + 20);
//get osams  //23-24
                        m_infoGHD.iOSams = getShort(m_pBuf400 + 22);
//get format //25-26;1:float2:fix3:fix(2)4:fix
                        m_infoGHD.iFormat = getShort(m_pBuf400 + 24);
//get data type  //29-30; 1:shot,2:cmp,3:single stack,4:stack
                        m_infoGHD.iDataType = getShort(m_pBuf400 + 28);
//get amp type //53-54
                        m_infoGHD.iAmpType = getShort(m_pBuf400 + 52);
            if(m_infoGHD.iSi <=0) m_infoGHD.iSi = m_iOSi;

            if(m_infoGHD.iSi != 0)
                if(m_infoGHD.iSams <=0) m_infoGHD.iSams = m_iOLtr*1000/m_infoGHD.iSi;

            m_iSi = m_infoGHD.iSi;
            m_iLtr = m_iSi * m_infoGHD.iSams/1000;
            m_iFormat = m_infoGHD.iFormat;
            m_iSamples = m_infoGHD.iSams;



            if(m_iFormat == 3)
                m_iBytes =  m_iSamples*2;
            else
                m_iBytes =  m_iSamples*4;
            //if(m_debug)
            qDebug("si,ltr,format,sam=%d,%d,%d,%d\n",m_iSi,m_iLtr,m_iFormat,m_iSamples);
            
            m_strStatus = "OK";
            if(m_iSi <= 0) m_strStatus = "SI ERR";
            if(m_infoGHD.iSams <=0) m_strStatus = "SAMPLES ERR";
            if(m_iLtr <=0) m_strStatus = "LTR ERR";
            if(m_iFormat <=0) m_strStatus = "FORMAT ERR";
        //	if(m_infoGHD.iTrs <=0) m_strStatus = "TRS ERR";

            if(m_strStatus != "OK") return -1;
    //		return 0;
    return 0;
}
int QSegyADI::setFileInfo()
{

     memset(m_pBuf400,0,400);

//get job no //1-4
                        setInt((int *)(m_pBuf400 + 0) ,m_infoGHD.iJobNo);
//get line no //5-8
                        setInt((int *)(m_pBuf400 + 4) ,m_infoGHD.iLineNo);
//get reel no //9-12
                        setInt((int *)(m_pBuf400 + 8) ,m_infoGHD.iReelNo);
//get all trs/shot //13-14
                        setShort((short *)(m_pBuf400 + 12) ,m_infoGHD.iTrs);
//get aux trs/shot  //15-16
                        setShort((short *)(m_pBuf400 + 14) ,m_infoGHD.iAuxs);
//get si  //17 -18
                        setShort((short *)(m_pBuf400 + 16) ,m_infoGHD.iSi);
//get osi  //19-20
                        setShort((short *)(m_pBuf400 + 18) ,m_infoGHD.iOSi);
//get sams  //21-22
                        setShort((short *)(m_pBuf400 + 20) ,m_infoGHD.iSams);
//get osams  //23-24
                        setShort((short *)(m_pBuf400 + 22) ,m_infoGHD.iOSams);
//get format //25-26;1:float2:fix3:fix(2)4:fix
                        setShort((short *)(m_pBuf400 + 24) ,m_iWriteFormat);
//get data type  //29-30; 1:shot,2:cmp,3:single stack,4:stack
                        setShort((short *)(m_pBuf400 + 28) ,m_infoGHD.iDataType);
//get amp type //53-54:1:none,2:3:agc
                        setShort((short *)(m_pBuf400 + 52) ,m_infoGHD.iAmpType);
 
            m_iSi = m_infoGHD.iSi;
            m_iLtr = m_iSi * m_infoGHD.iSams/1000;
//			m_iFormat = m_infoGHD.iFormat;
            m_iSamples = m_infoGHD.iSams;


            m_strStatus = "OK";
            if(m_iSi <= 0) m_strStatus = "SI ERR";
            if(m_infoGHD.iSams <=0) m_strStatus = "SAMPLES ERR";
            if(m_iLtr <=0) m_strStatus = "LTR ERR";
//			if(m_iFormat <=0) m_strStatus = "FORMAT ERR";
        //	if(m_infoGHD.iTrs <=0) m_strStatus = "TRS ERR";
 
            if(m_strStatus != "OK") return -1;

            return 0;
}
*/
int QSegyADI::seekTR(int tridx)
{
   long len;
   if (tridx < 0 || tridx >= m_iAllTraces) return -1;
   len = 3600 + tridx * (SEGY_HEAD_BYTES + m_iBytes);
   //r = lseek(m_iUnit,len,SEEK_SET);
   if (m_tp->seek(len)) return 0;
   else return -2;

}
int QSegyADI::reSeek()
{
   int id;
   if (!m_bIdxFile) return -1;

   id = m_pIdx[m_iCurrentIdx].idx;
   if (seekTR(id) < 0) return -3;
   return 0;
}
int QSegyADI::seekGather(int grp)
{
   int i, id;
   if (!m_bIdxFile) return -1;
   id = -1;
   for (i = 0; i < m_iAllGathers;  i++)
   {
      if (m_pIdx[i].grp == grp)
      {
         id = i;
         break;
      }
   }
   if (id == -1) return -2; //not found group
   m_iCurrentIdx = id;
   if (reSeek() < 0) return -3;
   return 0;
}
int QSegyADI::seekGatherIdx(int idx)
{

   if (!m_bIdxFile) return -1;
   if (idx < 0 || idx >= m_iAllGathers) return -2;
   m_iCurrentIdx = idx;
   if (reSeek() < 0) return -3;
   return 0;

}
int QSegyADI::seekNextGather()
{

   if (!m_bIdxFile) return -1;
   if (m_iCurrentIdx + 1 >= m_iAllGathers) return -2;
   m_iCurrentIdx++;
   if (reSeek() < 0) return -3;
   return 0;
}
int QSegyADI::seekPreGather()
{

   if (!m_bIdxFile) return -1;
   if (m_iCurrentIdx <= 0) return -2;
   m_iCurrentIdx--;
   if (reSeek() < 0) return -3;
   return 0;
}
int QSegyADI::seekFirstGather()
{

   if (!m_bIdxFile) return -1;

   m_iCurrentIdx = 0;
   if (reSeek() < 0) return -3;
   return 0;
}
int QSegyADI::seekLastGather()
{

   if (!m_bIdxFile) return -1;

   m_iCurrentIdx = m_iAllGathers - 1;
   if (reSeek() < 0) return -3;
   return 0;
}
int QSegyADI::getIndexGather(int idx)
{
   if (idx <0 || idx >= m_iAllGathers)
   {
      return -1;
   }
   //qDebug() << "in ADI,get idx,gather =" << idx << m_pIdx[idx].grp;
   //for (int i = 0; i < m_iAllGathers;i++)
   //{
   //   qDebug()<< i  <<m_pIdx[idx].idx<< m_pIdx[idx].grp<<m_pIdx[idx].num;
   //}
   return  m_pIdx[idx].grp;
}

int QSegyADI::getGatherIndex(int gather)
{
   int i,shot,idx;
   shot = gather;
   idx = -1;
   	//find the index
	for(i = 0; i < m_iAllGathers ;i++)
	{
		if(shot == m_pIdx[i].grp)
		{
			idx = i;
         break;
		}
	}
   return idx;// -1 not fond;
}

void QSegyADI::ibm_to_float(int from[], int to[], int n, int endian)
/***********************************************************************
ibm_to_float - convert between 32 bit IBM and IEEE floating numbers
************************************************************************
Input::
from		input vector
to		output vector, can be same as input vector
endian		byte order =0 little endian (DEC, PC's)
                =1 other systems 
************************************************************************* 
Notes:
Up to 3 bits lost on IEEE -> IBM

Assumes sizeof(int) == 4

IBM -> IEEE may overflow or underflow, taken care of by 
substituting large number or zero

Only integer shifting and masking are used.
************************************************************************* 
Credits: CWP: Brian Sumner,  c.1985
*************************************************************************/
{
   register int fconv, fmant, i, t;
//	int a;

   for (i = 0; i < n; ++i)
   {

      fconv = from[i];

      /* if little endian, i.e. endian=0 do this */
      if (endian == 0) fconv = (fconv << 24) | ((fconv >> 24) & 0xff) |
               ((fconv & 0xff00) << 8) | ((fconv & 0xff0000) >> 8);

      if (fconv)
      {
         fmant = 0x00ffffff & fconv;
         /* The next two lines were added by Toralf Foerster */
         /* to trap non-IBM format data i.e. conv=0 data  */
         if (fmant == 0) fmant = 0;
//			err(" data are not in IBM FLOAT Format !");

         if (fmant != 0)
         {
            t = (int)((0x7f000000 & fconv) >> 22) - 130;
            while (!(fmant & 0x00800000))
            {
               --t;
               fmant <<= 1;
            }
            if (t > 254) fconv = (0x80000000 & fconv) | 0x7f7fffff;
            else if (t <= 0) fconv = 0;
            else fconv = (0x80000000 & fconv) | (t << 23) | (0x007fffff & fmant);
         }
         else fconv = 0;
      }

      to[i] = fconv;
   }

   return;
}

void QSegyADI::int_to_float(int from[], float to[], int n, int endian)
/****************************************************************************
Author:	J.W. de Bruijn, May 1995
****************************************************************************/
{
   register int i;

   if (endian == 0)
   {
      for (i = 0; i < n; ++i)
      {
         swap4((unsigned char *)&from[i], 4);
         to[i] = (float)from[i];
         //to[i] = getFloat((void*)from[i]);
      }
   }
   else
   {
      for (i = 0; i < n; ++i)
      {
         to[i] = (float)from[i];
      }
   }
}
void QSegyADI::ieee_to_float(float from[], float to[], int n, int endian)
/****************************************************************************
Author:	J.W. de Bruijn, May 1995
****************************************************************************/
{
   register int i;

   if (endian == 0)
   {
      for (i = 0; i < n; ++i)
      {
         swap4((unsigned char *)&from[i], 4);
         to[i] = (float)from[i];
         //to[i] = getFloat((void*)from[i]);
      }
   }
   else
   {
      for (i = 0; i < n; ++i)
      {
         to[i] = (float)from[i];
      }
   }
}
void QSegyADI::short_to_float(short from[], float to[], int n, int endian)
/****************************************************************************
short_to_float - type conversion for additional SEG-Y formats
*****************************************************************************
Author: Delft: J.W. de Bruijn, May 1995
Modified by: Baltic Sea Reasearch Institute: Toralf Foerster, March 1997 
****************************************************************************/
{
   register int i;

   if (endian == 0)
   {
      for (i = n - 1; i >= 0; --i)
      {
         swap2((unsigned char *)&from[i], 2);
         to[i] = (float)from[i];

      }
   }
   else
   {
      for (i = n - 1; i >= 0; --i)
      {
         to[i] = (float)from[i];
      }
   }
}

int QSegyADI::setTraceHeader(char * head)
{

    //fill main header;
   setInt((int *)(head + 1 - 1), m_iLineCounter); //1-4  hd1
   setInt((int *)(head + 5 - 1), m_iReelCounter); //5-8  hd2
   setInt((int *)(head + 9 - 1), m_infoMHD.iShot); //9-12 hd3
   setInt((int *)(head + 13 - 1), m_infoMHD.iTr); //13-16:hd4
   //qDebug() << "hd4 = " << m_infoMHD.iTr;
   //setShort((short *)(head + 15 - 1), m_infoMHD.iSams); //15-16 

   setInt((int *)(head + 21 - 1), m_infoMHD.iCMP); //21-24 hd6
   setInt((int *)(head + 25 - 1), m_infoMHD.iCMPTr); //25-26  

   setShort((short *)(head + 29 - 1), m_infoMHD.iType); //  29-30//hd9h
   setInt((int *)(head + 37 - 1), m_infoMHD.iOffset); //37-40  

   setInt((int *)(head + 41 - 1), m_infoMHD.iEleReceiver); //41-44
   setInt((int *)(head + 45 - 1), m_infoMHD.iEleShot); //45-48
   setInt((int *)(head + 49 - 1), m_infoMHD.iDepthShot); //49-52
   setInt((int *)(head + 53 - 1), m_infoMHD.iEleDReceiver); //53-56
   setInt((int *)(head + 57 - 1), m_infoMHD.iEleDShot); //57-60
   setInt((int *)(head + 61 - 1), m_infoMHD.iWaterShot); //61-64
   setInt((int *)(head + 65 - 1), m_infoMHD.iWaterReceiver); //65-68

    setShort((short *)(head + 69 - 1), m_infoMHD.iScaleEle); //69-70
    setShort((short *)(head + 71 - 1), m_infoMHD.iScaleXy); //71-72
   
    setInt((int *)(head + 73 - 1), m_infoMHD.iShotX); //73-76: SHOTx
    setInt((int *)(head + 77 - 1), m_infoMHD.iShotY); //77-80: SHOTy
    setInt((int *)(head + 81 - 1), m_infoMHD.iReceiverX); //81-84: rx
    setInt((int *)(head + 85 - 1), m_infoMHD.iReceiverY); //85-88: ry

    setShort((short *)(head + 95 - 1), m_infoMHD.iUpholeShot); //95-96
    setShort((short *)(head + 97 - 1), m_infoMHD.iUpholeReceiver); //97-98
    setShort((short *)(head + 99 - 1), m_infoMHD.iStaticsShot); //99-100
    setShort((short *)(head + 101 - 1), m_infoMHD.iStaticsReceiver); //101-102
    setShort((short *)(head + 103 - 1), m_infoMHD.iStaticsSum); //103-104
    setShort((short *)(head + 105 - 1), m_infoMHD.iLagA); //105-106
    setShort((short *)(head + 107 - 1), m_infoMHD.iLagB); //107-108
    setShort((short *)(head + 109 - 1), m_infoMHD.iDelay); //109-110


    setShort((short *)(head + 111 - 1), m_infoMHD.iMuteS); //111-112
    setShort((short *)(head + 113 - 1), m_infoMHD.iMuteE); //113-114


   
//	m_infoMHD.iReceiverIndex = getInt(m_pcBuf -1 + 73);//73-76: SHOTx
   setShort((short *)(head + 115 - 1), m_infoMHD.iLtr * 1000 / m_infoMHD.iSi); //115-116,samples:
   setShort((short *)(head + 117 - 1), m_infoMHD.iSi); //117-118,si
                                                       //
   setShort((short *)(head + 117 - 1), m_infoMHD.iScaleSt); //125-126  

   setInt((int *)(head + 181 - 1), m_infoMHD.iCMPX); //181-184 
   setInt((int *)(head + 185 - 1), m_infoMHD.iCMPY); //185-188 
   setInt((int *)(head + 189 - 1), m_infoMHD.iLine); //189-192 line
   setInt((int *)(head + 193 - 1), m_infoMHD.iLine); //193-196 line

   if (0)
   {
      qDebug("Segy header infor:\n");
      qDebug(" m_iLineCounter = %d\n", m_iLineCounter);
      qDebug(" m_iReelCounter = %d\n", m_iReelCounter);
      qDebug(" m_infoMHD.iShot =%d\n", m_infoMHD.iShot);
      qDebug(" m_infoMHD.iTr =%d\n", m_infoMHD.iTr);

      qDebug(" m_infoMHD.iCMP =%d\n", m_infoMHD.iCMP);
      qDebug(" m_infoMHD.iType =%d\n", m_infoMHD.iType);
      qDebug(" m_infoMHD.iOffset =%d\n", m_infoMHD.iOffset);
      qDebug(" m_iSamples =%d\n", m_infoMHD.iLtr * 1000 / m_infoMHD.iSi);
      qDebug(" m_infoMHD.iSi =%d\n", m_infoMHD.iSi);
   } //end of debug
   m_iSamples = m_infoMHD.iLtr * 1000 / m_infoMHD.iSi;
   m_iSI = m_infoMHD.iSi;
   m_iLTR = m_infoMHD.iLtr;

   return 0;

}

int QSegyADI::setTraceHeader()
{

   char head[SEGY_HEAD_BYTES];
   memset(head, 0, SEGY_HEAD_BYTES);
   setTraceHeader(head);
   memcpy((char *)m_piHead, head, SEGY_HEAD_BYTES);
   return 0;
}

int QSegyADI::writeTrace(float *buf)
{
   int i;
   setTraceHeader();
   //qDebug() << "after set header";
   i = writeTrace(m_piHead, buf);
   //qDebug() << "after write";
   return i;

}
/*
int QSegyADI::openAppend(QString file)
{
    char filen[200];
    QString filename;
    filename = file;
//	m_bDiskFlag = TRUE;

        strcpy(filen,filename.toUtf8().data());

        ofile.setFileName(filename);
 
        if(!ofile.open(QIODevice::openOnly|QIODevice::Append))
            return -1;

    return 0;
}
*/

int QSegyADI::setMainHeader()
{
   setMainHeader(m_infoMHD,m_pcBuf);
   m_infoMHD.iSi = m_iSI;
   m_infoMHD.iLtr = m_iLTR;
   m_iCurGather = getInt(m_pcBuf +( m_iIdxHD -1)*sizeof(int));
   return 0;
}
#if 0
int QSegyADI::setMainHeader()
{

//	unsigned char *p;
//fill main header;
   m_infoMHD.iCMP = getInt(m_pcBuf + 20); //21-24 hd6
   m_infoMHD.iLtr = m_iLTR;
   m_infoMHD.iTr = getInt(m_pcBuf + 12); //13-16:hd4
   m_infoMHD.iOffset = getInt(m_pcBuf + 36); //37-40 hd10
   m_infoMHD.iShot = getInt(m_pcBuf + 8); //9-12 hd3
   m_infoMHD.iSi = m_iSI;
   m_infoMHD.iType = getShort(m_pcBuf + 28); //1:seis other:aux29-30
   m_infoMHD.iValid = 1; //ok;

   m_infoMHD.iLine = getInt(m_pcBuf + 188); //189-192  line
   m_infoMHD.iLineY = getInt(m_pcBuf + 192); //193-196  crossline

   m_infoMHD.iCMPX= getInt(m_pcBuf + 180); //181-184
   m_infoMHD.iCMPY= getInt(m_pcBuf + 184); //185-188 
   m_infoMHD.iShotX= getInt(m_pcBuf + 72); //73-76
   m_infoMHD.iShotY= getInt(m_pcBuf + 76); //77-79
   m_infoMHD.iReceiverX= getInt(m_pcBuf + 80); //81-84
   m_infoMHD.iReceiverY= getInt(m_pcBuf + 84); //85-88

    m_infoMHD.iStaticsShot= getShot(m_pcBuf + 98); //99-100
    m_infoMHD.iStaticsReceiver= getShot(m_pcBuf + 100); //101-102
    m_infoMHD.iStaticsSum= getShot(m_pcBuf + 102); //103-104

   m_infoMHD.fShotLine = getInt(m_pcBuf  + 77- 1); //77-80: SHOTy
   m_infoMHD.fShotPoint = getInt(m_pcBuf  + 73- 1); //73-76: SHOTx
//	m_infoMHD.iShotIndex = getInt(m_pcBuf -1 + 73);//73-76: rx
   m_infoMHD.fReceiverLine = getInt(m_pcBuf + 85- 1 ); //85-88: ry
   m_infoMHD.fReceiverPoint = getInt(m_pcBuf + 81- 1 ); //81-84: rx
//	m_infoMHD.iReceiverIndex = getInt(m_pcBuf -1 + 73);//73-76: SHOTx


   m_iCurGather = getInt(m_pcBuf +( m_iIdxHD -1)*sizeof(int));
   //qDebug() << "m_infoMHD.iShot ,gather =" <<m_infoMHD.iShot << m_iCurGather << m_iIdxHD;
   
   return 0;

}
#endif
int QSegyADI::setMainHeader(MAINHD_INFO &mhd,unsigned char * hd)
{

//	unsigned char *p;
//fill main header;
   // qDebug() << "start setMainHeader hd =" <<hd;
   int sams;
   sams =  getShort(hd + 115-1); // 115-116
   m_ic ++;
   
   if (sams != m_iSamples)
   { 
       if (sams>=0 && sams <10000) // mybe the sams in header is crazy
       {
      
           qDebug() << "==========sample changed:" << m_iSamples << "to " << sams << "index tr: " << m_ic;
           m_iSamples = sams;

           if (m_iFormat == 3) m_iBytes =  m_iSamples * 2;
           else m_iBytes =  m_iSamples * 4; 
       }
   }

   mhd.iShot = getInt(hd + 8); //9-12 hd3
   mhd.iTr = getInt(hd + 12); //13-16:hd4
   mhd.iChannel= getInt(hd + 12); //13-16 
   //mhd.iShotTr = getInt(hd + 12); //13-16         
                                   //
   mhd.iCMP = getInt(hd + 20); //21-24 
   mhd.iCMPTr = getInt(hd + 24); //25-28
   mhd.iType = getShort(hd + 28); //1:seis other:aux29-30
   mhd.iValid = 1; //ok 
   //
   mhd.iOffset = getInt(hd + 36); //37-40 hd10

   mhd.iEleReceiver = getInt(hd + 40); //41-44   
   mhd.iEleShot= getInt(hd + 44); //45-48   
   mhd.iEleDReceiver = getInt(hd + 52); //53-56  
   mhd.iEleDShot = getInt(hd + 56); //57-60 
   mhd.iDepthShot = getInt(hd + 48); //49-52   
   mhd.iWaterShot= getInt(hd + 60); //61-64   
    mhd.iWaterReceiver= getInt(hd + 64); //65-68   

   mhd.iLine = getInt(hd + 188); //189-192  line
   mhd.iLineY = getInt(hd + 192); //193-196  crossline

   mhd.iScaleEle= getShort(hd + 68); //69-70
   mhd.iScaleXy= getShort(hd + 70); //71-72
   mhd.iShotX= getInt(hd + 72); //73-76
   mhd.iShotY= getInt(hd + 76); //77-79
   mhd.iReceiverX= getInt(hd + 80); //81-84
   mhd.iReceiverY= getInt(hd + 84); //85-88

   mhd.iUpholeShot= getShort(hd + 94); //95-96
    mhd.iUpholeReceiver= getShort(hd + 96); //97-98
    mhd.iStaticsShot= getShort(hd + 98); //99-100
    mhd.iStaticsReceiver= getShort(hd + 100); //101-102
    mhd.iStaticsSum= getShort(hd + 102); //103-104
    mhd.iLagA= getShort(hd + 104); //105-106
    mhd.iLagB= getShort(hd + 106); //107-108
    mhd.iDelay= getShort(hd + 108); //109-110

    mhd.iMuteS= getShort(hd + 110); //111-112
    mhd.iMuteE= getShort(hd + 112); //113-114
    mhd.iScaleSt= getShort(hd + 124); //125-126
  

//	mhd.iReceiverIndex = getInt(hd -1 + 73);//73-76: SHOTx

   mhd.iCMPX= getInt(hd + 180); //181-184
   mhd.iCMPY= getInt(hd + 184); //185-188 

 
   mhd.iSi = m_iSI;
   mhd.iLtr = m_iLTR;
   if(m_iIdxHD >0) 
       m_iCurGather = getInt(hd +( m_iIdxHD -1)*sizeof(int));
   //qDebug() << "mhd.iShot ,gather =" <<m_infoMHD.iShot << m_iCurGather << m_iIdxHD;
   
   return 0;

}


int QSegyADI::getFileInfo()
{
//start1:
//get format //25-26;1:float2:fix3:fix(2)4:fix
   m_infoGHD.iFormat = getShort(m_pBuf400 + 24);
   if (m_infoGHD.iFormat <= 0 || m_infoGHD.iFormat > 5)
   {
      //setEndian(1);
      m_infoGHD.iFormat = getShort(m_pBuf400 + 24);
   }

//get job no //1-4
   m_infoGHD.iJobNo = getInt(m_pBuf400);//1-4
//get line no //5-8
   m_infoGHD.iLineNo = getInt(m_pBuf400 + 4);//5-8
//get reel no //9-12
   m_infoGHD.iReelNo = getInt(m_pBuf400 + 8);//9-12
//get all trs/shot //13-14
   m_infoGHD.iTrs = getShort(m_pBuf400 + 12);//13-4
//get aux trs/shot  //15-16
   m_infoGHD.iAuxs = getShort(m_pBuf400 + 14);//15-16
//get si  //17 -18
   m_infoGHD.iSi = getShort(m_pBuf400 + 16); ///1000;
//get osi  //19-20
   m_infoGHD.iOSi = getShort(m_pBuf400 + 18); //)/1000;
//get sams  //21-22
   m_infoGHD.iSams = getShort(m_pBuf400 + 20);
//get osams  //23-24
   m_infoGHD.iOSams = getShort(m_pBuf400 + 22);
   //15-16:format


//cmp cover num
   m_infoGHD.iFold = getShort(m_pBuf400 + 26);//27-28
//get data type  //29-30; 1:shot,2:cmp,3:single stack,4:stack
   m_infoGHD.iDataType = getShort(m_pBuf400 + 28);

   m_infoGHD.iVerCode = getShort(m_pBuf400 + 30);          //31-32 yzy
   m_infoGHD.iStartFreq = getShort(m_pBuf400 + 32);        //33-34 yzy
   m_infoGHD.iEndFreq = getShort(m_pBuf400 + 34);          //35-36 yzy
   m_infoGHD.iScanWindowTime = getShort(m_pBuf400 + 36);   //37-38 yzy
   m_infoGHD.iScanType = getShort(m_pBuf400 + 38);         //39-40
   m_infoGHD.iScanChTrc = getShort(m_pBuf400 + 40);       //41-42
   m_infoGHD.iTScanStartTime = getShort(m_pBuf400 + 42);   //43-44
   m_infoGHD.iTScanEndTime = getShort(m_pBuf400 + 44);     //45-46
   m_infoGHD.iTScanType = getShort(m_pBuf400 + 46);        //47-48
   m_infoGHD.iRelTrs = getShort(m_pBuf400 + 48);           //49-50
   m_infoGHD.iGain = getShort(m_pBuf400 + 50);             //51-52 yzy
                                                           //	    m_infoGHD.iAmpType= GetShort(m_pBuf400 + 52);          //53-54;1:non,2:serphial 3,agc,4:other
   m_infoGHD.iUnit = getShort(m_pBuf400 + 54);              //55-56
   m_infoGHD.iDirection = getShort(m_pBuf400 + 56);        //57-58
   m_infoGHD.iFrDirection = getShort(m_pBuf400 + 58);      //59-60
//get amp type //53-54
   m_infoGHD.iAmpType = getShort(m_pBuf400 + 52);


   m_iSI = m_infoGHD.iSi;
   m_iLTR = m_iSI * m_infoGHD.iSams;
   m_iFormat = m_infoGHD.iFormat;
   m_iSamples = m_infoGHD.iSams;

   qDebug() << "file infor start============================";
   qDebug() << "iFormat = " << m_infoGHD.iFormat;
   qDebug() << "m_infoGHD.iJobNo = " << m_infoGHD.iJobNo;
   qDebug() << "m_infoGHD.iReelNo = " << m_infoGHD.iReelNo;
   qDebug() << "m_infoGHD.iTrs = " << m_infoGHD.iTrs;
   qDebug() << "m_infoGHD.iAuxs = " << m_infoGHD.iAuxs;
   qDebug() << "m_infoGHD.iSi = " << m_infoGHD.iSi;
   qDebug() << "m_infoGHD.iOSi = " << m_infoGHD.iOSi;
   qDebug() << "m_infoGHD.iSams = " << m_infoGHD.iSams;
   qDebug() << "m_infoGHD.iOSams = " << m_infoGHD.iOSams;
   qDebug() << "m_infoGHD.iFold= " << m_infoGHD.iFold;
    qDebug() << "m_infoGHD.iFormat= " << m_infoGHD.iFormat;
   qDebug() << "m_infoGHD.iDataType = " << m_infoGHD.iDataType;
   qDebug() << "m_infoGHD.iVerCode = " << m_infoGHD.iVerCode;      
   qDebug() << "m_infoGHD.iStartFreq = " << m_infoGHD.iStartFreq;
   qDebug() << "m_infoGHD.iEndFreq = " << m_infoGHD.iEndFreq;
   qDebug() << "m_infoGHD.iScanWindowTime = " << m_infoGHD.iScanWindowTime;
   qDebug() << "m_infoGHD.iScanType = " << m_infoGHD.iScanType;
   qDebug() << "m_infoGHD.iScanChTrc = " << m_infoGHD.iScanChTrc;
   qDebug() << "m_infoGHD.iTScanStartTime= " << m_infoGHD.iTScanStartTime;
   qDebug() << "m_infoGHD.iTScanType= " << m_infoGHD.iTScanType;
   qDebug() << "m_infoGHD.iTScanStartTime= " << m_infoGHD.iTScanStartTime;
   qDebug() << "m_infoGHD.iTScanStartTime= " << m_infoGHD.iTScanStartTime;
   qDebug() << "m_infoGHD.iRelTrs = " << m_infoGHD.iRelTrs;
   qDebug() << "m_infoGHD.iPer = " << m_infoGHD.iPer;
   qDebug() << "m_infoGHD.iDirection = " << m_infoGHD.iDirection;
   qDebug() << "m_infoGHD.iFrDirection = " << m_infoGHD.iFrDirection;
   qDebug() << "m_infoGHD.iAmpType = " << m_infoGHD.iAmpType;
 
   qDebug() << "file infor end ============================";


   if (m_iFormat == 3) m_iBytes =  m_iSamples * 2;
   else m_iBytes =  m_iSamples * 4;
   qDebug() << "m_iBytes m_iSI,m_iLTR,m_iFormat,m_iSamples = " <<  m_iBytes << m_iSI <<m_iLTR <<m_iFormat << m_iSamples;
   m_strStatus = "OK";
   if (m_iSI <= 0) m_strStatus = "SI ERR";
   if (m_infoGHD.iSams <= 0) m_strStatus = "SAMPLES ERR";
   if (m_iLTR <= 0) m_strStatus = "LTR ERR";
   if (m_iFormat <= 0) m_strStatus = "FORMAT ERR";
   //	if(m_infoGHD.iTrs <=0) m_strStatus = "TRS ERR";
   qDebug() << "startus = " << m_strStatus;
   if (m_strStatus != "OK") return -1;
   //		return 0;
   return 0;
}
int QSegyADI::setFileInfo()
{

   memset(m_pBuf400, 0, 400);

//get job no //1-4
   setInt((int *)(m_pBuf400 + 0), m_infoGHD.iJobNo);
//get line no //5-8
   setInt((int *)(m_pBuf400 + 4), m_infoGHD.iLineNo);
//get reel no //9-12
   setInt((int *)(m_pBuf400 + 8), m_infoGHD.iReelNo);
//get all trs/shot //13-14
   setShort((short *)(m_pBuf400 + 12), m_infoGHD.iTrs);
//get aux trs/shot  //15-16
   setShort((short *)(m_pBuf400 + 14), m_infoGHD.iAuxs);
//get si  //17 -18
   setShort((short *)(m_pBuf400 + 16), m_infoGHD.iSi );
//get osi  //19-20
   setShort((short *)(m_pBuf400 + 18), m_infoGHD.iOSi );
//get sams  //21-22
   setShort((short *)(m_pBuf400 + 20), m_infoGHD.iSams);
//get osams  //23-24
   setShort((short *)(m_pBuf400 + 22), m_infoGHD.iOSams);
//get format //25-26;1:float2:fix3:fix(2)4:fix
   setShort((short *)(m_pBuf400 + 24), m_iWriteFormat);
//get data type  //29-30; 1:shot,2:cmp,3:single stack,4:stack
   setShort((short *)(m_pBuf400 + 28), m_infoGHD.iDataType);
//get amp type //53-54:1:none,2:3:agc
   setShort((short *)(m_pBuf400 + 52), m_infoGHD.iAmpType);

   m_iSI = m_infoGHD.iSi;
   m_iLTR = m_iSI * m_infoGHD.iSams;
//			m_iFormat = m_infoGHD.iFormat;
   m_iSamples = m_infoGHD.iSams;
   qDebug() << "m_iSamples = " <<m_iSamples;



   m_strStatus = "OK";
   if (m_iSI <= 0) m_strStatus = "SI ERR";
   if (m_infoGHD.iSams <= 0) m_strStatus = "SAMPLES ERR";
   if (m_iLTR <= 0) m_strStatus = "LTR ERR";

   if (m_strStatus != "OK") return -1;


   return 0;
}
SEGYTR240 QSegyADI::buildHead(int *p)
{
   int buf1[60];

   memcpy((void *)buf1, (void *)p, 60 * sizeof(int));
   swap4((unsigned char *)buf1, SEGY_HEAD_BYTES);
   SEGYTR240 th;

   unsigned char *buf = (unsigned char *)buf1;

   swap4(buf, 28);
   swap2(buf + 28, 8);
   swap4(buf + 36, 32);
   swap2(buf + 68, 4);
   swap4(buf + 72, 16);
   swap2(buf + 88, 92);
   swap4(buf + 180, 20);
   swap2(buf + 200, 4);
   swap4(buf + 204, 4);
   swap2(buf + 208, 12);
   swap4(buf + 220, 8);
   swap2(buf + 228, 12);

   memcpy((void *)&th, (void *)buf, 240);

//	th.num_trace_line = GetInt(buf);

   return th;
}


void QSegyADI::buildHead(int *buf, SEGYTR240 th)
{
   //int buf1[60];

   memcpy((void *)buf, (void *)&th, 240);

   swap4((unsigned char *)buf, 28);
   swap2((unsigned char *)buf + 28, 8);
   swap4((unsigned char *)buf + 36, 32);
   swap2((unsigned char *)buf + 68, 4);
   swap4((unsigned char *)buf + 72, 16);
   swap2((unsigned char *)buf + 88, 92);
   swap4((unsigned char *)buf + 180, 20);
   swap2((unsigned char *)buf + 200, 4);
   swap4((unsigned char *)buf + 204, 4);
   swap2((unsigned char *)buf + 208, 12);
   swap4((unsigned char *)buf + 220, 8);
   swap2((unsigned char *)buf + 228, 12);

   swap4((unsigned char *)buf, SEGY_HEAD_BYTES);
}
#if 0
main(int argc ,char ** argv)
{
    QApplication app(argc,argv);
    QSegyADI sgy;
    sgy.openReadFile("s175.sgy");
    sleep(5);
    qDebug() << "end------------------";
    app.exec();
}
#endif
