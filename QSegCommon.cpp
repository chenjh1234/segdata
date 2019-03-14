// QSegCommon.cpp: implementation of the QSegCommon class.
//
//////////////////////////////////////////////////////////////////////

#include "QSegCommon.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QSegCommon::QSegCommon()
{
	Init0();
}

QSegCommon::~QSegCommon()
{

}
void QSegCommon::Init0()
{
        m_iEndian = getEndian();
        //SetTape(&m_tape);
        m_debug = 0;
        m_iSI = -1;
        m_iLTR = -1;
        m_iIUnit = -1;
        m_iOUnit = -1;
        m_tp = new dataIO();
        m_tpo = new dataIO();

}

void QSegCommon::swap2(unsigned char *buf,  int len)
{
	int i;
	unsigned char ch1;
	if(m_iEndian == 1) return;
	for(i = 0;i <len ;i=i+2)
	{
		ch1 = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = ch1;
	}
}
void QSegCommon::swap4(unsigned char *buf,  int len)
{
	int i;
	unsigned char ch1,ch2;
	if(m_iEndian == 1) return;
	for(i = 0;i <len ;i=i+4)
	{
		ch1 = buf[i];
		ch2 = buf[i+1];
		buf[i] = buf[i+3];
		buf[i+1] = buf[i+2];
		buf[i+2] = ch2;
		buf[i+3] = ch1;
	}
}
void QSegCommon::swap8(unsigned char *buf,  int len)
{
        int i;
        unsigned char ch1,ch2,ch3,ch4;
        if(m_iEndian == 1) return;
        for(i = 0;i <len ;i=i+8)
        {
                ch1 = buf[i];
                ch2 = buf[i+1];
                ch3 = buf[i+2];
                ch4 = buf[i+3];
                buf[i] = buf[i+7];
                buf[i+1] = buf[i+6];
                buf[i+2] = buf[i+5];
                buf[i+3] = buf[i+4];
                buf[i+4] = ch4;
                buf[i+5] = ch3;
                buf[i+6] = ch2;
                buf[i+7] = ch1;
               // qDebug() << "this is swap8!!";
        }
}
/*
int QSegCommon::GetInt(unsigned char *buf)
{
	int i;
	Swap4(buf,4);
	i = *((int*)buf);
	return i;
}
int QSegCommon::GetShort(unsigned char *buf)
{
	int i;
	Swap2(buf,2);
	i =*((short *)buf);
	return i;
}
 */
QString QSegCommon::getStatus()
{
	return m_strStatus;
}
void QSegCommon::setStatus(QString s)
{
	 m_strStatus = s;
}
int QSegCommon::getEndian()
{
 
	char ch[4];
	int i,*ip;
	ch[0] = 0;
	ch[1] = 0;
	ch[2] = 0;
	ch[3] = 1;
	ip = (int *) ch;
	i = *ip;
 
	if(i == 1) return 1;
	else return 0;
}//sun:1,pc = 0;


int QSegCommon::getLtr()
{
        return m_iLTR;
}
int QSegCommon::setLtr(int ltr)
{
    m_iLTR = ltr;
    return  ltr;
}
int QSegCommon::getFormat()
{
	return m_iFormat;
}
int QSegCommon::setFormat(int fmt)
{
    m_iFormat = fmt;
    return  fmt;
}
int QSegCommon::getSi()
{
        return m_iSI;
}
int QSegCommon::setSi(int si)
{
    m_iSI = si;
    return  si;
}
int QSegCommon::getSamples()// if set return ,otherwis calculate;
{
        if(m_iSamples !=0)
            return m_iSamples;
        else
        {
            if (m_iSI !=0)  
                return m_iLTR * 1000 / m_iSI;
        }
        return 0;
}
int QSegCommon::setSamples(int sam)
{
    m_iSamples = sam;
    return  sam;
}
int QSegCommon::getInt(unsigned char *buf1)
{
	int i;
	unsigned char buf[4];
	memcpy(buf,buf1,4);
        swap4(buf,4);
	i = *((int*)buf);
	return i;

}
int QSegCommon::getInt1H(unsigned char *buf1)
{
    int i;
	i = (buf1[0]&0xF0)>>4;
	return i;
}
int QSegCommon::getInt1L(unsigned char *buf1)
{
    int i;
	i = buf1[0]&0x0F ;
	return i;
}
qint64 QSegCommon::getInt8(unsigned char *buf1)
{
    return getInt64(buf1);
}
qint64 QSegCommon::getInt64(unsigned char *buf1)
{
        qint64 i;
        unsigned char buf[8];
        memcpy(buf,buf1,8);
        swap8(buf,8);
        i = *((qint64 *)buf);
        return i;
}
int QSegCommon::getShort(unsigned char *buf1)
{
	int i;
	unsigned char buf[2];
	memcpy(buf,buf1,2);
        swap2(buf,2);
	i = *((short *)buf);
	return i;
}
float QSegCommon::getFloat(void *buf1)
{
//	int i;
	float f;
	char buf[4];
	memcpy(buf,buf1,4);
        swap4((unsigned char *)buf,4);
	f =*((float *)buf);
	return f;
}
int QSegCommon::setInt(int *buf,int id)
{
//	int i;
 
	*buf = id;
        swap4((unsigned char *)buf,4);
	return 0;
}
int QSegCommon::setShort(short *buf,short id)
{
//	int i;
	*buf = id;
        swap2((unsigned char *)buf,2);
	return 0;
}
int QSegCommon::BCD2Int(unsigned char *buf,int len,bool flag)
{

	int i1,i;
	int iret,j;
	iret = 0;
	if(flag) j = 0;
	else j = 1;

	for(i = j; i< len +j ;i++)
	{
		if(i/2*2 == i)
                        i1 = ((buf[i/2] & 0xf0) >> 4)* pow((float)10,(float)(len+j -i-1));
		else
                        i1 = ((buf[i/2] & 0x0f)) * pow((float)10,(float)(len+j -i-1));

		iret = iret +i1;
	}

	return iret;
}
 
/*
int QSegCommon::setTape(QDiskIO *tp)
{
	m_tp = tp;
	if(m_tp != NULL)
		return 0;
	else
		return -1;
}

QDiskIO * QSegCommon::getTape()
{
	return m_tp;
}
*/
float QSegCommon::getSignedFloat3_2(unsigned char *pp)
{
	unsigned char *p,ch[5];
	int s,is,i,i1,j;
	float f,ret;
	unsigned short sx,sy;
	memcpy(ch,pp,5);

			p = ch;//
			//i:sign
			s = ((*p) & 0x80 ) >> 7;
			if(s == 0) //sign:
				s = 1;
			else 
				s = -1;
			ch[0] = ch[0] & 0x7f;
			// interger:
                        is = getInt3(p);
			// fraction: pp +2;pp+1;pp+4;pp+5

			p = ch+3 ;//fraction :7-8:
			f = 0;
                        sy = getShort(p);

			for(i = 0; i < 16 ;i++)
			{
				i1 = -1*(i+1);//bit :15,14,13.....0
                                sx = pow((float)2 , (float)(15 - i));
				j = (sy & sx) >>(15 -i);

                                f = f +j *  pow((float)2 , (float)i1);//
			}
			ret = s * (is + f );

		 return ret;
}

int QSegCommon::getInt3(unsigned char *pp)
{
//	char *p;
	int is,*ip;
	char ch[4];
	memcpy(ch,pp,4);

	ip = (int *)ch;
        swap4((unsigned char *)ip,4);
	is = ((*ip) & 0xffffff00)>> 8;
 
	return is;
}
int QSegCommon::getSignedShort(unsigned char *pp)
{
	int s,is ;
	unsigned char ch[2];
 
	memcpy(ch,pp,2);
	s = (ch[0] &0x80) >>7;
	//sign
	if(s == 1)
		s = -1;
	else
		s = 1;
	ch[0] = ch[0] & 0x7f;
        is = s * getShort(ch);
	return is;


}

int QSegCommon::getSignedInt3(unsigned char *pp)
{
	int s,is ;
	unsigned char ch[4];
 
	memcpy(ch,pp,4);
	s = (ch[0] &0x80) >>7;
	//sign
	if(s == 1)
		s = -1;
	else
		s = 1;

	ch[0] = ch[0] & 0x7f;

        is = getInt3(ch);
	is = is *s;
	return is;

}
void QSegCommon::float_to_ibm(int from[], int to[], int n)
{
	register int fconv, fmant, ii, t;

	for (ii=0;ii<n;++ii) {
	fconv = from[ii];
		if (fconv) {
			fmant = (0x007fffff & fconv) | 0x00800000;
			t = (int) ((0x7f800000 & fconv) >> 23) - 126;
			while (t & 0x3) { ++t; fmant >>= 1; }
			fconv = (0x80000000 & fconv) | (((t>>2) + 64) << 24) | fmant; 
		} 
		to[ii] = fconv; 
	} 

        swap4((unsigned char *)to,n*4);
	return; 
}

float QSegCommon::getFloat1_1(unsigned char *pp)
{
	unsigned char *p;
	int id,i,i1,j;
	float f;
	unsigned char sx,sy;

	p = pp;
	id = (*p);

	p++;
	f = 0;

	sy = (*p);
	for(i = 0; i < 8 ; i++)
	{
		 
		i1 = -1*(i+1);//bit :15,14,13.....0
                sx = pow((float)2 , (float)(8 - i));
		j = (sy & sx) >>(8 -i);
                f = f +j *  pow((float)2 ,(float) i1);//
	}
	return id + f;
}
float QSegCommon::getFloat2_1(unsigned char *pp)
{
	unsigned char p[3];
	int id,i,i1,j;
	float f;
	unsigned char sx,sy;
    memcpy(p,pp,3);

 
	//id = (*p);
    id = getShort(p);

	 
	f = 0;

	sy = p[2];
	for(i = 0; i < 8 ; i++)
	{
		 
		i1 = -1*(i+1);//bit :15,14,13.....0
                sx = pow((float)2 , (float)(8 - i));
		j = (sy & sx) >>(8 -i);
                f = f +j *  pow((float)2 ,(float) i1);//
	}
	return id + f;
}
float QSegCommon::getFloat1(unsigned char *pp)
{
 
	int id,i,i1,j;
	float f;
	unsigned char sx,sy;
    
	sy = *(pp);
	for(i = 0; i < 8 ; i++)
	{
		 
		i1 = -1*(i+1);//bit :15,14,13.....0
                sx = pow((float)2 , (float)(8 - i));
		j = (sy & sx) >>(8 -i);
                f = f +j *  pow((float)2 ,(float) i1);//
	}
	return   f;
}

