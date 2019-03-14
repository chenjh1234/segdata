// QSeg2Write.h: interface for the QSeg2Write class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QSEG2WRITE_H__63AAB110_1A04_47DB_86D3_7F4F09ABDFFC__INCLUDED_)
#define AFX_QSEG2WRITE_H__63AAB110_1A04_47DB_86D3_7F4F09ABDFFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "QSeg2.h"
#include <qfile.h>
/**
	function: this is seg2 write API class.
                we can use some of its methed to write seg2 files:
	1:give the base parameters:

		QSeg2Write sg2;
                g.setBaseFileName (s);//defult = "dat"
                g.setExtFileName(QString ext); // seg2
                g.setStartFileNumber(int number);
                // default dat0000.seg2,dat0001.seg2.......
		sg2.setNumberOfTraces(x);
		sg2.setSamples(x);
	2:do write files:
                n  =0;
                sg2.setCurrentFileNumber(n);
		while( end of shot)
		{
			sg2.openFile() ;
			sg2.writeFileHeader();
			
			for(i = 0; i < N ;i ++)  N=number of traces
			{
				sg2.writeTraceHeader();
                                //get traces data from other format
				sg2.writeTraceHData(buf);
			}
			sg2.closeFile();

			sg2.gotoNextFile();
                        n++;
		}

  
*/

class QSeg2Write : public QSeg2  
{
public:
	QString getCurrentFileName();
	QSeg2Write();
	virtual ~QSeg2Write();
/**
	open file for write
	the file name is datxxxx.seg2,xxxx=counter.
	or the file name is datxxxx.seg2,xxxx=shot number.

*/
	int openFile();
	int openFile(int traces,int samples);
	int openFile(int traces,int samples,int shotnumber);
/**
	close file
*/
	int closeFile();
/**
	write file header:
*/
	int writeFileHeader();
	int writeFileHeader(int traces,int samples);
/**
	write the trace header and data, in sequency
*/
	int writeTrace(float *buffer);
/**
	set current file number ,and set the current filename according to it;
	useally this is the shot number
*/
	int setCurrentFileNumber(int number);
/**
	get current file number 
*/

//*=====================
	int getCurrentFileNumber();

	int * m_piTracePointer;

	QFile m_file;
	
	int m_iStartFileNumber;
	int m_iFileCounter;
	int m_iCurrentFileNumber;
	QString m_strBaseFileName; 
	QString m_strExtFileName;


	int setExtFileName(QString ext);
	int setStartFileNumber(int number);
	int setBaseFileName(QString name);
/**
	init sone inial parameters
*/
	void init();
	/**
		get the trace strings:m_strTraceString
	*/
	QString getTraceString();
	/**
		get the file strings:m_strFileString
	*/
	QString getFileString();

	/**
		set the file strings
	*/
	void setFileString(QString filestring);
	/**
		set the trace strings
	*/
	void setTraceString(QString tracestring);
/**
	write file header; before call this methed we have to set:\n
		traces of the shot.\n
		samples of the trace.\n
		data format:\n


*/
	int writeTraceHeader();
/**
	write the trace data, in sequency
*/
	int writeTraceData(float * buffer);
/*
	get trace pointer
*/
	int getTracePointer(int indexOfTrace);
/*
	get next file name in sequence
*/
	int gotoNextFile();
/*
	set the current file name according current parameter:
		m_iCurrentFileNumber
*/
	QString setCurrentFileName();

};

#endif // !defined(AFX_QSEG2WRITE_H__63AAB110_1A04_47DB_86D3_7F4F09ABDFFC__INCLUDED_)
