#ifndef SEGD2GRI_H
#define SEGD2GRI_H
#include "QSegbRead.h"
#include "../gridata/gridataadi.h"
class segb2gri
{
public:
    segb2gri();
    QSegbRead *m_segb;
    griData *m_gri;
    int openFile(char * fin,char * fout);

    int run();

    int head[128];
    float *buf,*pbuf;
    int m_icShot,m_icTr;
    QString infile,outfile;


};

#endif // SEGD2GRI_H
