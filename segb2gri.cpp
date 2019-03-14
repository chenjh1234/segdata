#include "segb2gri.h"

segb2gri::segb2gri()
{
    m_gri = new griData();
    m_segb = new QSegbRead();
    memset(head,0,128*4);
}
int segb2gri::openFile(char * filen,char *ofile)
{
    int i;
    QString segb;
    segb = filen;
    infile = segb;
    outfile = ofile;
    i = m_segb->InitTape(segb);

    if(i <=0)
    {
        qDebug() << "init segb err" << i;
        return -1;
    }
    i = m_gri->open_write(ofile);
    if(m_gri->flag !=0)
    {
        qDebug("open gri data error");
        return -2;
    }
    //prameters:
    return 0;
}


int segb2gri::run()
{
    int i;

    //prameters:
    int si,ltr,samples,format,trs,shot,ch;
    ltr = m_segb->m_iLTR;
    si = m_segb->m_iSI /1000;
    samples = m_segb->m_iSamples;
    trs = m_segb->m_iTrs;
    shot = m_segb->m_infoGHD.iFileNo ;
    qDebug() <<"Input file ="<< infile ;
    qDebug() <<"Outpout file ="<< outfile ;
    //qDebug() <<"ltr,si,samples="<< ltr << si <<samples;
    int iret,id,ishot;
     m_icShot = 0;
     m_icTr = 0;
    ishot = -1;
    pbuf = new float[samples*trs];
    //qDebug()<< "pbuf=" <<samples*trs;

    while(1)
    {
        iret = m_segb->ReadShot(pbuf,samples);
        if(iret <=0 )
        {
            qDebug()<<"Read segb out iret = "<< iret;
            break;
        }
        ishot = m_segb->m_iShot;
        trs = m_segb->m_iTrs;
        m_icShot ++;
       // qDebug() << "trs=" <<trs;

        for(i = 0; i< trs; i++)
        {
            ch = i+1;
            head[1-1] = ltr;
            head[2-1] = m_icShot;
            head[9-1] = si;
            head[10-1] = ltr;
            head[11-1] = 7;
            head[17-1] = ch;
            head[55-1] = 4;
            head[56-1] = ishot;
            head[124-1] = m_segb->m_iTrsAll ;
            if(m_segb->m_infoMHD.iType == 1)
                head[128-1] = 0;
            else
                head[128-1] = 2;

            buf = pbuf + i*samples;
            id = m_gri->write_a_trace(head,buf);
            if(id !=0 )
            {
                qDebug()<<"write griData err = "<< id;
                iret = id;
                break;
            }
            m_icTr ++;
        }//end of one shot;
        qDebug() <<"Shot = " << m_icShot << "FFID=" <<ishot<<"traces = " <<trs;
        //if(m_icShot == 3) break;
    }
    m_segb->CloseTape();
    m_gri->close_write();
    delete []pbuf;
    qDebug() <<"All shot = " << m_icShot << "All traces = " <<m_icTr;
    return iret;

}



