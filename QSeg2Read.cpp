// QSeg2Read.cpp: implementation of the QSeg2Read class.
//
//////////////////////////////////////////////////////////////////////

#include "QSeg2Read.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QSeg2Read::QSeg2Read()
{
	init();
}

QSeg2Read::~QSeg2Read()
{
	
}
void QSeg2Read::init()
{
	m_iFileCounter = 0;
}
int QSeg2Read::openFile()
{
        m_file.setFileName(m_strCurrentFileName);
        if(m_file.open(QIODevice::ReadOnly))
	{
		return 0;
	}
	else
		return -1;
}
/*

-1: read header err;
-2: not a seg2 format file
*/
int QSeg2Read::readFileHeader()
{
	int i,size;
	int flag;
	char *pbuf;
// read 32 header

	pbuf = new char[32];
        i = m_file.read(pbuf,32);
	if(i != 32)
		return -1;//read header err;

	flag = getShort(pbuf);
	if(flag != 0x3a55)
		return -2; // not a seg2 format file
	m_iReVision = getShort(pbuf + 2);
	m_iSizeOfTrPointerBlock = getShort(pbuf + 4);
	m_iNumberOfTraces = getShort(pbuf +6);
	delete []pbuf;

	qDebug("revision,size of p block,traces = %d,%d,%d\n",
			m_iReVision,
			m_iSizeOfTrPointerBlock,
			m_iNumberOfTraces);
	
// read trace pointer:

	size = m_iSizeOfTrPointerBlock;
	pbuf = new char[size +10];

        i = m_file.read(pbuf,size);
	if(i != size )
	{
		delete []pbuf;
		return -1;
	}
	// save to list:

	m_listPointer.clear();

	for(i = 0; i < m_iNumberOfTraces ; i++)
	{
		flag = getInt(pbuf + i* sizeof(int));
		m_listPointer.append(flag);
		qDebug(" idx=%d,pointer = %d,%x\n",i,flag,flag);
	}
	delete []pbuf;
// read the file strings:
	size = m_listPointer[0] - (32 + m_iSizeOfTrPointerBlock);

	pbuf = new char[size];

        i = m_file.read(pbuf,size);
	if(i != size )
	{
		delete []pbuf;
		return -1;
	}
	qDebug("size of file string = %d\n",size);

	m_strFileString = pbuf;

	delete []pbuf;
 //get m_iSamples;
        readTraceHeader(0);

	return 0;
}

int QSeg2Read::readATrace(int idx,float *buf)
{
	int id;
	id = readTraceHeader(idx);
	if(id !=0 )return id;
	id = readTraceData(idx,buf);
	if(id !=0 )return id;
	return 0;
}
/*
-1:read error
-2:not a seg2 trace flag
*/
int QSeg2Read::readTraceHeader(int idx)
{
	int i,size,ptr;
	int flag;
	char *pbuf;
	
	ptr = m_listPointer[idx];
// seek the trace:
        m_file.seek(ptr);

	pbuf = new char[32];
// read 32 header:

        i = m_file.read(pbuf,32);
	if(i != 32)
		return -1;//read header err;
	// flag;
	flag = getShort(pbuf);
	if(flag != 0x4422)
		return -2; // not a seg2 flag  error

	m_iSizeOfTrHeaderBlock = getShort(pbuf + 2);
	m_iSizeOfTrDataBlock = getInt(pbuf + 4);
	m_iSamples = getInt(pbuf + 8);
	m_iDataFormat = pbuf[12];

	delete []pbuf;

	qDebug("HDsize,DATASize,samples,format = %d,%d,%d,%d\n",
		m_iSizeOfTrHeaderBlock,
		m_iSizeOfTrDataBlock,
		m_iSamples,
		m_iDataFormat);


// read the trace string:
	size = m_iSizeOfTrHeaderBlock - 32;
	pbuf = new char[size];

        i = m_file.read(pbuf,size);
	if(i != size)
	{
		delete []pbuf;
		return -1;//read header err;
	}
	// save to trace string:
	m_strTraceString = pbuf;

	delete []pbuf;


	return 0;
}
/*
-1:index out of range;
-2:read error;
-3:format error
*/
int QSeg2Read::readTraceData(int idx,float *buf)
{
	int ptr,size,i,len;
	char *pbuf;
	if(idx < 0 || idx >= m_iNumberOfTraces)
		return -1;// index out of range;

	ptr = m_listPointer[idx] + m_iSizeOfTrHeaderBlock;
	size = m_iSizeOfTrDataBlock;
	pbuf = new char[size];

        m_file.seek(ptr);
        i = m_file.read(pbuf,size);
	if(i !=size)
	{
		return -2;// read error;
	}
	// read ok
	len = m_iSamples;

	switch(m_iDataFormat)
	{
	case 1:
		f1ToFloat(pbuf,buf,len);
		break;
	case 2:
		f2ToFloat(pbuf,buf,len);
		break;
	case 3:
		f3ToFloat((short *)pbuf,buf,len);
		break;
	case 4:
		f4ToFloat(pbuf,buf,len);
		break;
	case 5:
		f1ToFloat(pbuf,buf,len);
		break;
	default:
		delete []pbuf;
		return -3;//format error

		break;

	}

	delete []pbuf;

	return 0;
}

int QSeg2Read::closeFile()
{
	m_file.close();
	return 0;
}

int QSeg2Read::nextFile()
{
	m_iFileCounter++;
	int size = m_listFiles.count();
	if(m_iFileCounter < size)
		m_strCurrentFileName = m_listFiles[m_iFileCounter];
	else
		return -1; // no file in the  file list

	return m_iFileCounter;
}

int QSeg2Read::setFiles(QStringList list)
{
	m_listFiles = list;
	return m_listFiles.count();
}



int QSeg2Read::addAFile(QString file)
{
	m_listFiles.append(file);
	return m_listFiles.count();
}

int QSeg2Read::startRead()
{
	m_iFileCounter = 0;
	if(m_listFiles.count() != 0)
		m_strCurrentFileName = m_listFiles[m_iFileCounter];
	else
		return -1; // no file in the  file list
//
	return 0;
	
}

int QSeg2Read::setCurrentFileName(QString file)
{
	m_strCurrentFileName = file;
	return 0;
}

int QSeg2Read::setCurrentFileNumber(int idx)
{
	m_iFileCounter = idx;
	int size = m_listFiles.count();
	if(m_iFileCounter < size)
		m_strCurrentFileName = m_listFiles[m_iFileCounter];
	else
		return -1; // no file in the  file list

	return m_iFileCounter;
}
