// QTapeData.cpp: implementation of the QTapeData class.
//
//////////////////////////////////////////////////////////////////////

#include "QTapeData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QTapeData::QTapeData()
{
	Init();
}

QTapeData::~QTapeData()
{
	delete []m_pcBuf;
}

void QTapeData::Init()
{
	m_ch = QTextCodec::codecForName("GB2312");  

	m_strChRead = "读";
	m_strChWrite = "写";
	m_strChPauseCopy = "暂停拷贝";
	m_strChContinueCopy = "继续拷贝";
	m_strChStartCopy = "开始拷贝";
	m_strChAdvanced = "高级操作";
	m_strChNormal = "正常操作";

	m_strChSelectShot = "选择炮号：";
	m_strChSelectFile = "选择文件：";
	m_strChSelectRecord ="选择记录：";

	m_iTapeBlockLen = 64000;
	m_pcBuf = new unsigned char[m_iTapeBlockLen];
	SetCopyParDefault();

	m_iTapeMode = READ_MODE;
	m_iFileMode = WRITE_MODE;
	m_iCopyStatus = COPY_STOPED;

	//test HexOut:
 /*
	int i;
	QString str;
	char buf[500];

	for(i = 0;i <500; i++)
		buf[i] = i;
	str = HexOut((unsigned char*)buf,160,3);
	qDebug("%s\n",str.ascii());
 
*/	


}

void QTapeData::SetCopyParDefault()
{
	int i;
	for(i = 0;i <SELECT_PAIR;i++)
	{
		m_ParSelectRange.from[i] = 0;
		m_ParSelectRange.to[i] = 0;
	}
	IsParSelected();
	m_iParInputSI = 0;
	m_iParInputLTR = 0;
	m_iCopyFormat = COPY_SEGY;
	m_iCopyMode = COPY_NEW;
	m_iParOutputSI = 0;
	m_iParOutputLTR = 0;
}

bool QTapeData::IsParSelected()
{
	int i,ic,sum,id,icc;
	bool b;
	b = false;
	ic = 0;
	sum = 0;
	icc = 0;
	id = 1;

	for(i = 0; i <SELECT_PAIR ; i++)
	{
		if(m_ParSelectRange.from[i] == 0 || m_ParSelectRange.to[i] == 0)
		{
			//continue;
			break;
		}
		else
		{
			ic = m_ParSelectRange.to [i] - m_ParSelectRange.from[i] +1;
			if(ic < 0) break;
			if(i >0)
			{
				id = m_ParSelectRange.from[i] - m_ParSelectRange.to[i-1];
			}
			
				if(id <=0)//the range in overlap;
				{
					id =-1*id;
					ic = ic - id -1;
				}
			
			
			if(ic >0)
			{
				sum = sum +ic;
				icc++;
			}
			b = true;
		}
	}
	m_ParSelectRange.size = icc;
	m_ParSelectRange.files = sum;
	return b;
}

QString QTapeData::GetCH(QString ch)
{
	//return m_ch->toUnicode(ch);
    return ch;
}

QString QTapeData::HexOut( unsigned char *buf,int rby,int line) 
{
	// TODO: Add your command handler code here
	QString out;

	int all_lines,i,j,print_lines;
	char str[80];
	int ichs = 16;
	all_lines = rby/ichs+1;
//	printf("ic = %d\n",ic);
	print_lines = line;
//	if(all_lines < print_lines ) print_lines = all_lines;
// print hex code
	if(print_lines <=0) print_lines = 1;
//	printf("print_lines=%d\n",print_lines);
	for(i = 0;i<print_lines;i++)
	{
		//sprintf(str,"%06d | ",rby+i*ichs);
		sprintf(str,"%06d | ", i*ichs);
		HexOutStrN(str,out);
		for(j = 0;j<ichs;j++)
		{
			if(j != 7 )
			{
				sprintf(str,"%02X ",buf[i*ichs+j]);
				HexOutStrN(str,out);
			}
			else
			{
				sprintf(str,"%02X-",buf[i*ichs+j]);
				HexOutStrN(str,out);
			}

		}
		sprintf(str," | ");
		HexOutStrN(str,out);
// print ascii code
		for(j = 0;j<ichs;j++)
		{
			if(buf[i*ichs+j] >= 0 && buf[i*ichs+j] <= ichs +1)
			{
				sprintf(str,".");
				HexOutStrN(str,out);
			}
			else
			{
				sprintf(str,"%1C",buf[i*ichs+j]);
				HexOutStrN(str,out);
			}
		}
		sprintf(str," ");
		HexOutStr(str,out);
	}	
	
	return out;
}
void QTapeData::HexOutStr(char *str,QString &out)
{
	//printf("%s\n",str);
	out = out + str + "\n";
}
void QTapeData::HexOutStrN(char *str,QString &out)
{
	//printf("%s",str);
	out = out + str ;
}