#include "segd2gri.h"

segd2gri::segd2gri()
{
    m_gri = new griData();
    m_segd = new QSegdRead();
}
int segd2gri::openFile(char * filen,char *ofile)
{
    int i;
    QString segd;
    segd = filen;
    infile = segd;
    outfile = ofile;

    i = m_segd->InitTape(segd);
    if(i <=0)
    {
        qDebug() << "init segd err";
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


int segd2gri::run()
{
    int i;

    //prameters:
    int si,ltr,samples,format,ch,shot;
    ltr = m_segd->m_iLTR;
    si = m_segd->m_iSI /1000;
    samples = m_segd->m_iSamples;
    ch = m_segd->m_infoMHD.iChannel;
    shot = m_segd->m_infoMHD.iShot;
    qDebug() <<"Input file ="<< infile ;
    qDebug() <<"Outpou file ="<< outfile ;
    qDebug() <<"ltr,si,samples="<< ltr << si <<samples;
    int iret,id,ishot;
     m_icShot = 0;
     m_icTr = 0;
    ishot = -1;
    while(1)
    {
            iret = m_segd->ReadTrace(buf,samples);
            if(iret <=0 )
            {
                qDebug()<<"Read segd out iret = "<< iret;
                break;
            }
            ch = m_segd->m_infoMHD.iChannel;
            shot = m_segd->m_infoMHD.iShot;

            head[1-1] = ltr;
            head[2-1] = m_segd->m_infoMHD.iShot;
            head[9-1] = si;
            head[10-1] = ltr;
            head[11-1] = 7;
            head[17-1] = ch;
            head[55-1] = 4;
            head[56-1] = shot;
            head[124-1] = m_segd->m_iTrsAll ;
            if(m_segd->m_infoMHD.iType == 1)
                head[128-1] = 0;
            else
                head[128-1] = 2;


            //m_segd->m_infoMHD.fReceiverLine;
            if(m_segd->m_infoMHD.iType !=1)
            {
                //omit the aux trace;
                //qDebug() << "aux trace" ;
                continue;// skip output aux trace.
            }
            //qDebug() << "shot,trace="<< head[2-1] << head[17-1] ;

            id = m_gri->write_a_trace(head,buf);
            if(id !=0 )
            {
                qDebug()<<"write griData err = "<< id;
                iret = id;
                break;
            }
            m_icTr ++;
            if(ishot != head[2-1])
            {
                ishot =head[2-1];
                m_icShot ++;
            }
            //one dhot only:
            //if(m_icShot >=2) break;
    }
    m_segd->CloseTape();
    m_gri->close_write();
    qDebug() <<"All shot = " << m_icShot << "All traces = " <<m_icTr;
    return iret;

}



