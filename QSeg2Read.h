// QSeg2Read.h: interface for the QSeg2Read class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QSEG2READ_H__BD114D5F_96CA_46F4_A7D3_30FEC038127A__INCLUDED_)
#define AFX_QSEG2READ_H__BD114D5F_96CA_46F4_A7D3_30FEC038127A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "QSeg2.h"
#include "qfile.h"
#include "qstring.h"
#include "QList"
/**
  examples:
  int i,j,sam;
  QSeg2Read *g;
  QStringList list;// append files to list;
  g = new QSeg2Read();

    i = g.setFiles(list);
    g.startRead();
    while(1)
    {
  // start: a file
    i = g.openFile();
    i = g.readFileHeader();
    // read all traces in the shot(file);
    for(i = 0;i <g.m_iNumberOfTraces;i++)
    {
         g.readATrace(i,trace+i*sam);
         sam = g.m_iSamples;
    }
    g.close();

    j = g.nextfile();
    if(j <0) break;

    }

  */

class QSeg2Read : public QSeg2  
{
public:

	QSeg2Read();
	virtual ~QSeg2Read();
        int setCurrentFileNumber(int idx);
        int setCurrentFileName(QString file);
        int startRead();
        int addAFile(QString File);
        void init();
        QStringList m_listFiles;
        int setFiles(QStringList list);
        int nextFile();
        int closeFile();
        int readTraceData(int idx,float *buf);
        int readTraceHeader(int idx);
        int readATrace(int idx,float *buf);
        int readFileHeader();
        int openFile();
	
	QFile m_file;
	int m_iFileCounter;
        typedef QList<int> TRACE_POINTER_LIST;
	TRACE_POINTER_LIST m_listPointer;
	char *m_buf;



};

#endif // !defined(AFX_QSEG2READ_H__BD114D5F_96CA_46F4_A7D3_30FEC038127A__INCLUDED_)
