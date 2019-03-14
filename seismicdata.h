#ifndef SEISMICDATA_H
#define SEISMICDATA_H
#include<QString>
#include<QStringList>
#include<QMap>
#define GRISYS 0
#define SEGY 1
#define SEGD 2
#define SEG2 3
#define GEOEAST 4
#define NOT_SUPPORT -1

/**
  thsi is a virtual class for seismicData interfase;\n
  we try to access all the seismic data use the same interface;\n


  */
class seismicData
{
public:
    seismicData();
    /**
     \return: 0:ok,-1:not this data;
      */
    virtual int isSeismicData(QString name)=0;
    /**
      open data,and get the file information,\n
        int m_fSI;//in m_iSI == 0, m_fSI works\n
        int m_iSI;// sample rate in ms\n
        int m_iSamples;// samples in a trace\n
        int m_iLTR;// trace length in ms\n
        int m_iHeadBytes;// head bytes\n
        int m_iHeadLen;// head wo\n
      get ready for the readTrace method;

    */
    virtual int openData(QString file)=0;
//    virtual int openData(QStringList filelist)=0;

    /**
        read trace
      */
    virtual int readTrace(int *head ,float *buf)=0;
    virtual int readTraces(int *head ,float *buf,int trs)=0;


    virtual int closeData(QString name)=0;

    int m_fSI;//in m_iSI == 0, m_fSI works
    int m_iSI;// sample rate in ms
    int m_iSamples;// samples in a trace
    int m_iLTR;// trace length in ms
    int m_iHeadBytes;// head bytes
    int m_iHeadLen;// head word,


    QMap<int,QString> m_dataType;
    int m_iDataType;
};

#endif // SEISMICDATA_H
