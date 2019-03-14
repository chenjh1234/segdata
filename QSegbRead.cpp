// QSegbRead.cpp: implementation of the QSegbRead class.
//
//////////////////////////////////////////////////////////////////////

#include "QSegbRead.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QSegbRead::QSegbRead() : QSegCommon()
{
   Init();
}

QSegbRead::~QSegbRead()
{

}
void QSegbRead::Init()
{
//	m_iIbytes = 65000;
   memset((void *)&m_infoMHD, 0, sizeof(MAINHD_INFO));
   m_piFormat[0] = 0200;

   m_iFormatNum = 1;
   m_iLastRBytes = 0;
   m_iRbytes = 0;
   m_fMP = 0;
   //m_iDataHDBytes = 20;
   m_iDataBytes = 0;
   m_iHeadBytes = TAPE_BLOCK;
   m_ic = 0;
   m_ica = 0;
   m_icFile  = 0;
   m_icShot  = 0;
   m_iOffsetBytes = 0; 0x4da34; //
   m_iOffsetBytes = SEGD_DISK_OFFSET; //
   m_iTraceGapBytes = 0;
   m_iShotsInFile = 0;
   strcpy(m_tif8, "TIF8");
   m_bTif8 = false;
   m_pPre = 0;
   m_pNext = 0;
   m_pThis = 0;
   _1st = true;

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
int QSegbRead::InitTape(QString file)
{
   int iret;
   QStringList list;
   list.append(file);
   iret =  InitTape(list);
   //qDebug()<< "trs In InitTape ="<< m_iTrs<<m_iAux<<m_iTrsAll;
   return iret;

}


int QSegbRead::InitTape(QStringList filelist)
{
   int iret;
//	int iunit;
// get unit
//	m_iIUnit = iunit;
//open tape
   m_listFiles = filelist;
   m_iAllFiles = m_listFiles.size();

   iret = NextFile();
   //qDebug() <<"Next file return from initTape = "<< iret;
   if (iret == -3 || iret == -2)
   {
      SetOffset(m_iOffsetBytes);
      m_tp->close();
      m_icFile = 0;
      iret = NextFile();
   }
   //qDebug()<< "trs In InitTape ="<< m_iTrs<<m_iAux<<m_iTrsAll;

   return iret;
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
int QSegbRead::GetFileInfo()
{
   int i, iret, bytes;
//	m_iTrsInFile = 0;
   QString str;

   bytes = m_iHeadBytes;
   // qDebug() << "bytes in fileinfo1 = " <<m_pPre<<m_pThis<<m_pNext <<bytes;

   // maybe 1st
   iret = m_tp->read(m_pcBuf, bytes);
   //qDebug() << "read tape =" << iret << bytes;
   if (iret == 0) return iret;
   else if (iret < 0) return -1;
   else if (iret > 0)
   {

//get file number: BCD://1-2

      m_infoGHD.iFileNo = BCD2Int(m_pcBuf, 4);



//get format:3-4
      m_infoGHD.iFormatCode = BCD2Int(m_pcBuf + 3 - 1, 4);
      if (m_infoGHD.iFormatCode != 200)
      {
         qDebug() << "format code !=200" << m_infoGHD.iFormatCode;
         return -1;
      }

//iManufacturerID: 13 :
      m_infoGHD.iManufacturerID = BCD2Int(m_pcBuf + 13 - 1, 2);


//get bytes/scan:11-12h ;
      m_infoGHD.iBytesPerScan = BCD2Int(m_pcBuf + 11 - 1, 3);
//get si:12L
//m_infoGHD.iSi = (float)m_pcBuf[qw-1]/16 * 1000;//increment 1/16
      m_infoGHD.iSi = BCD2Int(m_pcBuf + 12 - 1, 1, false) * 1000;

// get ltr:17
      m_infoGHD.iLtr = BCD2Int(m_pcBuf + 17 - 1, 2) * 1000;



//--------------------is segd format?? check---------------
//is segd format??
      int flag = -1;
      // check format code:
      for (i = 0; i < m_iFormatNum; i++)
      {
         if (m_infoGHD.iFormatCode == m_piFormat[i]) flag = 1;
      }

      //if(flag == -1) return -2 ;//not a segb,format code is not in range

//--------------------------------
      if (m_infoGHD.iSi <= 0) m_infoGHD.iSi = m_iOSi;
      if (m_infoGHD.iLtr <= 0) m_infoGHD.iLtr = m_iOLtr;

      m_iLTR = m_infoGHD.iLtr;
      m_iSI = m_infoGHD.iSi;
      m_iFormat = m_infoGHD.iFormatCode;
      if (m_iSI != 0) m_iSamples = m_iLTR * 1000 / m_iSI + 1;


      //get the for para: scan type,channel sets,channel number,channel type;

      int trsall, aux, trs;
      //4: scan code = "0xFDFDFDFF";
      // aux bytes£»10£º
      trs = (m_infoGHD.iBytesPerScan - 4 - 10) / (5 * 2) * 4;
      aux = 0;

      trsall = trs + aux;
      m_iTrs = trs;
      m_iAux = aux;
      m_iTrsAll = trsall;
      m_iShot = m_infoGHD.iFileNo;
      m_ic = 0;
      m_ica = 0;
      //qDebug() <<"ttt="<< m_infoGHD.iFileNo <<bytes<<m_iShot<<this;

      //debug:


      int headbytes;
      //24+J, J  = (No. Bytes/Scan)¨C(No. of Bytes for Binary Gain and Sync)+(2 Bytes)

      //headbytes =  24 + trs *2 + 6*2 +22;
      headbytes =  24 + m_infoGHD.iBytesPerScan - (4 + trs / 2) + 2 + HEAD_GAP;

      m_iHeadBytes = headbytes; //gaps after header =HEAD_GAP;
      m_iTrailBytes =  0;
      m_iScanBytes = m_infoGHD.iBytesPerScan;
      m_iDataBytes = m_iSamples * m_infoGHD.iBytesPerScan;
      m_iShotBytes = m_iHeadBytes + m_iDataBytes;
      if (DEBUG)
      {
         //qDebug("File Infor = ===\n");
         qDebug("iFileNo = %d\n", m_infoGHD.iFileNo);
         qDebug("iFormatCode = %d\n", m_infoGHD.iFormatCode);
         qDebug("iBytesPerScan = %d\n", m_infoGHD.iBytesPerScan);
         qDebug("Ltr = %d\n", m_iLTR);
         qDebug("iSi = %d\n", m_iSI);
         qDebug() << "trs,aux,alltre =" << m_iTrs << m_iAux << m_iTrsAll;
         qDebug("iSamples= %d\n", m_iSamples);
         qDebug() << "head,data,scan shot bytes=" << m_iHeadBytes << m_iDataBytes << m_iScanBytes << m_iShotBytes;
      } //end of debug


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
int QSegbRead::ReadShot(float *buf, int sams)
{
   int iret, i, j, n, ipp;
   QString str, str1;
   int iskiped = 0;
   unsigned char *pbuf, *pp, *ppstart;
   int g[4], q[4], s;
   unsigned int   *ip, ic;
   float ff;
   short ss;
   s = 1;
//file info :header:
   iret = GetFileInfo();
   if (iret <= 0) return iret;
   //qDebug() << "ishot = "<<m_iShot;
//buffer:
   pbuf = new unsigned char[m_iDataBytes];
//read:whole shot
   m_iRbytes = m_tp->read(pbuf, m_iDataBytes);
   if (m_iRbytes != m_iDataBytes)
   {
      qDebug() <<  "read truncate in ReadShot";
   }
   iret = m_iRbytes / m_iSamples * m_iScanBytes;
   iret = m_iSamples;
// get sync code:
   if (_1st)
   {
      strncpy((char *)syncCode, (char *)pbuf, 4);
      _1st = false;
      //qDebug() << "sync_code = " << str.sprintf("%0X%X%X%X",syncCode[0],syncCode[1],syncCode[2],syncCode[3]);
   }

   //decode:
   pp = pbuf;
   int ii;
   ii = 0;
   for (ic = 0; ic < sams; ic++)
   {
      //checkh sync:
      //qDebug() << "ic ===" << ic;
      while (!(pp[0] == syncCode[0] && pp[1] == syncCode[1] && pp[2] == syncCode[2] && pp[3] == syncCode[3]))
      {
         pp++;
         if (pp >= pbuf + m_iRbytes - 4) break;
         //qDebug() << "skip= " << ii << " offset =" << str.sprintf("%0X", pp - pbuf);
         ii++;

      }

      pp = pp + 14; //skip the synccode(4)) and 10 bytes auxCode;

      for (i = 0; i < m_iTrs / 4; i++) // all traces of the same sample
      {
         //pp = pbuf + 14 + ic*m_iScanBytes + 2*5*i ;//10 bytes for 4 samples
         // pp = pp + 2*5*i;
         //for g
         for (n = 0; n < 4; n++)
         {
            if (n / 2 * 2 == n) //even:
            {
               g[n] = (pp[n / 2] & 0xf0) >> 4;
            }
            else //odd:
            {
               g[n] = pp[n / 2] & 0x0f;

            }
            //if(g[n] == 0xff)  g[n] = 0;
            //qDebug() <<n << g[n];
            g[n] = 15 - g[n]; //yes 2**(-n)* 2**15(amplifyed by 2**15 times)

         }
         //qDebug()<< "after exp"<<i<<ic;
         pp = pp + 2; // skip gain;

         for (n = 0; n < 4; n++)
         {
            ss = getShort(pp);
            if (ss < 0)
            {
               ss = -((~ss) & (~1));
            }

            ff = ss * pow((float)2, (float)g[n]);

            ipp = ic  + sams * i * 4 + n * sams;
            // qDebug() << "ff ="<< ff;
            buf[ipp] = ff;
            //  qDebug() << "ipp ="<< ipp << ic<<  i <<n<<sams;
            pp = pp + 2;
         }

         //qDebug()<< "after amp" <<i<<ic<<pp-pbuf<<m_iDataBytes;
      } //end of one scan bytes
        //qDebug() << "after scan =" << str.sprintf("%d", pp - pbuf);
      if (pp >= pbuf + m_iRbytes - 4) break;
   } // end of all samples;
     // qDebug() << "ishot0 = "<<m_iShot;
   delete[] pbuf;
   //qDebug() << "ishot1 = "<<m_iShot;
   m_iFFID = m_iShot;
   return iret;
}

/*=============== fill main header
1:get info from data header
2:is the trace seismic data or Aux
3: fill main header 

return :data header bytes(include extender bytes);
*/

int QSegbRead::SetOffset(int off)
{
   m_iOffsetBytes = off;
   return 0;
}
int QSegbRead::CloseTape()
{
   m_tp->rewind();
   m_tp->unload();
   return m_tp->close();
}



int QSegbRead::GetTrs()
{

   return m_iTrs;
}
int QSegbRead::GetAuxes()
{

   return m_iAux;
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


void QSegbRead::F0015_to_float(short from[], float to[], int len)
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


void QSegbRead::F8015_to_float(short from[], float to[], int len)
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


void QSegbRead::F8022_to_float(char from[], float to[], int len)
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


void QSegbRead::F8024_to_float(short from[], float to[], int len)
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


void QSegbRead::F8036_to_float(unsigned char from[], float to[], int len)
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


void QSegbRead::F8038_to_float(short from[], float to[], int len)
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

void QSegbRead::F8042_to_float(char from[], float to[], int len)

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


void QSegbRead::F8044_to_float(short from[], float to[], int len)

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


void QSegbRead::F8048_to_float(short from[], float to[], int len)

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


void QSegbRead::F8058_to_float(short from[], float to[], int len)

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


float QSegbRead::GetMP(unsigned char *mp)
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

int QSegbRead::NextFile()
{
   qint64 fsize;
   int shots;

   //  qDebug() << "icfile =" <<m_icFile ;
// not the first time:
   if (m_icFile != 0)
   {
      m_tp->close();
      if (m_icFile >= m_iAllFiles) return 0; // complete
   }

// next file :first file is 0;

   m_file =  m_listFiles[m_icFile];
   m_icFile++;
//         qDebug() <<"Filename ==== " << m_file << m_icFile<<m_iAllFiles ;
// open

   if (!m_tp->open(m_file))
   {
      qDebug() << "open err " << m_file;
      return -1; //cannot open
   }

// offset
   m_tp->seek(m_iOffsetBytes);

   // qDebug() << "this in Nextfile1 = "<<m_pThis << m_iOffsetBytes;

// read the file header;
   int iret;
   iret = GetFileInfo();
   //qDebug()<< "trs In Next File1 ="<< m_iTrs<<m_iAux<<m_iTrsAll;
   // qDebug() << "iret from get file info = "<<iret << m_iShotBytes;
   if (iret == -1) return -2; //read err
   else if (iret == 0) return -2; //read err
   else if (iret == -2) return -3; // not segd
   else if (iret > 0)
   {
      // first time read ,we donot know the real data header bytes;
      //the data bytes;

      m_ic = 0;
      m_ica = 0;
// shots in the file
// ???????

      // m_tp->seek(m_iShotBytes);
      m_tp->seek(m_iOffsetBytes); //2015.3.2

      return iret;
   }
   else return -4;

}
