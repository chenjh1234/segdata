#ifndef SEGD2GRI_H
#define SEGD2GRI_H
#include "QSegdRead.h"
#include "../gridata/gridataadi.h"
class segd2gri
{
public:
    segd2gri();
    QSegdRead *m_segd;
    griData *m_gri;
    int openFile(char * fin,char * fout);

    int run();

    int head[128];
    float buf[10000];
    int m_icShot,m_icTr;
    QString infile,outfile;


};

#endif // SEGD2GRI_H
