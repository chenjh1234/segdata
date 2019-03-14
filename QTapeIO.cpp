// dataIO.cpp: implementation of the dataIO class.
//
//////////////////////////////////////////////////////////////////////

#include "QTapeIO.h"

// use the low lever open read,write,close"
int myOpen(char *c, int m)
{
   return open(c, m);
}
int myRead(int i, char *b, int m)
{
   return read(i, b, m);
}
int myWrite(int i, char *b, int m)
{
   return write(i, b, m);
}
int myClose(int i)
{
   return close(i);
}
// TPF=========================================
int TPF::openFile(QString f)
{
   fileName = f;
   bytes = -1;
   ic = 0;
   return 0;
}
int TPF::writeBytes(int b)
{
   QString str;
   if (b != bytes)
   { // make line
     // not the 1st:
      if (bytes != -1)
      {
         if (bytes == 0)
         {
            str = QString("%1 %2").arg(ic, 10).arg("FM", 10);
         }
         else str = QString("%1 %2").arg(ic, 10).arg(bytes, 10);

         listLine.append(str);
         qDebug() << "str = " << str;
      }
      bytes = b;
      ic = 1;
   }
   else ic++;
   return 0;
}
int TPF::closeFile()
{
   QString str;
   int i,id;
   id = 0;
// last block:
   if (bytes == 0)
   {
      str = QString("%1 %2").arg(ic, 10).arg("FM", 10);
   }
   else str = QString("%1 %2").arg(ic, 10).arg(bytes, 10);
   
   listLine.append(str);
// write to file
   QFile f(fileName);
   if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
   {
      QTextStream out(&f);
      for (i = 0; i < listLine.size(); i++)
      {
         str = listLine[i];
         //qDebug() << "write = " << str;
         out << str << "\n";
      }
      f.close();
   }
   else
   {
      qDebug() << "open TPF file err : " << fileName;
      id = -1;
   }
   ic = 0; 
   bytes = -1;
   listLine.clear();
   return id;
}
//======================================================================
dataIO::dataIO()
{
   init();
}
dataIO::~dataIO()
{
   if (ifileBuf != NULL)
   {
      delete[] ifileBuf;
   }
}
void dataIO::init()
{
   TP_ERR();
   iunit = 0;
   devType = 0;
   eofFlag = 0;
   eotFlag = 0;
   eof2Flag = 0;
   ifileLen = 0;
   ifileBuf = NULL;
   ifilePtr = 0;
   iTapeBlock = 0;
   setTapeBlock(TAPE_BLOCK);
}
int dataIO::getTapeBlock()
{
   return iTapeBlock;

}
int dataIO::setTapeBlock(int id)
{
   int i; 
   if (id > iTapeBlock)
   {
       iTapeBlock = id; 
   }
   i = iTapeBlock;
   return i;
}

int dataIO::setType(int i)
{
   if (i != DEV_DISK && i != DEV_TAPE && i != DEV_TPIMG)
   {
      i = DEV_DISK;
   }
   devType = i;
   return 0;
}
int dataIO::getType()
{
   return devType;
}
int dataIO::openDev(DEV *d, int mode)
{
   dev = *d;
   switch (d->type)
   {
   case DEV_DISK:
      return openFile(d->name, mode);
      break;
   case DEV_TAPE:
      return openTape(d->name, mode);
      break;
   case DEV_TPIMG:
      return openTPIMG(d->name, mode);
   }
   return 0;

}

//open==================================
int dataIO::open(char *c, int id)
{
   QString s = c;
   return open(s, id);
}
int dataIO::open(QString s)
{
   int id;
   id = 0;
   return open(s, id);
}

int dataIO::open(QString s, int id)
{
   int ret;
   filename = s;
   file.setFileName(s);
   devType = DEV_DISK;
   rwFlag = id;
   if (s.left(5) == QString("/dev/"))
   {
      setType(DEV_TAPE);
   }
   if (id == FILEOPEN_READ) ret =  file.open(QIODevice::ReadOnly);
   if (id == FILEOPEN_WRITE) ret =  file.open(QIODevice::WriteOnly);
   if (id == FILEOPEN_RW) ret =  file.open(QIODevice::ReadWrite);
   if (ret) return OPENFILE_OK;
   else return OPENFILE_ERR;
}
//openfile==============================
int dataIO::openFile(QString s, int id)
{
   return open(s, id);
}
//openTape==============================
int dataIO::openTape(QString s, int id)
{
   int mode, ret;
   devType = DEV_TAPE;
   mode = O_RDONLY;
   rwFlag = id;
   if (id == FILEOPEN_READ) mode = O_RDONLY;
   if (id == FILEOPEN_WRITE) mode = O_WRONLY;
   if (id == FILEOPEN_RW) mode = O_RDWR;
   //qDebug() << s.toUtf8().data() ;
   ret = myOpen(s.toUtf8().data(), mode);
   if (ret > 0)
   {
      iunit = ret;
      ret = OPENFILE_OK;
   }
   else
   {
      ret = OPENFILE_ERR;
      iunit  = ret;
   }
   return ret;
}
QString dataIO::tpimgFile(QString s)
{
   if (s.endsWith(TPIMG_DATA)) return s.remove(s.lastIndexOf(TPIMG_DATA), QString(TPIMG_DATA).size());
   if (s.endsWith(TPIMG_INDEX)) return s.remove(s.lastIndexOf(TPIMG_INDEX), QString(TPIMG_INDEX).size());
   if (s.endsWith(TPIMG_TPF)) return s.remove(s.lastIndexOf(TPIMG_TPF), QString(TPIMG_TPF).size());
   return s;
}
QString dataIO::tpimgDFile(QString s)
{
   QString str;
   str = tpimgFile(s);
   str = str + TPIMG_DATA;
   return str;
}
QString dataIO::tpimgTPFFile(QString s)
{
   QString str;
   str = tpimgFile(s);
   str = str + TPIMG_TPF;
   return str;
}
QString dataIO::tpimgIFile(QString s)
{
   QString str;
   str = tpimgFile(s);
   str = str + TPIMG_INDEX;
   return str;
}
//openTPIMG========================================
int dataIO::openTPIMG(QString s, int id)
{
   int ret, i;
   QString dataFile, indexFile, tpfFile;
   rwFlag = id;
   s = tpimgFile(s);

   dataFile = s + TPIMG_DATA;
   indexFile = s + TPIMG_INDEX;
   tpfFile = s + TPIMG_TPF;

   filename = s;
   file.setFileName(dataFile);
   ifile.setFileName(indexFile);
   qDebug() << "openTPIMG =" << dataFile << indexFile << tpfFile;

   devType = DEV_TPIMG;
//data:
   if (id == FILEOPEN_READ) ret =  file.open(QIODevice::ReadOnly);
   if (id == FILEOPEN_WRITE) ret =  file.open(QIODevice::WriteOnly);
   if (id == FILEOPEN_RW) ret =  file.open(QIODevice::ReadWrite);
   qDebug() << "dret =" << ret;
   if (!ret) return OPENFILE_ERR;
// TPF:
   if (id == FILEOPEN_WRITE)
   {
      tpf.openFile(tpfFile);
   }

// idx:
   if (id == FILEOPEN_READ) ret =  ifile.open(QIODevice::ReadOnly);
   if (id == FILEOPEN_WRITE) ret =  ifile.open(QIODevice::WriteOnly);
   if (id == FILEOPEN_RW) ret =  ifile.open(QIODevice::ReadWrite);
   qDebug() << "idret =" << ret;
   if (!ret) return OPENFILE_INDEX_ERR;
   qDebug() << "start iopen";

   if (id == FILEOPEN_READ )//|| id == FILEOPEN_RW)
   {
      i =  iOpenRead();
      qDebug() << " iopen read =" << i;
      return i;
   }
   if (id == FILEOPEN_WRITE)
   {
      i =  iOpenWrite();
      qDebug() << "iopen write = " << i;
      return i;
   }
   return OPENFILE_ERR; // read | write
}
// close==================================
int dataIO::rewindClose()
{
   if (devType == DEV_TAPE)
   {
      rewind();
      myClose(iunit);
      iunit = 0;
   }

   return 0;
}

int dataIO::close()
{
   if (devType == DEV_DISK)
   {
      file.close();
   }
   if (devType == DEV_TPIMG)
   {
      // qDebug() << "bf delete";
      if (rwFlag == FILEOPEN_WRITE)
      {
         iCloseWrite();
         tpf.closeFile();
      }
      if (ifileBuf != NULL) delete[] ifileBuf;
      ifileBuf = NULL;
      ifilePtr = 0;
      ifileLen = 0;

      file.close();
      ifile.close();
   }
   if (devType == DEV_TAPE)
   {
      //rewind();
      myClose(iunit);
      iunit = 0;
   }

   return 0;
}
// inext======================
// ip   v
//  0   1   2   3    4
//  50  60  70  80   0
//dat ^
//
int dataIO::iNext()
{
   int bytes;
   //eofFlag = 0;
   //qDebug() << "iNext ptr,len = " << ifilePtr <<ifileLen ;
   if (ifilePtr >= ifileLen)
   {
      eotFlag = 1;
      return -1; // eot return -1 but set the flag of eotFlag;
   }
   bytes = ibuf[ifilePtr];
   ifilePtr++;
   return bytes;
}
int dataIO::iPre()
{
   int bytes;
   if (ifilePtr <= 0)
   {
      return -1;
   }
   ifilePtr--; // ok is rigtt
   bytes = ibuf[ifilePtr];
   return bytes;
}
// inext f=========================
qint64 dataIO::iNextF() // point to record after EOF
{
   qint64 sum;
   int bytes;
   sum = 0;
   while (1)
   {
      bytes  = iNext();
      if (bytes < 0) return -1;
      if (bytes == 0) return sum;
      sum = sum + bytes;
   }
   //return sum;
}
qint64 dataIO::iPreF() // point to record at EOF
{
   qint64 sum;
   int bytes;
   sum = 0;
   while (1)
   {
      bytes  = iPre();
      if (bytes < 0) break;
      if (bytes == 0) return sum;
      sum = sum + bytes;
   }
   return -1;
}
// iWrite====================================
int dataIO::iOpenWrite()
{
   int i;
   qint64 llen, block;
   llen = IBLOCK; // block size
   ifileLen = llen / 4;
   if (ifileBuf != NULL) delete[] ifileBuf;
   ifileBuf = new char[llen];
   ibuf = (int *)ifileBuf;
   ifilePtr = 0;
   return OPENFILE_OK;
}
int dataIO::iWrite(BYTE *buf, int iby)
{
   int bytes;
   if (ifilePtr >= ifileLen) // infact ==
   {
      ifilePtr = 0;
      bytes = ifile.write(ifileBuf, IBLOCK);
      if (bytes != IBLOCK) return WRITEFILE_INDEX_ERR;
   }
   ibuf[ifilePtr] = *((int *)buf);
   ifilePtr++;
   return iby;
}
int dataIO::iCloseWrite()
{
   int ibyte, ret;
   ret = 0;
   qDebug() << "close= " << ifilePtr;
   if (ifilePtr > 0) // infact ==
   {
      ibyte = ifile.write(ifileBuf, ifilePtr * sizeof(int));
      if (ibyte != ifilePtr * sizeof(int))  ret = WRITEFILE_INDEX_ERR;
      ifilePtr = 0;
   }
   return ret;
}

//iOPenRead:-------------------
int dataIO::iOpenRead()
{
   int i;
   qint64 llen, block, left, num;
   llen = ifile.size();
   ifileLen = llen / 4;
   if (ifileBuf != NULL) delete[] ifileBuf;
   ifileBuf = new char[llen];
   ibuf = (int *)ifileBuf;
   ifilePtr = 0;

   block = IBLOCK;

   num = llen / block;
   left = llen - num * block;
// num blocks:
   for (i = 0; i < num; i++)
   {
      ifile.read(ifileBuf + i * block, block);
   }
   // last block
   if (left != 0)
   {
      ifile.read(ifileBuf + num * block, left);
   }
   ifile.close();

   qDebug() << "in iopenRead" << left;
   return OPENFILE_OK;
}
//read============================================
int dataIO::read(BYTE *buf, int iby)
{
   int ret, bytes;
// disk?
   if (devType == DEV_DISK)
   {
      ret =  file.read((char *)buf, iby);
      bytes = ret;
      if (bytes >= 0)
      {
         // data block;
         eofFlag = 0;
         eotFlag = 0;
         if (ret != iby)
         {
            if (file.atEnd())
            {
               eotFlag = 1;
               eofFlag = 1;
               eof2Flag = 1;
            }
            else ret =  READFILE_ERR;
         }
      }
      else ret = READFILE_ERR;
      return ret;
   }

// tape:
   if (devType == DEV_TAPE)
   {
      iby = getTapeBlock();
      ret =  myRead(iunit, (char *)buf, iby);
      bytes = ret;
      if (bytes > 0)
      {
         // data block;
         eofFlag = 0;
         eotFlag = 0;
         eof2Flag = 0;
         // iby = TAPEBLOCK; iret < iby
         //if (ret != iby) ret =  READFILE_ERR;
      }
      else if (bytes == 0)
      {
         // eof: not read data
         if (eofFlag == 1) eof2Flag = 1;
         eofFlag = 1;
         qDebug() << "Tape read EOF ,eofFlag, eof2Flag =" << eofFlag << eof2Flag;
      }
      else if (bytes < 0) ret = READFILE_BYTE_ERR;
      return ret;
   }
//tpimg:
   if (devType == DEV_TPIMG)
   {
      // bytes of the block;
      //ret = ifile.read((char *)&bytes, sizeof(int));
      //if (ret != sizeof(int)) return READFILE_INDEX_ERR;
      // ifile ptr:
      bytes = iNext();
      //qDebug() << "iNext read bytes = " << bytes;
//data:
      if (bytes > 0)
      {
         // data block;
         //qDebug() << "in read normal = " << eofFlag << eotFlag;
         eofFlag = 0;
         eotFlag = 0;
         eof2Flag = 0;
        
         ret = file.read((char *)buf, bytes);
         // qDebug() << ": read file pos = " << file.pos();
         if (ret != bytes) ret = READFILE_ERR;
      }
      else if (bytes == 0)
      {
         // eof: not read data
         if (eofFlag == 1) eof2Flag = 1;
         eofFlag = 1;
         //qDebug() << "in read eof,eot = " << eofFlag << eotFlag;
         ret = 0;
      }
      else if (bytes < 0)
      {
         if (eotFlag == 1)  //iNext return -1 but set eotFlag;
         {
            // qDebug() << "eot1 = " << eotFlag;
            ret = 0;
         }
         else ret = READFILE_BYTE_ERR;
      }

      return  ret;
   }
   // not a type
   return READFILE_TYPE_ERR;

}

int dataIO::readLong(BYTE *buf, int iby)
{
   return read(buf, iby);
}
// rewind====================================
int dataIO::rewind()
{
   // disk?
   if (devType == DEV_DISK)
   {
       qDebug() << "disk rewind ";
      return file.seek(0);
   }
// tape:
   if (devType == DEV_TAPE)
   {
      return tapeio(&iunit, TP_REWIND);
   }
   //tpimg:
   if (devType == DEV_TPIMG)
   {
      ifilePtr = 0;
      return file.seek(0);
   }
   return 0;
}

//write======================================

int dataIO::write(BYTE *buf, int iby)
{

   int ret, bytes;
// disk?
   if (devType == DEV_DISK)
   {
      ret = file.write((char *)buf, iby);
      if (ret != iby) ret = WRITEFILE_ERR;

      return ret;
   }
// tape:
   if (devType == DEV_TAPE)
   {
       if (iby == 0)
           ret = writeEOF();
       else
           ret = myWrite(iunit, (char *)buf, iby);
      if (ret != iby) ret = WRITEFILE_ERR;
      return ret;
   }
//tpimg:
   bytes = iby;
   if (devType == DEV_TPIMG)
   {
      // bytes of the block;
 // tpf:
      tpf.writeBytes(bytes);
      //ret = ifile.write((char *)&bytes, sizeof(int));
      ret = iWrite((BYTE *)&bytes, sizeof(int)); //TPIMAGfile write                                            //TPF file write;
      //qDebug() << "after write bytes = " << bytes;
      if (ret != sizeof(int)) return WRITEFILE_INDEX_ERR;

      if (bytes > 0)
      {
         // data block;
         ret = file.write((char *)buf, bytes);
         //qDebug() << ": write file pos = " << file.pos();
         if (ret != bytes) return WRITEFILE_ERR;
      }
      else if (bytes == 0)
      {
         // eof: not read data
         ret = 0;
      }
      else if (bytes < 0) return WRITEFILE_BYTE_ERR;
      return  ret;
   }
// not a type
   return WRITEFILE_TYPE_ERR;

}
//size= =seek=====pos=
qint64 dataIO::size()
{
   if (devType == DEV_DISK) return file.size();
   return 0;
}
qint64 dataIO::seek(qint64  l)
{
   if (devType == DEV_DISK) return file.seek(l);
   return 0;
}

qint64 dataIO::pos()
{
   if (devType == DEV_DISK) return file.pos();
   return 0;

}
// unload:=============================
int dataIO::unload()
{
   if (devType == DEV_TAPE)
   {
      return tapeio(&iunit, TP_UNLOAD);
   }
   return 0;
}
// unload:=============================
int dataIO::status()
{
   if (devType == DEV_TAPE)
   {
      return tapeio(&iunit, TP_STA);
   }
   return 0;
}
//eof:================================

int dataIO::writeEOF()
{
   int bytes, ret;
   if (devType == DEV_TAPE)
   {
      return tapeio(&iunit, (char *)TP_EOF);
   }

   bytes = 0;
   if (devType == DEV_TPIMG)
   {
      // bytes of the block;

      //ret = ifile.write((char *)&bytes, sizeof(int));
      ret = iWrite((BYTE *)&bytes, sizeof(int));
      if (ret != sizeof(int)) return WRITEFILE_INDEX_ERR;
      return 0;
   }
   return 0;
}
bool dataIO::isOpen()
{
   return isReady();
}
bool dataIO::isReady()
{
   if (devType != DEV_TAPE)  return file.isOpen();
   else return iunit;
}
// file forward===============
int dataIO::fileForword()
{
   qint64 bytes;
   int ret;

   if (devType == DEV_TAPE)
   {
      return tapeio(&iunit, (char *)TP_FWF);
   }

   bytes = 0;
   if (devType == DEV_TPIMG)
   {
      // read ifile util bytes == 0;

      bytes = iNextF();
      qDebug() << "file forword ret = " << bytes;
      if (bytes > 0)
      {
         // data block;
         eofFlag = 0;
         eotFlag = 0;
         eof2Flag = 0;
         file.seek(file.pos() + bytes);
      }
      if (bytes < 0) return -1;
      return  0;
   }
   return 0;
}
int dataIO::fileForword(int num)
{
   int i;
   for (i = 0; i < num; i++)
   {
      if (fileForword() != 0) return i;
   }
   return num;
}
// record forward:==============
int dataIO::recordForword(int num)
{
   int i;
   for (i = 0; i < num; i++)
   {
      if (recordForword() != 0) return i;
   }
   return num;
}
int dataIO::recordForword()
{
   int bytes, ret;
   if (devType == DEV_TAPE)
   {
      return tapeio(&iunit, (char *)TP_FWR);
   }

   bytes = 0;
   if (devType == DEV_TPIMG)
   {
      bytes = iNext();

      if (bytes > 0)
      {
         // data block;
         eofFlag = 0;
         eof2Flag = 0;
         eotFlag = 0;
         file.seek(file.pos() + bytes);
      }
      else if (bytes == 0)
      {
         // eof: not read data
         if (eofFlag == 1) eof2Flag = 1;
         eofFlag = 1;
         ret = 0;
      }
      else if (bytes < 0) return READFILE_BYTE_ERR;
      return 0;
   }
   return 0;
}
// recordBackword:-----------------

int dataIO::recordBackword(int num)
{
   int i;
   for (i = 0; i < num; i++)
   {
      if (recordBackword() != 0) return i;
   }
   return num;
}
int dataIO::recordBackword()
{
   int bytes, ret;
   if (devType == DEV_TAPE)
   {
      return tapeio(&iunit, (char *)TP_BKR);
   }

   bytes = 0;
   if (devType == DEV_TPIMG)
   {
      bytes = iPre();

      if (bytes > 0)
      {
         // data block;
         eofFlag = 0;
         eof2Flag = 0;
         eotFlag = 0;
         file.seek(file.pos() - bytes); // void

      }
      else if (bytes == 0)
      {
         // eof: not read data
         //if (eofFlag == 1) eotFlag = 1;
         eofFlag = 1;
         ret = 0;
      }
      else if (bytes < 0) return READFILE_BYTE_ERR;
      return 0;
   }
   //others return 0;
   return 0;
}
// file backward===============
int dataIO::fileBackword()
{
   qint64 bytes;
   int ret;

   if (devType == DEV_TAPE)
   {
      return tapeio(&iunit, (char *)TP_BKF);
   }

   bytes = 0;
   if (devType == DEV_TPIMG)
   {
      // read ifile util bytes == 0;

      bytes = iPreF();
      qDebug() << "bkf bytes = " << bytes;
      if (bytes > 0)
      {
         // data block;
         //eofFlag = 0;
         //qDebug() << "bkf bytes = " <<bytes;
         file.seek(file.pos() - bytes);
      }
      if (bytes < 0) return -1;
      return  0;
   }
   return 0;
}
int dataIO::fileBackword(int num)
{
   int i;
   for (i = 0; i < num; i++)
   {
      if (fileBackword() != 0) return i;
   }
   return num;
}


#if 0
int dataIO::tapeio(int *iunit, char *opkey)
{
   return 0;
}
#endif

#if  !defined(WIN32)
struct mtop  tap;
#endif
int dataIO::tapeio(int *iunit, char *opkey)
{
#if  !defined(WIN32)
   /* struct mtop  tap;
   */
   QString str;
   int i, ist;
   int noring, eof, bot, ok, eot, err, rdy;
   short ifc = (-1);
   char chstring[4];
   tap.mt_count = 1;

   str = "";
   noring = 16;
   eof    = -2;
   eot    = 128;
   bot    = 64;
   ok     = 0;
   rdy    = 3;
   err    = -1;

   for (i = 0; i < 3; i++)
   {
      chstring[i] = opkey[i];
   }
   chstring[3] = '\0';
   if (strcmp(chstring, "eof") == 0) ifc = MTWEOF;
   if (strcmp(chstring, "fwf") == 0) ifc = MTFSF;
   if (strcmp(chstring, "bkf") == 0) ifc = MTBSF;
   if (strcmp(chstring, "fwr") == 0) ifc = MTFSR;
   if (strcmp(chstring, "bkr") == 0) ifc = MTBSR;
   if (strcmp(chstring, "rew") == 0) ifc = MTREW;
   if (strcmp(chstring, "uld") == 0) ifc = MTOFFL;
   if (strcmp(chstring, "sta") == 0) ifc = MTNOP;

   tap.mt_op = ifc;
   ist = ioctl(*iunit, MTIOCTOP, &tap);
   if (ist == -1)        str = "ERROR";
   else if (ist == 16)   str = "NORING";
   else if (ist == -2)   str = "EOF";
   else if (ist == 128)  str = "EOT";
   else if (ist == 64)   str = "BOT";
   else if (ist == 3)    str = "NORDY";
   else if (ist == 0)    str = "OK";

   m_status = str;
   return ist;
#endif
}
void  dataIO::h80()
{
   QString filen = "h801";
   dataIO tp;
   unsigned char buf[10000];
   int *ip, len, it, l, i;
   len = 80;
   l = 0;
   tp.openTPIMG(filen, 1);
   qDebug() << "open ok";
// header 80*3
   for (i = 0; i < 3 ; i++)
   {
      //qDebug() << "w ok";
      memset(buf, i, len);
      it = tp.write(buf, len);
      if (it != len)
      {
         qDebug() << "write heder error 0";
         return;
      }
   }   
  // eof:
   it = tp.write(buf, l);
// 10 data of 1000;
   len  =1000;
   for (i = 0; i < 10; i++)
   {
      memset(buf, i+10, len);
      it = tp.write(buf, len);
      if (it != len)
      {
         qDebug() << "write data error 0";
         return;
      }
   }
  // tail 80*3:
   len =80;
   for (i = 0; i < 3 ; i++)
   {
      //qDebug() << "w ok";
      memset(buf, i+20, len);
      it = tp.write(buf, len);
      if (it != len)
      {
         qDebug() << "write tail error 0";
         return;
      }
   }   
// 2 eof:

   it = tp.write(buf, l);
   it = tp.write(buf, l);
   tp.close();
   qDebug() << "close";

}


//=====================================================================
#if 0 // test tpimg:=========================================
QString filen = "tptest1";
// ifile: 5*4=100 ,eof,5*4=100,eof,eof
// date:  01234,eof,56789,eof,eof
void testW()
{
   dataIO tp;
   unsigned char buf[100];
   int *ip, len, it, l, i;
   len = 20;
   l = 0;
   tp.openTPIMG(filen, 1);
   qDebug() << "open ok";
   for (i = 0; i < 5; i++)
   {
      //qDebug() << "w ok";
      memset(buf, i, len);
      it = tp.write(buf, len);
      if (it != len)
      {
         qDebug() << "write error 0";
         return;
      }
   }
   it = tp.write(buf, l);
   for (i = 0; i < 5; i++)
   {
      memset(buf, i + 5, len);
      it = tp.write(buf, len);
      if (it != len)
      {
         qDebug() << "write error 0";
         return;
      }
   }
   it = tp.write(buf, l);
   it = tp.write(buf, l);
   tp.close();
   qDebug() << "close";

}

void testW1()
{
   dataIO tp;
   unsigned char buf[5000];
   int *ip, len, it, l, i, j, num;
   len = 2000;
   l = 0;
   tp.openTPIMG(filen, 1);
   qDebug() << "open ok";
   j = 0;
   num = 100;
   while (1)
   {
      j ++;

      for (i = 0; i < num; i++)
      {
         //qDebug() << "w ok";
         memset(buf, i/10, len);
         it = tp.write(buf, len);
         if (it != len)
         {
            qDebug() << "write error 0";
            return;
         }
      }
      it = tp.write(buf, l);
      if (j >= 100) break;
   }

   it = tp.write(buf, l);
   tp.close();
   qDebug() << "close";

}
void testR()
{
   qDebug() << "test read :";
   dataIO tp;
   unsigned char buf[100];
   int *ip, len, it, l, i;
   len = 10;
   l = 0;
   tp.openTPIMG(filen, 0);


   for (i = 0; i < 14; i++)
   {
      it = tp.read(buf, len);
      qDebug() << i << it << buf[0] << buf[1];
   }
   tp.close();


}
void testR1()
{
   qDebug() << "test read :";
   dataIO tp;
   unsigned char buf[100];
   int *ip, len, it, l, i;
   len = 10;
   l = 0;
   tp.openTPIMG(filen, 0);

   it = tp.read(buf, len);
   qDebug() << "read= 0" << it << buf[0] << buf[1];
   it = tp.fileForword();
   qDebug() << "fwf " << it << buf[0] << buf[1];
   it = tp.read(buf, len);
   qDebug() << "read=5" << it << buf[0] << buf[1];
   it = tp.fileForword();
   qDebug() << "fwf" << it << buf[0] << buf[1];
   it = tp.read(buf, len);
   qDebug() << "read = EOF to end of file" << it << buf[0] << buf[1];
   it = tp.fileBackword(3);
   qDebug() << "bkf3 " << it << buf[0] << buf[1];
   it = tp.read(buf, len);
   qDebug() << "read=EOF " << it << buf[0] << buf[1];
   it = tp.read(buf, len);
   qDebug() << "read=5 " << it << buf[0] << buf[1];

   tp.close();


}
#endif
#if 0
void testimgF()
{
   dataIO tp;
   QString str;
   str = QString("111") + TPIMG_DATA;
   qDebug() << str << "->" << tp.tpimgFile(str);
   str = QString("222") + TPIMG_INDEX;
   qDebug() << str << "->" << tp.tpimgFile(str);
   str = "22211";
   qDebug() << str << "->" << tp.tpimgFile(str);
}
main()
{
   testW1(); // ok
             //testR();
             //testR1();
             //testimgF();

}

#endif
