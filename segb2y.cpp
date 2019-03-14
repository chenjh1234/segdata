#include "segb2y.h"

segb2y::segb2y()
{


    m_segb = new QSegbRead();
    m_segy = new QSegyADI();
}
int segb2y::openFile(char * filen,char *ofile)
{
    int i;
    QString segb;
    segb = filen;

    i = m_segb->InitTape(segb);
    qDebug() << "aaa="<<m_segb->m_iShot<<m_segb->m_iTrs<<i<<m_segb;
    if(i <=0)
    {
        qDebug() << "init segb err";
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

int segb2y::set3200(char *buf)
{
    memcpy(m_segy->m_pBuf3200,buf,3200);
    return 0;
}
int segb2y::set400()
{
            m_segy->m_infoGHD.iAmpType = 1;//none
            m_segy->m_infoGHD.iAuxs = m_segb->GetAuxes();
            m_segy->m_infoGHD.iDataType = 1;//shot
            m_segy->m_infoGHD.iFormat = 2;//long
            m_segy->m_infoGHD.iJobNo = 1;
            m_segy->m_infoGHD.iLineNo =1;

            m_segy->m_infoGHD.iOSi = m_segb->m_iSI;
            m_segy->m_infoGHD.iReelNo = 1;

            m_segy->m_infoGHD.iSi = m_segb->m_iSI;
            m_segy->m_infoGHD.iTrs = m_segb->m_iTrs;




            m_iSI = m_segy->m_infoGHD.iSi ;
            m_iFormat = 2;
            m_iLtr = m_segb->m_iLTR;

            m_iSamples = m_iLtr*1000/m_iSI;
            m_segy->m_infoGHD.iOSams = m_iSamples;
            m_segy->m_infoGHD.iSams = m_iSamples;
            //qDebug()<< m_iSamples<<m_iSI <<m_iLtr;
            //exit(1);

            if(m_segy->m_infoGHD.iTrs <=0 ||
               m_segy->m_infoGHD.iSi <=0 ||
               m_segy->m_infoGHD.iSams <=0 ||m_segy->m_infoGHD.iSams >MAX_SAMPLES)
            {
               qDebug() << "set400 error!!!" << m_segb->m_iSamples<<m_segb->m_iTrs<<m_iSI;
               return -1;
            }
            return m_segy->setFileInfo();

            //return 0;
}
int segb2y::setSegyHeader()
{
            int i;
            i = sizeof(QSegyADI::MAINHD_INFO);
            memcpy((void*)&m_segy->m_infoMHD,(void*)&m_segb->m_infoMHD,sizeof(QSegyADI::MAINHD_INFO));
            return 0;
}
int segb2y::run()
{
    int i;

    //prameters:
    int si,ltr,samples,ch,shot;

    ltr = m_segb->m_iLTR;
    si = m_segb->m_iSI /1000;
    samples = m_segb->m_iSamples;
    ch = m_segb->m_infoMHD.iChannel;
    //shot = m_segb->m_iShot;
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
    int iret,id,ishot,trs;
    m_icShot = 0;
    m_icTr = 0;
    ishot = -1;
    trs = m_segb->m_iTrs;
    pbuf = new float[samples*trs];
    //qDebug()<< "pbuf=" <<samples*trs;

    while(1)
    {
        iret = m_segb->ReadShot(pbuf,samples);
 //qDebug() << "after read trs=" << m_segb->m_iShot<< m_segb->m_infoGHD.iFileNo;
        if(iret <=0 )
        {
            qDebug()<<"Read segb out iret = "<< iret;
            break;
        }
        //ishot = m_segb->m_iShot;
        ishot = m_segb->m_iFFID;
        trs = m_segb->m_iTrs;
        m_icShot ++;
        //qDebug() << "after read trs=" <<trs << ishot<<m_segb->m_iShot;

        for(i = 0; i< trs; i++)
        {
            ch = i+1;
            m_segy->m_iReelCounter = m_icTr +1;
            m_segy->m_iLineCounter = m_icTr +1;
            //fill main header;
            m_segb->m_infoMHD.iShot = ishot;
            m_segb->m_infoMHD.iTr = ch ;
            m_segb->m_infoMHD.iChannel = ch;
            m_segb->m_infoMHD.iType = 1;//1:seis 0:aux
            m_segb->m_infoMHD.iCMP = 0;
            m_segb->m_infoMHD.iLtr = ltr;
            m_segb->m_infoMHD.iOffset = 0;
            m_segb->m_infoMHD.iSi = si*1000;
            m_segb->m_infoMHD.iValid =1;//ok;
            m_segb->m_infoMHD.iTrsAll = m_segb->m_iTrsAll;
            m_segb->m_infoMHD.iTrs = m_segb->m_iTrs;
            m_segb->m_infoMHD.iAux = m_segb->m_iAux;

            buf = pbuf + i*samples;
            setSegyHeader();
            //qDebug() << "after set header" << i<< samples;
            id = m_segy->writeTrace(buf);
            if(id =0 )
            {
                qDebug()<<"write griData err = "<< id;
                iret = id;
                break;
            }
            //qDebug() << " writr    = " <<  m_icShot  << m_icTr<<ch;

            m_icTr ++;
        }//end of one shot;
        qDebug() <<"Shot = " << m_icShot << "FFID=" <<ishot<<"traces = " <<trs;
        if(m_icShot == 3) break;
    }
    m_segb->CloseTape();
    m_segy->closeWrite();
    delete []pbuf;
    qDebug() <<"All shot = " << m_icShot << "All traces = " <<m_icTr;
    return iret;

}

