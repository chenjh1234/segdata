// QSegdRead.cpp: implementation of the QSegdRead class.
//
//////////////////////////////////////////////////////////////////////

#include "QSegdRead.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QSegdRead::QSegdRead():QSegCommon()
{
	Init();
}

QSegdRead::~QSegdRead()
{

}
void QSegdRead::Init()
{
//	m_iIbytes = 65000;
	memset((void*)&m_infoMHD,0,sizeof(MAINHD_INFO));
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
	//m_piFormat[10] = 8058;
	m_iFormatNum = 9;
	m_iLastRBytes = 0;
	m_iRbytes = 0;
	m_fMP =0;
	m_iDataHDBytes = 20;
	m_iDataBytes =0;
	m_iHeadBytes = TAPE_BLOCK;
	m_ic = 0;
	m_ica = 0;
	m_icFile  =0;
	m_iOffsetBytes = 4*1024*1024;
	m_iTraceGapBytes = 0;
	m_iShotsInFile = 0;
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


int QSegdRead::InitTape(QStringList filelist)
{
	int iret;
//	int iunit;
// get unit
//	m_iIUnit = iunit;
//open tape
        m_listFiles = filelist;
        m_iAllFiles = m_listFiles.size();
	m_icFile = 0;
	iret = NextFile();
//2008.6.5:
// default :offset = 4*1024*1024;
// for support offset = 0
// read err -2:
// if the offset point to thr end of file,it will return read err -2
// update 2010.4.21,
//	if(iret == -3)
	if(iret == -3 ||iret == -2 )
	{
		SetOffset(0);
                m_tp->close();
		m_icFile = 0;
		iret = NextFile();
	}

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
/**======================================================================
	ReadRecord

	1: read a tape record
	2:set status:EOF,EOT,ERR,OK

@return :
	bytes read;
	0:EOT:complete
	<0 error;

*/
int QSegdRead::InitTape(QString file)
{
    QStringList list;
    list.append(file);
    return InitTape(list);

}

int QSegdRead::ReadRecord()
{
	int bytes,chsets,ltr,ic,i;
	// ltr from the general header:
	ltr = m_infoGHD.iLtr;
	chsets = m_infoGHD.iMaxChSets;// number of channel sets for each scan type

	ic = 0;
	// ltr: from this channel set
	for(i = 0; i < chsets; i ++)
	{
		ic += m_pinfoSHD[i].iChs;
		if( m_ic < ic)
		{
			if(m_pinfoSHD[i].iEndTime != 0)
				ltr = m_pinfoSHD[i].iEndTime;
			break;
		}
	}
// add 2008.6.7 for func = 13 ,have one extra sample(4 byte)
// for Sercel 2008.6.27 for m_iTraceGapBytes
 
	m_iDataBytes = ltr*1000/m_infoGHD.iSi * m_fFormatBytes + m_iTraceGapBytes;	

	bytes = m_iDataHDBytes + m_iDataBytes;

 
                m_iRbytes = m_tp->read(m_pcBuf,bytes);
	if(m_debug)
		qDebug("bytes,dataDH,dataBytes,returnBytes = %d,%d,%d\n",bytes,m_iDataHDBytes ,m_iDataBytes,m_iRbytes);

		if(m_iRbytes <0)
		{//err
			  m_strStatus = "ERR";
		}
		else if(m_iRbytes == bytes)
		{//ok
			m_strStatus = "OK";
		}
		else if(m_iRbytes ==0)
		{//end of this file
			//when read is not enough,we ignore the last read;
			m_strStatus = "EOT";
			return 0;
		}
		else //return is not enough;
		{//end of this file
			//when read is not enough,we got the last read;
			//m_strStatus = "OK";// 2008.8
			m_strStatus = "EOT";
		}
		m_ic ++;
//		m_iCurrentTrace = m_ic;

		if(m_ic == m_iTrsAll)
		{
			m_strStatus = "EOF";
		//2008.11.3 :for trace
//			m_ic = 0;
//			m_ica = 0;

		}
 
		return m_iRbytes;
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
	int i,iret,i1,bytes;
//	m_iTrsInFile = 0;
	bytes = m_iHeadBytes;
        iret = m_tp->read(m_pcBuf,bytes);
	if(iret ==0)
		return iret;
	else if(iret <0)
		return -1;
	else if(iret >0)
	{
//get file number: //1-2
		if(m_pcBuf[0] != 0xff) 
			m_infoGHD.iFileNo = BCD2Int(m_pcBuf,4);
		else
			m_infoGHD.iFileNo = -1;

//get format:3-4
		m_infoGHD.iFormatCode = BCD2Int(m_pcBuf+3-1,4);

//get additional gen header:12h
		m_infoGHD.iAddGHD = BCD2Int(m_pcBuf+12-1,1);
//		iManufacturerID: 17 ,add 2008.6.7
		m_infoGHD.iManufacturerID = BCD2Int(m_pcBuf+17-1,2);
// add 2008.6
		if(m_infoGHD.iManufacturerID == 13)
			m_iTraceGapBytes = 4;

//get bytes/scan:20-22=0;
		m_infoGHD.iBytesPerScan = BCD2Int(m_pcBuf+20-1,6);
//get si:23,bin 
		m_infoGHD.iSi = (float)m_pcBuf[23-1]/16 * 1000;//increment 1/16
//int iPolarity;        //24h:4 bit	
		m_infoGHD.iPolarity =  *(m_pcBuf + 24-1)&(unsigned char)0xf0>>4;
		
//get file type:26h(Record Type)
		m_infoGHD.iFiletype = BCD2Int(m_pcBuf+26-1,1);
// get ltr26l,27
		float f;
		if(int(m_pcBuf[26-1] & 0x0f )!=int( 0x0f))
		{
			i1= BCD2Int(m_pcBuf+26-1,3,false);
			//m_infoGHD.iLtr = i1*100;//why *100, yes ok,see document
			// update 2007.10
			f = i1*0.1*1.024*1000;//increment 1.024 ,bcd:00.0
			m_infoGHD.iLtr = f;	
		}
		else
			m_infoGHD.iLtr = -1;

//get scan types:28
		m_infoGHD.iScanTypes = BCD2Int(m_pcBuf+28-1,2,true);
//get (Chan Sets/Scan Type)  max ch sets:29

		if(m_pcBuf[29-1] != 0xff) 
			m_infoGHD.iMaxChSets = BCD2Int(m_pcBuf+29-1,2,true);
		else
			m_infoGHD.iMaxChSets = -1;

//get Skew blocks:30
		m_infoGHD.iSkew = BCD2Int(m_pcBuf+30-1,2,true);
//get extended header:31
		if(m_pcBuf[31-1] != 0xff) 
			m_infoGHD.iExtended = BCD2Int(m_pcBuf+31-1,2,true);
		else
			m_infoGHD.iExtended = -1;

//get external header:32
		if(m_pcBuf[32-1] != 0xff) 
			m_infoGHD.iExternal = BCD2Int(m_pcBuf+32-1,2,true);
		else
			m_infoGHD.iExternal = -1;


//--------------------is segd format?? check---------------
   //is segd format??
		int flag = -1;
		// check format code:
		for(i = 0;i < m_iFormatNum;i++)
		{
			if(m_infoGHD.iFormatCode == m_piFormat[i]) flag = 1;
		}
		
		if(flag == -1) return -2 ;//not a segd,format code is not in range
		m_fFormatBytes =float(m_infoGHD.iFormatCode - m_infoGHD.iFormatCode/10*10)/2.0;
//----------------------general header #2:----------------------------------------------------- 
//  general header #2:

   //file number: #2:1-3,bin
		unsigned char *p;
		p = m_pcBuf+32;
		if(m_infoGHD.iAddGHD >0)
		{

			if(m_infoGHD.iFileNo == -1)
			{
				//m_infoGHD.iFileNo = BCD2Int(m_pcBuf+32,6,true);
                                m_infoGHD.iFileNo = getInt3(p);
			}
			 

  //Extended Channel Sets/Scan Type  #2:4-5,bin
 			if(m_infoGHD.iMaxChSets  == -1)
			{
                                m_infoGHD.iMaxChSets = getShort(p + 4 -1);
			}
			 
//get extended header: #2:6-7 bin 
			if(m_infoGHD.iExtended  == -1)
			{
                                m_infoGHD.iExtended = getShort(p + 6 -1);
			}
			 
//get external  header: #2:8-9 bin 
			if(m_infoGHD.iExternal  == -1)
			{
                                m_infoGHD.iExternal = getShort(p + 8 -1);
			}
			 

//get segd version: #2:11-12 : 	 
                         m_infoGHD.version = getFloat16(p + 11 -1);

//get trailer     : #2:13-14 : 	 
                         m_infoGHD.iTrailer = getShort(p + 13 -1);

	//ltr:          #2 15-17,bin 
			if(m_infoGHD.iLtr == -1)
			{
			//	m_infoGHD.iLtr = GetInt(m_pcBuf+32 +14-1);
			//	m_infoGHD.iLtr = m_infoGHD.iLtr & 0x00ffffff;
                                m_infoGHD.iLtr = getInt3(p + 15 - 1);
			}
			 
		}
	
//-------------------get shot line point in header #3--------------
		p = m_pcBuf + 32 *2;

		if(m_infoGHD.iAddGHD >1)//#3
		{
			
	//shotline: integer :4-6:,7-8
                        m_infoGHD.fShotLine = getSignedFloat32(p + 4 -1);
	//shotpoint:integer :9-11,12-13
                        m_infoGHD.fShotPoint = getSignedFloat32(p + 9 -1);
	//shotpoint index: integer :14
			m_infoGHD.iShotIndex = *(p + 14-1);
	//int iPhaseControl;    //15
			m_infoGHD.iPhaseControl = *(p + 15-1);
	//	int iTypeVibrator ;   //16
			m_infoGHD.iTypeVibrator = *(p + 16-1);
	//	int iPhaseAngle;      //17-18 : signed short
                        m_infoGHD.iPhaseAngle = getSignedShort(p + 17 -1);
	//	int iSourceSetNO;     //20;
			m_infoGHD.iSourceSetNO = *(p + 20-1);

		}
//--------------------------------
		if(m_infoGHD.iSi <=0) m_infoGHD.iSi = m_iOSi;
		if(m_infoGHD.iLtr <=0 ) m_infoGHD.iLtr = m_iOLtr;

                m_iLTR= m_infoGHD.iLtr;
                m_iSI = m_infoGHD.iSi;
		m_iFormat = m_infoGHD.iFormatCode;
                if(m_iSI !=0)
                        m_iSamples = m_iLTR*1000/m_iSI;
//----------------------------------------------------------
	// the scan type and the channel sets header

		int j;
		float mp;
		int begin;  
		//get the for para: scan type,channel sets,channel number,channel type;
		int chsetsBytes ,loopEnd;
		int trsall,aux,trs;

		begin = 32 + m_infoGHD.iAddGHD*32;
		chsetsBytes = m_infoGHD.iScanTypes * m_infoGHD.iMaxChSets * 32 ;
		loopEnd = begin + chsetsBytes;
		
		p =  m_pcBuf+begin;
		j = 0;
		trs = 0;
		aux =0;
 
		for(i = begin ;i < loopEnd ; i= i+32)
		{
			//scan type           1:bcd
			m_pinfoSHD[j].iScanTypeNumber = BCD2Int(p + j*32,2,true);
			//channel sets number 2:bcd
			m_pinfoSHD[j].iChSetNumber = BCD2Int(p + j*32 + 2 -1 ,2,true);
			//int iStartTime;      //3-4: bin start time
                        m_pinfoSHD[j].iStartTime = getShort(p + j*32 + 3-1)*2;//increment 2
			//int iEndTime;        //5-6 bin end time
                        m_pinfoSHD[j].iEndTime = getShort(p + j*32 + 5-1)*2;//increment 2
			
			//* mp//7-8 ; 7:-3~-10,8:s,4~-2;
			mp = GetMP(p+j*32 + 7-1);
			m_pinfoSHD[j].mp = mp;
			 
			if(m_pinfoSHD[j].iScanTypeNumber == 0 ||
				m_pinfoSHD[j].iChSetNumber == 0)
			{
				//j++;
				break;
			}
			//channel numbers 9-10
			m_pinfoSHD[j].iChs = BCD2Int(m_pcBuf+begin+j*32 +8,4,true);
			//channel type 11h
			m_pinfoSHD[j].iChType = BCD2Int(m_pcBuf+begin+ j*32 +11 - 1,1);//1:seis9:aux
			if(m_pinfoSHD[j].iChType == 1)
				trs += m_pinfoSHD[j].iChs;
			else
				aux += m_pinfoSHD[j].iChs;
			
			//int iGain;           //12l;bin
			m_pinfoSHD[j].iGain = *(m_pcBuf+begin+ j*32 +12-1)&(unsigned char)0x0f; 
			//int iAF;             //13-14;bcd; Alias Filter Frequency
			m_pinfoSHD[j].iAF = BCD2Int(m_pcBuf+begin+ j*32 +13-1,4); 
			//int iAS;             //15-16;bcd; Alias Filter Slope
			m_pinfoSHD[j].iAS = BCD2Int(m_pcBuf+begin+ j*32 +15-1,4); 
			//int iLC;             //17-18;bcd; Low Cut Filter
			m_pinfoSHD[j].iLC = BCD2Int(m_pcBuf+begin+ j*32 +17-1,4); 
			//int iLS;             //19-20;bcd; Low Cut Filter Slope
			m_pinfoSHD[j].iLS = BCD2Int(m_pcBuf+begin+ j*32 +19-1,4); 
			//int iNF1;             //21-22;bcd; First Notch Filter
			m_pinfoSHD[j].iNF1 = BCD2Int(m_pcBuf+begin+ j*32 +21-1,4); 
			//int iNF2;             //23-24;bcd; Second Notch Filter
			m_pinfoSHD[j].iNF2 = BCD2Int(m_pcBuf+begin+ j*32 +23-1,4); 
			// iNF3;             //25-26;bcd; Third Notch Filter
			m_pinfoSHD[j].iNF3 = BCD2Int(m_pcBuf+begin+ j*32 +25-1,4); 
			//intint iVS;              //30;bin; Vertical Stack
			m_pinfoSHD[j].iVS = *(m_pcBuf+begin+ j*32 +30-1); 
			//int iCAB;             //31;bin; Streamer No.
			m_pinfoSHD[j].iCAB = *(m_pcBuf+begin+ j*32 +31-1);
			//int iARY;             //32;bin; Array Forming
			m_pinfoSHD[j].iARY = *(m_pcBuf+begin+ j*32 +32-1);

			j++;	
		}
		m_iSHDs = j;

		trsall = trs + aux;
		m_iTrs = trs;
		m_iAux = aux;
		m_iTrsAll = trsall;



		//debug:
		if(m_debug)
		{
		qDebug("File Infor = ===\n");
		qDebug("iFileNo = %d\n", m_infoGHD.iFileNo);
		qDebug("iFormatCode = %d\n", m_infoGHD.iFormatCode);
		qDebug("iAddGHD = %d\n", m_infoGHD.iAddGHD   );      
		qDebug("iBytesPerScan = %d\n", m_infoGHD.iBytesPerScan);
		qDebug("iSi = %d\n", m_infoGHD.iSi);
		qDebug("iFiletype = %d\n", m_infoGHD.iFiletype);
	    qDebug("iLtr  = %d\n", m_infoGHD.iLtr);
		qDebug("iScanTypes = %d\n", m_infoGHD.iScanTypes);
		qDebug("iMaxChSets; = %d\n", m_infoGHD.iMaxChSets);
		qDebug("Channel sets table heads: = %d\n",m_iSHDs);
		qDebug("extended : = %d\n",m_infoGHD.iExtended);
		qDebug("external : = %d\n",m_infoGHD.iExternal);
	
		for(i = 0 ;i < m_iSHDs;i++)
		{
			qDebug(" no=%d, scantype = %d. channel sets = %d,trs = %d, time = %d,type = %d\n",
				i,
				m_pinfoSHD[i].iScanTypeNumber,
				m_pinfoSHD[i].iChSetNumber,
				m_pinfoSHD[i].iChs,
				m_pinfoSHD[i].iEndTime,
				m_pinfoSHD[i].iChType);
		}
		}//end of debug

		int headbytes;
		headbytes = (1	+ m_infoGHD.iAddGHD 
						+ m_infoGHD.iScanTypes * m_infoGHD.iMaxChSets
						+ m_infoGHD.iSkew
						+ m_infoGHD.iExtended
						+ m_infoGHD.iExternal)*32;
		m_iHeadBytes = headbytes;
		m_iTrailBytes = m_infoGHD.iTrailer *32;

	}//end of iret>0;


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
int QSegdRead::ReadTrace(float *buf,int sams)
{
	int iret;
	QString str,str1;
	int iskiped = 0;
// read a block		 
	ReadRecord();
	
        str = getStatus();
	//if(str == "OK" )//for tape
	//if(str == "OK" || str == "EOF")//for diskfile
	//2008.8
	if(str == "OK" || str == "EOF"||str == "EOT")//for diskfile
	{
		iret = ConvertData(buf,sams);
	}
    //else //if tape
	if(str == "EOF" || str == "DEOF")
	{
		// read next shot:
		m_ic = 0;
		m_ica = 0;

		iret = GetFileInfo();
		if(iret == -1) 
		{
			m_strStatus = "HERR";
			return -1;
		}
	    else if(iret == 0) 
		{
			//2008.5.12: for disk file:
			//is the file end,for read 0 byte when GetFileInfo()
			//double eof end of the tape
			//m_strStatus = "DEOF";
			iret = NextFile();
                        setStatus("EOT");//
			return iret; //==0, complete ,no file can read
			             // >0,eot;		 		 
		}
		else if(iret == -2) // not segd
		{
			m_strStatus = "NSEGD";
			return -3;
		}
		 
	}//end of eof
	else if(str == "EOT" || str == "EOTERR")
	{
		//open next segd file:
		iret = NextFile();
		//m_strStatus = "EOTERR"	;
		//return -4;
                setStatus("EOT");
		return iret;//==0,complete
	}
	else if(str == "ERR" ||str == "SERR")
	{
		return -5;// diskfile;
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
int QSegdRead::ConvertData(float *buf,int sams)
{
	int len,len1;

	SetMainHeader();

//	convert
        if(m_iSI !=0)
                len = m_iLTR*1000/m_iSI;

	if(len >0)
		if(len >sams) len = sams;
	else
		len = sams;

//	len1 = m_iLastRBytes;
//	if(len1 == 0)// for first time read;
//	{
		len1 = m_iRbytes;
//	}

	switch(m_iFormat)
	{
	case 8015:
            swap2(m_pcBuf + m_iDataHDBytes,len1 - m_iDataHDBytes);
		F8015_to_float ( (short*) (m_pcBuf + m_iDataHDBytes), buf, len);
		break;
	case 8022:
		
		F8022_to_float ( (char*) m_pcBuf + m_iDataHDBytes, buf, len);
		break;
	case 8024:
                swap2(m_pcBuf + m_iDataHDBytes,len1 - m_iDataHDBytes);
		F8024_to_float ( (short*) (m_pcBuf + m_iDataHDBytes), buf, len);
		break;
	case 8042:
		F8042_to_float ( (char*) (m_pcBuf + m_iDataHDBytes), buf, len);
		break;
	case 8044:
                swap2(m_pcBuf + m_iDataHDBytes,len1 - m_iDataHDBytes);
		F8044_to_float ( (short*) (m_pcBuf + m_iDataHDBytes), buf, len);
		break;
	case 8048:
                swap2(m_pcBuf + m_iDataHDBytes,len1 - m_iDataHDBytes);
		F8048_to_float ( (short*) (m_pcBuf + m_iDataHDBytes), buf, len);
		break;
	case 8058:
                swap2(m_pcBuf + m_iDataHDBytes,len1 - m_iDataHDBytes);
		F8058_to_float ( (short*) (m_pcBuf + m_iDataHDBytes), buf, len);
		break;
	case 8036:
	//2008.6.12 add 8036,8038	 
		F8036_to_float ( (unsigned char*) (m_pcBuf + m_iDataHDBytes), buf, len);
		//F8036_to_float (unsigned char from[], float to[], int len)
		break;
	case 8038:
                swap2(m_pcBuf + m_iDataHDBytes,len1 - m_iDataHDBytes);
		F8038_to_float ( (short*) (m_pcBuf + m_iDataHDBytes), buf, len);
		break;
	}

// mp processing: for_mp
	int i;
	for( i = 0; i < len ;i++);
	{
		buf[i] = buf[i] * pow(2,m_fMP);
	}
 

	return 0;

}
/*=============== fill main header
1:get info from data header
2:is the trace seismic data or Aux
3: fill main header 

return :data header bytes(include extender bytes);
*/
int QSegdRead::SetMainHeader()
{
	int i;
	int iFile ;
	int iScan ;
	int iCHsets;
	int iTr;
	int iExHDs;
	float fRLine,fRPoint,iRIndex;
	int iSamples;
	int iSensorType;
	 

	//int len = m_iRbytes;

//first header:---------20bytes-----------------------------------

	// file: 1-2 ; bcd
	if(m_pcBuf[0] != 0xff)
		iFile = BCD2Int(m_pcBuf +1 -1 ,4,true);//1-2
	else
		iFile = -1;
	// scan type: 3;bcd
	iScan = BCD2Int(m_pcBuf +3 -1 ,2,true);     //3
	// channel sets: 4;bcd
	if(m_pcBuf[0] != 0xff)
		iCHsets = BCD2Int(m_pcBuf +3 ,2,true);   //4
	else
		iCHsets = -1;

	// channel trace  number 5-6 ;bcd
	iTr = BCD2Int(m_pcBuf + 5 -1 ,4,true);       //5-6
	// 7-9:bcd;  first time word                    7-9
	// extended header :10 ;bcd
	iExHDs = BCD2Int(m_pcBuf +10 -1  ,2,true);//10
	//11: bin sample skew 
	//12: bin, trace edit
	//13-15:bin; time break window
	//16-17: bin;extended channel set number
	// 18-20: binextended file number:18-20
	if(iCHsets == -1)
                iCHsets = getShort(m_pcBuf +16 - 1);//16-17

	if(iFile == -1)
                iFile = getInt3(m_pcBuf +18 -1 );//18-20
// find the trace type,mp

	int iType = -1;
	for(i = 0;i <m_iSHDs ; i++)
	{
		if(m_pinfoSHD[i].iScanTypeNumber == iScan &&
		   m_pinfoSHD[i].iChSetNumber    ==  iCHsets)
		{
			iType = m_pinfoSHD[i].iChType;
			//for_mp
 			m_fMP  = m_pinfoSHD[i].mp; 
			break;
		}
	}
	if(iType !=1) iType = 0;
	//cjh: here we can do some extra info:to covert segd to segy
	m_iDataHDBytes =  20 + iExHDs*32;
//extended header ----------------32---------------------------------
 
	if(iExHDs >0)
	{
	//receiver line: 1-3: int or 11-15: 3int+2fraction
		unsigned char *p;
		p = m_pcBuf + 20  -1 +1;//1-3
	// 2006.4: ||-->&&
		if(*p == 0xff && *(p+1) ==0xff && *(p+2)==0xff)
		{
			p = m_pcBuf + 20  -1 +11;// 11-15
                        fRLine = getSignedFloat32(p);
		}
		else//1-3;
		{
                        i = getSignedInt3(p);
			fRLine = i;
		}
	//receiver point4-6: int or 16-20: 3int+2fraction
		p = m_pcBuf + 20  -1 +4;//4-6
		if(*p == 0xff && *(p+1) ==0xff && *(p+2)==0xff)
		{
			p = m_pcBuf + 20  -1 +16;// 16-20
                        fRPoint = getSignedFloat32(p);
		}
		else//4-6;
		{
                        i = getSignedInt3(p);
			//2006.4:fRLine->fRPoint
			fRPoint = i;
		}
	//iReceiver point Index: 7:bin
		p = m_pcBuf + 20  -1 +7;//7
		iRIndex = *((char *)p);
	//isamples: 8-10 :bin
		p = m_pcBuf + 20  -1 +8;//8-10
                iSamples = getInt3(p);

		//11-15:receiver line(extened)
		//16-20:receiver point(extened)
		//21:bin;sensor type
		p = m_pcBuf + 20  -1 +21;//21
		iSensorType = (unsigned char)*p;

	}
	//2008.10.24: counter the seismic and auxiliary individually
	if(iType == 1)//seismic
	{
		m_iCurrentTrace = m_ic - m_ica;
	}
	else//auxiliary:
	{
		m_ica++;
		m_iCurrentTrace = m_ica;
	}
	
//fill main header;
	m_infoMHD.iShot = iFile;
	m_infoMHD.iTr = m_iCurrentTrace ;
	m_infoMHD.iChannel = iTr;
	m_infoMHD.iType = iType;//1:seis 0:aux
	m_infoMHD.iCMP = 0;	
        m_infoMHD.iLtr = m_iLTR;
	m_infoMHD.iOffset = 0;
        m_infoMHD.iSi = m_iSI;
	m_infoMHD.iValid =1;//ok;
	m_infoMHD.fShotLine = m_infoGHD.fShotLine ;
	m_infoMHD.fShotPoint = m_infoGHD.fShotPoint;
	m_infoMHD.iShotIndex = m_infoGHD.iShotIndex ;
	m_infoMHD.fReceiverLine = fRLine;
	m_infoMHD.fReceiverPoint = fRPoint;
	m_infoMHD.iReceiverIndex = iRIndex;
	m_infoMHD.iTrsAll = m_iTrsAll;
	m_infoMHD.iTrs = m_iTrs;
	m_infoMHD.iAux = m_iAux;


	if(m_debug)
	qDebug("data header info:file = %d,tr= %d,type = %d, si = %d,ltr = %d,%d,headers = %d\n",
                        iFile,m_ic,iTr,iType,m_iSI,m_iLTR,m_iDataHDBytes);


	return 0;
}
int QSegdRead::SetOffset(int off)
{
	m_iOffsetBytes = off;
	return 0;
}
int QSegdRead::CloseTape()
{
        m_tp->rewind( );
        m_tp->unload();
        return m_tp->close( );
}

 

int QSegdRead::GetTrs()
{
	int i;
	int id = 0;
	for(i = 0;i <m_iSHDs ;i++)
	{
		id = id + m_pinfoSHD[i].iChs;
	}
	return id;
}
int QSegdRead::GetAuxes()
{
	int i;
	int id = 0;
	for(i = 0;i <m_iSHDs ;i++)
	{
		if(m_pinfoSHD[i].iChType !=1)
		 id = id + m_pinfoSHD[i].iChs;
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
 

void QSegdRead::F0015_to_float (short from[], float to[], int len)
{
 register int i;
 register short ex1_4;
 int expo;
 short fraction;

 for (i = 0; i < len; i += 4) {
	ex1_4 = GET_S(from);
 	expo = ((ex1_4 >> 12) & 0x0F) - 15;
	fraction = GET_S(from);
	if (fraction < 0) fraction = -((~fraction)&(~1));
 	*(to++) = ldexp((double) fraction, expo);

 	expo = ((ex1_4 >> 8) & 0x0F) - 15;
	fraction = GET_S(from);
	if (fraction < 0) fraction = -((~fraction)&(~1));
 	*(to++) = ldexp((double) fraction, expo);

 	expo = ((ex1_4 >> 4) & 0x0F) - 15;
	fraction = GET_S(from);
	if (fraction < 0) fraction = -((~fraction)&(~1));
 	*(to++) = ldexp((double) fraction, expo);

 	expo = (ex1_4 & 0x0F) - 15;
	fraction = GET_S(from);
	if (fraction < 0) fraction = -((~fraction)&(~1));
 	*(to++) = ldexp((double) fraction, expo);
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


void QSegdRead::F8015_to_float (short from[], float to[], int len)
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


void QSegdRead::F8022_to_float (char from[], float to[], int len)
{
 register int i;
 register int ex1_4;
 int expo;
 short fraction;

 for (i = 0; i < len; i ++) {
      ex1_4 = GET_C(from);
      expo = ((ex1_4 >> 3) & 14) - 4;
      fraction = ex1_4 & 15;
	if (ex1_4 & 128) fraction = -(15^fraction);
 	*(to++) = ldexp((double) fraction, expo);
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


void QSegdRead::F8024_to_float (short from[], float to[], int len)
{
 register int i;
 register int ex1_4;
 int expo;
 short fraction;

 for (i = 0; i < len; i ++) {
      ex1_4 = GET_S(from);
      expo = ((ex1_4 >> 11) & 14) - 12;
      fraction = ex1_4 & 4095;
      if (ex1_4 & 32768) fraction = -(4095^fraction);
      *(to++) = ldexp((double) fraction, expo);
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


void QSegdRead::F8036_to_float (unsigned char from[], float to[], int len)
{
 register int i;
 register long int ival;

 for (i = 0; i < len; i ++) {
      ival = GET_UC(from);
      ival <<= 8; ival |= GET_UC(from);
      ival <<= 8; ival |= GET_UC(from);
      if(ival > 8388607) ival -= 16777216;
      *(to++) = (float) ival;
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


void QSegdRead::F8038_to_float (short from[], float to[], int len)
{
 int i;
 long int ex1_4;
 long int ex2_4;
 long int value;

 for (i = 0; i < len; i ++) {
      ex1_4 = GET_S(from);
      ex2_4 = GET_S(from);
      value = (ex1_4<<16) | (ex2_4&65535);
      *(to++) = (float) value;
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

void QSegdRead::F8042_to_float (char from[], float to[], int len)

{
 register int i;
 register int ex1_4;
 int expo;
 short fraction;

 for (i = 0; i < len; i ++) {
      ex1_4 = GET_C(from);
      expo = ((ex1_4 >> 3) & 12) - 5;
      fraction = ex1_4 & 31;
      if (ex1_4 & 128) fraction = -fraction;
      *(to++) = ldexp((double) fraction, expo);
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


void QSegdRead::F8044_to_float (short from[], float to[], int len)

{
 register int i;
 register int ex1_4;
 int expo;
 short fraction;

 for (i = 0; i < len; i ++) {
      ex1_4 = GET_S(from);
      expo = ((ex1_4 >> 11) & 12) - 13;
      fraction = ex1_4 & 8191;
      if (ex1_4 & 32768) fraction = -fraction;
      *(to++) = ldexp((double) fraction, expo);
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


void QSegdRead::F8048_to_float (short from[], float to[], int len)

{
 register int i;
 register int ex1_4;
 int expo;
 long int fraction;

 for (i = 0; i < len; i ++) {
      ex1_4 = GET_S(from); 
      expo = ((ex1_4 >> 6) & 508) - (24+256);
      fraction = ex1_4 & 255;
      fraction <<= 16; fraction |= (GET_S(from)&65535);
      if (ex1_4 & 32768) fraction = -fraction;
      *(to++) = ldexp((double) fraction, expo);
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


void QSegdRead::F8058_to_float (short from[], float to[], int len)

{
 register int i;
 register int ex1_4, ex2_4;
 int expo;
 long int fraction;

 for (i = 0; i < len; i ++) {
      ex1_4 = GET_S(from); 
      ex2_4 = GET_S(from);
      expo = ((ex1_4 >> 7) & 255);
      fraction = ex1_4 & 127;
      fraction <<= 16; fraction |= (ex2_4&65535);
      if(expo) fraction |= 8388608;
      else fraction <<= 1;
      if (ex1_4 & 32768) fraction = -fraction;
      *(to++) = ldexp((double) fraction, expo-(23+127));
 }
} 


float QSegdRead::GetMP(unsigned char *mp)
{
	float f7,f8,ff;
	int s ;
	int i,i7,i8;
	unsigned char ch;
	strncpy((char *)&ch,(char *)mp,1);

// byte 7th:
	f7 = 0;
	i7 = -3;
	for( i = 0; i < 8; i++)
	{
		ff = 0;
                if((ch & (unsigned char)((float)pow((float)2,(float)7-i))) != 0)
		{
                        ff = pow((float)2,(float)i7 - i);
			f7 = f7 + ff;
		}
	}

// byte 8th:
	strncpy((char *)&ch,(char *)mp +1,1);
	f8 = 0;
	i8 = 4;
	for( i = 0; i < 8-1 ; i++)
	{
		ff = 0;
                if((ch & (unsigned char)(pow((float)2,(float)7-1 -i))) != 0)
		{
                        ff = pow((float)2,(float)i8 - i);
			f8 = f8 + ff;
		}
	}

//sign:
	s  = 0;
        if((ch & (unsigned char)(pow((float)2,(float)7))) != 0) s =1;
// final:
	if(s == 0)
		ff = f8+f7;
	else
		ff = -1*(f7+f8);

	return ff;
}

int QSegdRead::NextFile()
{
        long fsize;
	int shots;

    //qDebug("NextFile ==== %d,%d\n", m_icFile,m_tp->m_iDiskFiles);
// not the first time:
	if(m_icFile != 0 )
	{
                m_tp->close();
                if(m_icFile >= m_iAllFiles)
                return 0;// complete
	}		

// next file :first file is 0;

        m_file =  m_listFiles[m_icFile];
	m_icFile ++;
       // qDebug("Filename ==== %s\n", m_file.ascii());
// open

        if(!m_tp->open(m_file))
		return -1;//cannot open
// offset
        m_tp->seek(m_iOffsetBytes);

// read the file header;
	int iret;
	iret = GetFileInfo();
	if(iret == -1) return -2;//read err
	else if(iret == 0) return -2;//read err
	else if(iret == -2) return -3;// not segd
	else if(iret >0) 
	{
                // first time read ,we donot know the real data header bytes;
                //the data bytes;
		if(m_icFile <= 1)// first file:??
		{// for all file ,not just first segd file,avoid the data header is not the same
                        m_tp->seek(m_iOffsetBytes + m_iHeadBytes);//
			ReadRecord();//read
			SetMainHeader();//find the real data_header,next will read the whole trace;
                        m_tp->seek(m_iOffsetBytes + m_iHeadBytes);
		}
		m_ic = 0;
		m_ica = 0;
// shots in the file
                fsize = m_tp->size();
		shots = (fsize - m_iOffsetBytes)/
			    (m_iHeadBytes + m_iTrailBytes + 
					(m_iDataHDBytes + m_iDataBytes)*m_iTrsAll);
		m_iShotsInFile = shots +1;	 		
		return iret;
	}
	else
	      return -4;

}
