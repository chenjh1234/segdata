// QSeg2Write.cpp: implementation of the QSeg2Write class.
//
//////////////////////////////////////////////////////////////////////

#include "QSeg2Write.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QSeg2Write::QSeg2Write()
{
	init();
}

QSeg2Write::~QSeg2Write()
{
	if(m_piTracePointer != NULL)
		delete m_piTracePointer;

}

void QSeg2Write::init()
{
	setBaseFileName("dat");
	setStartFileNumber(0);
	setExtFileName("seg2");
	m_iFileCounter = 0;
	m_piTracePointer = NULL;
	m_strPathName = "";
        m_iCurrentFileNumber =0;

}
 
	
int QSeg2Write::writeFileHeader()
{
	int i,iNum;
	short sNum;
//	char bNum;
	char *p;
//
	if(m_iNumberOfTraces <=0)
	{
		m_strErr = "Number of traces <= 0";
		return -1;
	}
	if(m_iSamples <=0)
	{
		m_strErr = "Samples <= 0";
		return -2;
	}
//start
	// file descriptrt Blockk ID
	sNum = 0x3a55;
	p = (char* )&sNum;
	swap2(p);
        m_file.write(p,2);

// reversion;
	sNum = m_iReVision;
	p = (char* )&sNum;
	swap2(p);
        m_file.write(p,2);

// size os trace pointer sub-block:

	int size;
	size = 4 * m_iNumberOfTraces ;
//	size = (size + 32)/32 *32;
	m_iSizeOfTrPointerBlock = size;

	sNum = size;
	p = (char* )&sNum;
	swap2(p);
        m_file.write(p,2);

// number of traces:
	sNum = m_iNumberOfTraces;
	p = (char* )&sNum;
	swap2(p);
        m_file.write(p,2);

// size of string term, first string term:0x0100
	sNum = 0x0001;
	p = (char* )&sNum;
	swap2(p);
        m_file.write(p,2);

// second string term,size of line term 0x0001
	sNum = 0x0100;
	p = (char* )&sNum;
	swap2(p);
        m_file.write(p,2);

// first line term,second line term 0x0a00
	sNum = 0x000a;
	p =(char* )&sNum;
	swap2(p);
        m_file.write(p,2);

// 14-31: reserved,len = 18;
	char *ch;
	int len = 18;
	ch = new char[len];
	memset(ch,0,len);
        m_file.write(ch,len);
	delete []ch;
// trace pointer sub-block:

	if(m_piTracePointer != NULL)
		delete m_piTracePointer;

	m_piTracePointer = new int[m_iNumberOfTraces +10];


	m_iSizeOfTrHeaderBlock = 32 + m_strTraceString.length();
	m_iSizeOfTrDataBlock = m_iSamples * sizeof(float);// only for format 4

	for(i = 0; i < m_iNumberOfTraces ; i++)
	{
		m_piTracePointer[i] = 32 + m_iSizeOfTrPointerBlock + m_strFileString.length() +
			             i * (m_iSizeOfTrHeaderBlock + m_iSizeOfTrDataBlock);
		
		iNum = m_piTracePointer[i];
		p =(char* )&iNum;
		swap4(p);
                m_file.write(p,4);
	}

	//file strings:
	if(m_strFileString.length() !=0 )
	{
                m_file.write(m_strFileString.toUtf8().data(),m_strFileString.length());
	}

	return 0;
}
int QSeg2Write::writeFileHeader(int traces,int samples)
{
	setNumberOfTraces(traces);
	setSamples(samples);
	return writeFileHeader();
}
int QSeg2Write::getTracePointer(int indexOfTrace)
{
	if(m_piTracePointer)
		return m_piTracePointer[indexOfTrace];
	else
		return -1;
}

int QSeg2Write::writeTraceHeader()
{
	int iNum;
	short sNum;
	char bNum;
	char *p;

// traces descriptrt Blockk ID
	sNum = 0x4422;
	p = (char* )&sNum;
	swap2(p);
        m_file.write(p,2);
// size of this block(2-3):
	sNum = m_iSizeOfTrHeaderBlock;
	p = (char* )&sNum;
	swap2(p);
        m_file.write(p,2);
// size of data block(4-7):
	iNum = m_iSizeOfTrDataBlock;
	p = (char* )&iNum;
	swap4(p);
        m_file.write(p,4);
// samples (8-11):
	iNum = m_iSamples;
	p = (char* )&iNum;
	swap4(p);
        m_file.write(p,4);
	// format 12:
	bNum = m_iDataFormat;
	p = (char* )&bNum;
	//swap4(p);
        m_file.write(p,1);
	//reserved:13-32 =19;
	char *ch;
	int len = 19;
	ch = new char[len];
	memset(ch,0,len);
        m_file.write(ch,len);
	delete []ch;


	//trace strings:
	if(m_strTraceString.length() !=0 )
	{
                m_file.write(m_strTraceString.toUtf8().data(),m_strTraceString.length());
	}

	return 0;
}
int QSeg2Write::writeTraceData(float * buffer)
{
	int ib;
	char *p;
	p = (char*) buffer;
	swap4(p,m_iSamples);
        ib = m_file.write(p,sizeof(float) * m_iSamples);
	if(ib == int (sizeof(float) * m_iSamples))
		return 0;
	else 
		return -1;
}
int QSeg2Write::gotoNextFile()
{
	m_iFileCounter ++;
	m_iCurrentFileNumber = m_iStartFileNumber + m_iFileCounter;
	setCurrentFileName();
	return 0;
}
int QSeg2Write::openFile()
{
	setCurrentFileName();
        m_file.setFileName(m_strCurrentFileName);
        if(m_file.open(QIODevice::ReadWrite))
	{
		return 0;
	}
	else
		return -1;
}
int QSeg2Write::openFile(int traces,int samples)
{
	setNumberOfTraces(traces);
	setSamples(samples);
	return openFile();
}
int QSeg2Write::openFile(int traces,int samples,int shotnumber)
{
	setNumberOfTraces(traces);
	setSamples(samples);
	setCurrentFileNumber(shotnumber);
	return openFile();
}
int QSeg2Write::closeFile()
{
	m_file.close();
	return 0;
}
QString QSeg2Write::setCurrentFileName()
{
	QString str,dot,num,dirsign;
	dot = ".";
	dirsign = "/";
	num.sprintf("%04d",m_iCurrentFileNumber);
	if(m_strPathName != "")
		if(m_strExtFileName !="")
			str = m_strPathName + dirsign + m_strBaseFileName + num + dot + m_strExtFileName;
		else
			str = m_strPathName + dirsign + m_strBaseFileName + num;

	else
		if(m_strExtFileName !="")
			str = m_strBaseFileName + num + dot + m_strExtFileName;
		else
			str = m_strBaseFileName + num ;

	m_strCurrentFileName = str;
	return str;
}

int QSeg2Write::setBaseFileName(QString name)
{
	m_strBaseFileName = name;
	return 0;
}

int QSeg2Write::setStartFileNumber(int number)
{
	m_iStartFileNumber = number;
	return number;
}

int QSeg2Write::setExtFileName(QString ext)
{
	m_strExtFileName = ext;
	return 0;
}

void QSeg2Write::setFileString(QString filestring)
{
	m_strFileString = filestring;
}

QString QSeg2Write::getFileString()
{
	return m_strFileString;
}

void QSeg2Write::setTraceString(QString tracestring)
{
	m_strTraceString = tracestring;
}

QString QSeg2Write::getTraceString()
{
	return m_strTraceString;
}

int QSeg2Write::setCurrentFileNumber(int number)
{
	m_iCurrentFileNumber = number;
	setCurrentFileName();
	return number;

}

int QSeg2Write::getCurrentFileNumber()
{
	return m_iCurrentFileNumber;
}

int QSeg2Write::writeTrace(float *buffer)
{
	if(writeTraceHeader() !=0) 
		return -1;
	if(writeTraceData(buffer) !=0) 
		return -2;
	return 0;
}

QString QSeg2Write::getCurrentFileName()
{
	return m_strCurrentFileName;
}
