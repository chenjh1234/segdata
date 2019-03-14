#ifndef SEGD2Y_H
#define SEGD2Y_H
//#include <../segdata/segd2y.h>
#include "QSegyADI.h"
#include "QSegdRead.h"
class segd2y
{

public:
    segd2y();
    QSegdRead *m_segd;
    QSegyADI *m_segy;
    int openFile(char * fin,char * fout);

    int run();
    //set 3200bye to segy internal buffer
    int set3200(char *c);
    int set400();
    int setSegyHeader();


    int head[128];
    float buf[10000];
    int m_icShot,m_icTr;
    int m_iSamples;
    int m_iSI;
    int m_iFormat;
    int m_iLtr ;


};

#endif // SEGD2Y_H
