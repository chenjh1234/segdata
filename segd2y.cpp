#include "segd2y.h"

segd2y::segd2y()
{

    m_segy = new QSegyADI();
    m_segd = new QSegdRead();
}
int segd2y::openFile(char * filen,char *ofile)
{
    int i;
    QString segd;
    segd = filen;

    i = m_segd->InitTape(segd);
    if(i <=0)
    {
        qDebug() << "init segd err";
        return -1;
    }

    i = m_segy->openWrite(QString(ofile));
    if(i!=0)
    {
        qDebug("open segy data error");
        return i;
    }
    //prameters:
    return 0;
}

int segd2y::set3200(char *buf)
{
    memcpy(m_segy->m_pBuf3200,buf,3200);
    return 0;
}
int segd2y::set400()
{
            m_segy->m_infoGHD.iAmpType = 1;//none
            m_segy->m_infoGHD.iAuxs = m_segd->GetAuxes();
            m_segy->m_infoGHD.iDataType = 1;//shot
            m_segy->m_infoGHD.iFormat = 2;//long
            m_segy->m_infoGHD.iJobNo = 1;
            m_segy->m_infoGHD.iLineNo =1;
            m_segy->m_infoGHD.iOSams = m_segd->m_iSamples;
            m_segy->m_infoGHD.iOSi = m_segd->m_iSI;
            m_segy->m_infoGHD.iReelNo = 1;
            m_segy->m_infoGHD.iSams = m_segd->m_iSamples;
            m_segy->m_infoGHD.iSi = m_segd->m_iSI;
            m_segy->m_infoGHD.iTrs = m_segd->GetTrs();

            m_iSamples = m_segy->m_infoGHD.iSams;
            m_iSI = m_segy->m_infoGHD.iSi ;
            m_iFormat = 2;
            m_iLtr = m_iSamples *m_iSI/1000;

            if(m_segy->m_infoGHD.iTrs <=0 ||
               m_segy->m_infoGHD.iSi <=0 ||
               m_segy->m_infoGHD.iSams <=0 ||m_segy->m_infoGHD.iSams >MAX_SAMPLES)
               return -1;
            return m_segy->setFileInfo();

            //return 0;
}
int segd2y::setSegyHeader()
{
            int i;
            i = sizeof(QSegyADI::MAINHD_INFO);
            memcpy((void*)&m_segy->m_infoMHD,(void*)&m_segd->m_infoMHD,sizeof(QSegyADI::MAINHD_INFO));
            return 0;
}
int segd2y::run()
{
    int i;

    //prameters:
    int si,ltr,samples,format,ch,shot;

    ltr = m_segd->m_iLTR;
    si = m_segd->m_iSI /1000;
    samples = m_segd->m_iSamples;
    ch = m_segd->m_infoMHD.iChannel;
    shot = m_segd->m_infoMHD.iShot;
    qDebug() <<"ltr,si,samples="<< ltr << si <<samples;

    //init:
    //write 3200
    i = m_segy->set3200();// set 3200buffer
    i = m_segy->write3200();
    if(i !=0)
    {
            qDebug() <<"write 3200 bytes error";
            return -1;
    }
    qDebug() <<"write 3200 ok";
    //set 400
    i = set400(); //set buffer400
    if(i !=0)
    {
         qDebug() <<"set 400 bytes error";
         return -1;
    }
      qDebug() <<"set 400 ok";
    // write400:
    i =m_segy->write400();
    if(i !=0)
    {
         qDebug() <<"Write400 bytes error";
         return -1;
    }
    qDebug() <<"write 400 ok";
    int iret,id,ishot,icc;
    m_icShot = 0;
    m_icTr = 0;
    ishot = -1;
    icc =0; //trace in this shot
    while(1)
    {
            iret = m_segd->ReadTrace(buf,samples);
            if(iret <=0 )
            {
                qDebug()<<"Read segd out iret = "<< iret;
                break;
            }
            m_segy->m_iReelCounter = m_icTr +1;
            m_segy->m_iLineCounter = m_icTr +1;
            //qDebug() << " reel = " <<  m_icTr +1;


            setSegyHeader();
            id = m_segy->writeTrace(buf);
            if(id =0 )
            {
                qDebug()<<"write griData err = "<< id;
                iret = id;
                break;
            }
           // qDebug() << " writr reel = " <<  m_icTr +1;

            m_icTr ++;
            icc++;
            ch = m_segd->m_infoMHD.iChannel;
            shot = m_segd->m_infoMHD.iShot;
            if(ishot != shot)
            {
                if(m_icShot !=0)
                    qDebug() << "shot=" << ishot << m_icTr << icc;
                icc = 0;
                ishot =shot;
                m_icShot ++;

            }
            //one shot only:
            if(m_icShot >=5) break;
    }
    m_segd->CloseTape();
    m_segy->closeWrite();
    qDebug() <<"All shot = " << m_icShot << "All traces = " <<m_icTr;
    return iret;

}


