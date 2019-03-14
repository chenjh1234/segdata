// dataIO.h: interface for the dataIO class.
//
//////////////////////////////////////////////////////////////////////

#ifndef DATA_IO_H
#define DATA_IO_H


#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#if  !defined(WIN32)
#include <sys/mtio.h>
#endif

//#include <stdio.h>
#include<QString>
#include<QStringList>
#include<QFile>
#include<QDebug>
#define TAPE_BLOCK 256000
#define TAPE_BLOCK_MIN 60000
//
#define TP_REWIND "rew"
#define TP_UNLOAD "uld"
#define TP_FWF "fwf"
#define TP_FWR "fwr"
#define TP_BKF "bkf"
#define TP_BKR "bkr"
#define TP_EOF "eof"
#define TP_STA "sta"



// file open id: 
#define FILEOPEN_READ 0
#define FILEOPEN_WRITE 1
#define FILEOPEN_RW 2
//
#define OPENFILE_OK 1
#define OPENFILE_ERR -2
#define OPENFILE_INDEX_ERR -3

//#define READFILE_OK
#define READFILE_ERR -11    // read data !=  bytes;
#define READFILE_INDEX_ERR -12 // read index !=4bytes
#define READFILE_BYTE_ERR -13 // block size read from index file <0
#define READFILE_TYPE_ERR -14 // not a type of:disk,tpimg,tape

 
#define WRITEFILE_ERR -21    // read data !=  bytes;
#define WRITEFILE_INDEX_ERR -22 // read index !=4bytes
#define WRITEFILE_BYTE_ERR -23 // block size read from index file <0
#define WRITEFILE_TYPE_ERR -24 // not a type of:disk,tpimg,tape

#define TP_ERR()\
tpErr[OPENFILE_OK] = "OK";\
tpErr[OPENFILE_ERR] = "COPY_READ_ERR";\
tpErr[OPENFILE_INDEX_ERR] = "OPENFILE_INDEX_ERR";\
\
tpErr[READFILE_ERR] = "READFILE_ERR";\
tpErr[READFILE_INDEX_ERR] = "READFILE_INDEX_ERR";\
tpErr[READFILE_BYTE_ERR] = "READFILE_BYTE_ERR";\
tpErr[READFILE_TYPE_ERR] = "READFILE_TYPE_ERR";\
\
tpErr[WRITEFILE_ERR] = " WRITEFILE_ERR";\
tpErr[WRITEFILE_INDEX_ERR] = "WRITEFILE_INDEX_ERR";\
tpErr[WRITEFILE_BYTE_ERR] = "WRITEFILE_BYTE_ERR";\
tpErr[WRITEFILE_TYPE_ERR] = "WRITEFILE_TYPE_ERR";\


//#define IBLOCK 3 * sizeof(int) // a test
#define IBLOCK 200 * 1000 * sizeof(int)// block  save to disk
#define BYTE unsigned char
// device type:

#define DEV_DISK 0
#define DEV_TAPE 1
#define DEV_TPIMG 2

#define TPIMG_DATA ".TP"
#define TPIMG_INDEX ".TPIMG"
#define TPIMG_TPF ".TPF"
// for tpimg ,two files :tp:datafile,tpimg: is the tpimg;
// tpimg: record a int for one recorder bytes: >0: record bytes,0:filemarks,0,0 endof tape:
// sgy:3200,400,5240,5340....0,0

#define Q2CH toUtf8().data()
#if 1
class DEV
{
public:
    QString name;// device name,filename
    QString id;//3480,3590,8mm...
    qint64 size; //capasity, filesize
    int type;// tape,disk,tapeimg
    QString reel;
    #if 1
    DEV  operator = (DEV p)
    {
        name = p.name;
        id = p.id;
        size = p.size;
        type  = p.type;
        reel = p.reel;
        return p;
    }
    #endif
};
#endif
class TPF
{
public:
    TPF(){bytes = -1;ic = -1;};
    int openFile(QString f);
    int writeBytes(int);
    int closeFile();
    QList<QString> listLine;
    int bytes,ic;
    QString fileName;

};
class dataIO
{ 
public:
    dataIO();
    ~dataIO();
    void init();
// open diskfile;
    /**
     * return 1 ok, -1 ERR
     */
    
    int open(int i);
    int open(QString s,int id);
    int open(QString s);

    int open(char * s,int id);
    int openFile(QString s,int id);
// open tape
    int openTape(QString s,int id);
    static QString tpimgFile(QString s);// if .TP or .TPIMG remove it;
    static QString tpimgDFile(QString s);
    static QString tpimgIFile(QString s);
    static QString tpimgTPFFile(QString s);
// open tpimg
    int openTPIMG(QString s,int id);
//
    int openDev(DEV*d,int mode);
//
    bool isReady();// true if ready
    bool isOpen();
// close
    int rewindClose(); //  for tape:
    int close();
    int read(BYTE *buf,int iby);
    int readLong(BYTE *buf,int iby);
    int write(BYTE *buf,int iby);   
         
    int rewind();// 0: ok   for all type
    int unload();// 0:ok,tape only
// for tape,tpimg below:
//    
    int writeEOF();//0 if ok,-1 :err 
    int writeEOF(int num);

    int fileForword();// 0:ok
    int fileForword(int num);// >0  if ok

    int recordForword();//0:ok
    int recordForword(int num);//   treat EOF as a record

    int fileBackword();// 0 if o
    int fileBackword(int num);

    int recordBackword();//0:ok
    int recordBackword(int num);// treat EOF as a record

    int status();// 0:ok

    int setType(int i);
    int getType();
// index location;
/**
*  calculate the bytes,move the ifilePtr.
*/
    int iNext(); // tpimg : indexfile point
    int iPre(); // tpimg : indexfile point
    qint64 iNextF();
    qint64 iPreF();

    int iWrite(BYTE *buf, int iby);// sequential write
    int iOpenRead();// read all,things to ifileBuf;
    int iOpenWrite();// init for block
    int iCloseWrite();// write last block
//
    int getTapeBlock();
    int setTapeBlock(int id);
    void h80();
    QString filename,filenameImg;

    qint64  size();
    qint64 seek(qint64 l);
    qint64 pos();
    int eofFlag, eof2Flag,eotFlag; 
    QMap<int,QString> tpErr;
    DEV dev;
    int iTapeBlock;
private:
   
    int  tapeio(int *iunit,char *opkey);
    QFile file,ifile;
    QStringList m_listFiles;
   

    int iunit;// file handle;
    int rwFlag ;// open for read || write 
    int devType;// DEV_DISK |DEV_TPIMG | DEV_TAPE
    
    QString m_status;
    char *ifileBuf;// read : all index buf,write:block buf;
    int ifileLen; // read: intLen of the file,write: length of lbuf;
    int ifilePtr;// pointer for ibuf;
    int *ibuf; // pointer to ifileBuf (int of);
  
    TPF tpf;

};


#endif // DATA_IO
