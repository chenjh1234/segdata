#ifndef SEGB2Y_H
#define SEGB2Y_H
#include "QSegyADI.h"
#include "QSegbRead.h"
class segb2y
{

public:
    segb2y();
    QSegbRead *m_segb;
    QSegyADI *m_segy;
    int openFile(char * fin,char * fout);

    int run();
    //set 3200bye to segy internal buffer
    int set3200(char *c);
    int set400();
    int setSegyHeader();


    int head[128];
    float *buf,*pbuf;
    int m_icShot,m_icTr;
    int m_iSamples;
    int m_iSI;
    int m_iFormat;
    int m_iLtr ;


};

#endif // SEGB2Y_H
