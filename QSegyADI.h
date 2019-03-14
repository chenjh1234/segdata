// QSegyADI.h: interface for the QSegyADI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SEGY_ADI_H)
#define SEGY_ADI_H

#include "QString"
#include <fcntl.h>      /* Needed only for _O_RDWR definition */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>



// for PC:
//#include <io.h>
#include <QFile>
#include <QString>
#include <QThread>
#include <QDebug>

#define SEGY_MAX_GROUPS 9000
#define MAX_SAMPLES 9000
#define SEGY_HEAD_BYTES 240
#define SEGY_HEAD_WORDS 60
#define CPC 0
#define CSUN 1

/*================ SEGY 400 binary header ===============================*/
#pragma pack(4)
typedef struct _SEGY_BIN_400
{
   /*1==3201-3204*/     int  jobnum;         /* Job number.                          */
   /*2==3205-3208*/     int  line_num;       /* Line number.
                                                  (only one line per reel.)          */
   /*3==3209-3212*/     int  reel_num;       /* Reel number.
                                                  reel number of tape.               */
   /*4==3213-3214*/     short traces;        /* Total traces number.
                                                  number of data traces per ensemble */
   /*4==3215-3216*/     short auxtrace;      /* Auxiliary traces.
                                                  Number of auxiliary traces/record  */
   /*5==3217-3218*/     short si;            /* Sample interval.
                                                  Sample interval (us) for this reel */
   /*5==3219-3220*/     short osi;           /* Original sample interval.
                                                  Original Field record of
                                                  sample interval (us).              */
   /*6==3221-3222*/     short samples;       /* Number samples.
                                                  Number of samples per data trace   */
   /*6==3223-3224*/     short osamples;      /* Original Number samples.
                                                  Field record Number samples
                                                  per data trace.                    */
   /*7==3225-3226*/     short format;        /* Sample format:                       */
   /*   1 = 4-byte IBM floating-point.
        2 = 4-byte two's complement integer.
        3 = 2-byte two's complement integer.
        5 = 4-byte IEEE floating-point.
        8 = 1-byte two's complement integer.*/
   /*7==3227-3228*/     short fold;          /* Ensemble fold.
                                                  Maximum CDP fold per trace ensemble*/
   /*8==3229-3230*/     short sortcode;      /* Trace sorting code:
                                                 -1 = Other
                                                  0 = Unknown
                                                  1 = As reaorded(no sorting)
                                                  2 = CDP ensemble
                                                  3 = Single fold continuos profile
                                                  4 = Horizontally stacked
                                                  5 = Common source point
                                                  6 = Common receiver point
                                                  7 = Common offset point
                                                  8 = Common mid-point
                                                  9 = Common conversion point        */
   /*8==3231-3232*/     short vscode;        /* Vertical sum code:
                                                  1 = no sum,
                                                  2 = two sum...
                                                  N = M-1 sum (M = 2 to 32,767)      */
   /*9==3233-3234*/     short hsfs;          /* Sweep frequency at start(Hz).        */
   /*9==3235-3236*/     short hsfe;          /* Sweep frequency at end(Hz).          */
   /*10==3237-3238*/    short hslen;         /* Sweep length (ms).                   */
   /*10==3239-3240*/    short hstyp;         /* Sweep type code:
                                                  1 = linear
                                                  2 = parabolic
                                                  3 = exponential
                                                  4 = other                          */
   /*11==3241-3242*/    short schan;         /* Trace number of sweep channel.       */
   /*11==3243-3244*/    short hstas;         /* Sweep trace taper length at start.
                                                  Sweep trace taper length in
                                                  milliseconds at start if tapered   */
   /*12==3245-3246*/    short hstae;         /* Sweep trace taper length at end.     */
   /*12==3247-3248*/    short htatyp;        /* Taper type:
                                                  1 = linear
                                                  2 = cos-squared
                                                  3 = other                          */
   /*13==3249-3250*/    short hcorr;         /* Correlated data traces:
                                                  1 = no
                                                  2 = yes                            */
   /*13==3251-3252*/    short bgrcv;         /* Binary gain recovered:
                                                  1 = yes
                                                  2 = no                             */
   /*14==3253-3254*/    short rcvm;          /* Amplitude recovery method:
                                                  1 = none
                                                  2 = spherical divergence
                                                  3 = AGC
                                                  4 = other                          */
   /*14==3255-3256*/    short mfeet;         /* Measurement system:
                                                  1 = Meters
                                                  2 = Feet                           */
   /*15==3257-3258*/    short polyt;         /* Impulse signal polarity:
                                                  1 = increase in pressure or upward
                                                      geophone case movement gives
                                                      negative number on tape
                                                  2 = increase in pressure or upward
                                                      geophone case movement gives
                                                      positive number on tape        */
   /*15==3259-3260*/    short vpol;          /* Vibratory polarity code:
                                                 code    seismic signal lags pilot by:
                                                  1   -   337.5 to  22.5 degrees
                                                  2   -    22.5 to  67.5 degrees
                                                  3   -    67.5 to 112.5 degrees
                                                  4   -   112.5 to 157.5 degrees
                                                  5   -   157.5 to 202.5 degrees
                                                  6   -   202.5 to 247.5 degrees
                                                  7   -   247.5 to 292.5 degrees
                                                  8   -   293.5 to 337.5 degrees     */
   /*16-75==3261-3500*/ short unass[120];   /* Unassigned                            */
   /*76==3501-3502*/    short vernum;       /* SEG Y Format Revision Number.         */
   /*76==3503-3504*/    short fixed;        /* Fixed length trace flag.              */
   /*77==3505-3506*/    short exthead;      /* Number of 3200-byte,Extended Textual
                                                  File Header records following the
                                                  Binary Header.                     */
   /*77-100=3507-3600*/ short hunass[47];
}SEGYBIN400;                             /* SEGY binary header                    */

/*************+++++++++ SEGY 240 trace header ++++++++++*******************/

/*************===========================================***********************/

typedef struct _SEGY_TR_HEADER_
{
   /*1==1--4*/     int   num_trace_line;  /*Trace sequence number within line.            */
   /*2==5--8*/     int   num_trace_reel;  /*Trace sequence number within files.           */
   /*3==9--12*/    int   field_shot;      /*Original field record number.                 */
   /*4==13--16*/   int   field_trace_num; /*Trace number.
                                            within the original field record              */
   /*5==17--20*/   int   shot_num;        /*Energy source point number.                   */
   /*6==21--24*/   int   cdp_num;         /*CDP number.                                   */
   /*7==25--28*/   int   cdp_traces;      /*Trace number within CDP.                      */
   /*8==29--30*/   short traceid;         /*Trace identification code:
                                           -1 - Other
                                            0 - Unknown
                                            1 - Seismic data,
                                            2 - Dead,
                                            3 - Dummy,
                                            4 - Time break,
                                            5 - Uphole,
                                            6 - Sweep,
                                            7 - Timing,
                                            8 - Waterbreak,
                                            9 - Near-field gun signature.
                                           10 - Far-field gun signature.
                                           11 - Seismic pressure sensor.
                                           12 - Multicomponent seismic sensor
                                                  - Vertical component.
                                           13 - Multicomponent seismic sensor
                                                  - Cross-line component.
                                           14 - Multicomponent seismic sensor
                                                  - In-line component.
                                           15 - Rotated multicomponent seismic sensor
                                                  - Vertical component.
                                           16 - Rotated multicomponent seismic sensor
                                                  - Transverse component.
                                           17 - Rotated multicomponent seismic sensor
                                                  - Radial component.
                                           18 - Vibrator reaction mass.
                                           19 - Vibrator baseplate.
                                           20 - Vibrator estimated ground force.
                                           21 - Vibrator reference.
                                           22 - Time-velocity pairs.
                                           23...N - optional use,(maximum N=32767)      */
   /*8==31--32*/    short numvs;         /*Number of vertically summed traces.          */
   /*9==33--34*/    short num_horiz;     /*Number of horizontally stacked traces.       */
   /*9==35--36*/    short datause;       /*Data use.
                                             1=production,
                                             2=test)                                    */
   /*10==37--40*/   int   offset;        /*Offset.
                                            Distance from source point to receiver group */
   /*11==41--44*/   int   receiver_elev; /*Receiver group elevation.                     */
   /*12==45--48*/   int   source_elev;   /*Surface elevation at source.                  */
   /*13==49--52*/   int   well_depth;    /*Source depth below surface.                   */
   /*14==53--56*/   int   datum_elev_rp; /*Datum elevation at receiver group.            */
   /*15==57--60*/   int   datum_elev_sp; /*Datum elevation at source.                    */
   /*16==61--64*/   int   source_wd;     /*Water depth at source.                        */
   /*17==65--68*/   int   receiver_wd;   /*Water depth at group.                         */
   /*18==69--70*/   short scalel;        /*Scale for elevation.
                                             Trace Header bytes 41-68 to give the
                                             real value.
                                             Scalar=1,+10,+100,+1000,or +10000.
                                             if positive,scalar is used as a multiplier;
                                             if negative,scalar is used as divisor.     */
   /*18==71--72*/   short scalco;        /*Scaler for coordinates.
                                             Trace Header bytes 73-88 and 181-188
                                             to give the real value.
                                             Scalar=1,+10,+100,+1000,or +10000.
                                             if positive,scalar is used as a multiplier;
                                             if negative,scalar is used as divisor      */
   /*19==73--76*/   int   xsp;           /*Source coordinate - X.                       */
   /*20==77--80*/   int   ysp;           /*Source coordinate - Y.                       */
   /*21==81--84*/   int   xgp;           /*Group coordinate - X.                        */
   /*22==85--88*/   int   ygp;           /*Group coordinate - Y.                        */
   /*23==89--90*/   short coord_unit;    /*Coordinate units:
                                            1 = Length (meters or feet)
                                            2 = Seconds of arc
                                            3 = Decimal degrees
                                            4 = Degrees,minutes,second(DMS)             */
   /*23==91--92*/   short speed_wind;    /*Weathering velocity.                         */
   /*24==93--94*/   short speed_sub;     /*Subweathering velocity.                      */
   /*24==95--96*/   short time_sp;       /*Uphole time at source(ms).                   */
   /*25==97--98*/   short time_pr;       /*Uphole time at group(ms).                    */
   /*25==99--100*/  short static_sp;     /*Source static correction(ms).                */
   /*26==101--102*/ short static_pr;     /*Group static correction(ms).                 */
   /*26==103--104*/ short static_sum;    /*Total static applied(ms).                    */
   /*27==105--106*/ short delay_ta;      /*Lag time A (ms).
                                             time in ms between end of 240-
                                             byte trace identification header and time
                                             break, positive if time break occurs after
                                             end of header, time break is defined as
                                             the initiation pulse which maybe recorded
                                             on an auxiliary trace or as otherwise
                                             specified by the recording system          */
   /*27==107--108*/ short delay_tb;      /*Lag time B (ms).
                                            time in ms between the time break
                                            and the initiation time of the energy source,
                                            may be positive or negative                 */
   /*28==109--110*/ short delay_record;  /*Delay recording time(ms).
                                            between initiation time of energy source
                                            and time when recording of data samples
                                            begins.                                     */
   /*28==111--112*/ short mute_start;    /*Mute start time.                             */
   /*29==113--114*/ short mute_end;      /*Mute end time.                               */
   /*29==115--116*/ short nsamples;      /*Number of samples in this trace.             */
   /*30==117--118*/ short si_tr;         /*Sample interval(ms).                         */
   /*30==119--120*/ short gain_type;     /*Gain type of field instruments code:
                                            1 = fixed
                                            2 = binary
                                            3 = floating point
                                            4 -- N = optional use                       */
   /*31==121--122*/ short gain_const;    /*Instrument gain constant(dB).                */
   /*31==123--124*/ short init_gain;     /*Instrument early or initial gain(dB).        */
   /*32==125--126*/ short corr;          /*Correlated:
                                            1 = no
                                            2 = yes                                     */
   /*32==127--128*/ short trsfs;         /*Sweep frequency at start(Hz).                */
   /*33==129--130*/ short trsfe;         /*Sweep frequency at end(Hz).                  */
   /*33==131--132*/ short trslen;        /*Sweep length(ms).                            */
   /*34==133--134*/ short trstype;       /*Sweep type:
                                            1 = linear
                                            2 = parabolic
                                            3 = exponential
                                            4 = other                                   */
   /*34==135--136*/ short start_time;    /*Sweep trace taper length at start(ms).       */
   /*35==137--138*/ short end_time;      /*Sweep trace taper length at end(ms).         */
   /*35==139--140*/ short tape_type;     /*Taper type:
                                            1 = linear
                                            2 = cos^2
                                            3 = other                                   */
   /*36==141--142*/ short filter_alias;  /*Alias filter frequency,if used.              */
   /*36==143--144*/ short filter_slope;  /*Alias filter slope.                          */
   /*37==145--146*/ short limit_filter;  /*Notch filter if used.                        */
   /*37==147--148*/ short limit_slope;   /*Notch filter slope.                          */
   /*38==149--150*/ short low_filter;    /*Low cut frequency, if used.                  */
   /*38==151--152*/ short high_filter;   /*High cut frequncy, if used.                  */
   /*39==153--154*/ short low_tape;      /*Low cut frequency, if used.                  */
   /*39==155--156*/ short high_tape;     /*High cut slope.                              */
   /*40==157--158*/ short year;          /*Year data recorded.                          */
   /*40==159--160*/ short day;           /*Day of year.                                 */
   /*41==161--162*/ short clock;         /*Hour of day (24 hour clock).                 */
   /*41==163--164*/ short minutes;       /*Minute of hour.                              */
   /*42==165--166*/ short sec;           /*Second of minute.                            */
   /*42==167--168*/ short ttime;         /*Time basis code:
                                            1 = local
                                            2 = GMT
                                            3 = other                                   */
   /*43==169--170*/ short weight;        /*Trace weighting factor.
                                             defined as 1/2^N volts for the least
                                             sigificant bit.(N=0,1,...32767)            */
   /*43==171--172*/ short roll_group_num; /*Geophone group number.
                                              of roll switch position one                */
   /*44==173--174*/ short first_group_num; /*Geophone group number.
                                              of trace number one within original
                                              field record.                              */
   /*44==175--176*/ short last_group_num; /*Geophone group number.
                                             of last trace within original field record.*/
   /*45==177--178*/ short gap_size;       /*Gap size.
                                              (total number of groups dropped           */
   /*45==179--180*/ short otrav;          /*Over travel taper code:
                                              1 = down (or behind)
                                              2 = up (or ahead)                         */
   /*46==181-184*/  int   cdp_x;          /*CDP coordinate - X.                         */
   /*47==185-188*/  int   cdp_y;          /*CDP coordinate - Y.                         */
   /*48==189-192*/  int   linenumin;      /*3D in_line number.                          */
   /*49==193-196*/  int   linenumcross;   /*3D cross_line number.                       */
   /*50==197-200*/  int   shotponit_num;  /*Shotpoint number.                           */
   /*51==201-202*/  short scal_shotpoint; /*Scalar for shotpoint number.
                                              to be applied to the shotpoint number
                                              in Trace Header bytes 197-200 to give the
                                              real value.  If positive, scalar is used as
                                              a multiplier; if negative as a divisor;
                                              if zero the shotpoint number is not scaled.*/
   /*51==203-204*/  short trace_unit;     /*Trace value measurement unit:
                                             -1 = Other (should be described in Data
                                                  Sample Measurement Units Stanza)
                                              0 = Unknown
                                              1 = Pascal (Pa)
                                              2 = Volts (v)
                                              3 = Millivolts (mV)
                                              4 = Amperes (A)
                                              5 = Meters (m)
                                              6 = Meters per second (m/s)
                                              7 = Meters per second squared (m/s^2)
                                              8 = Newton (N)
                                              9 = Watt (W)                               */
   /*52==205-208*/  int   trans_mantissa;  /*Transduction Constant-A.                    */
   /*53==209-210*/  short trans_power;     /*Transduction Constant-B.
                                              (i.e. Bytes 205-208 * 10**Bytes 209-210).  */
   /*53==211-212*/  short trans_unit;      /*Transduction Units.
                                              The unit of measurement of the Data Trace
                                              samples after they have been multiplied by
                                              the Transduction Constant specified in Trace
                                              Header bytes 205-210.
                                             -1 = Other (should be described in Data
                                                  Sample Measurement Unit stanza,
                                              0 = Unknown
                                              1 = Pascal (Pa)
                                              2 = Volts (v)
                                              3 = Millivolts (mV)
                                              4 = Amperes (A)
                                              5 = Meters (m)
                                              6 = Meters per second (m/s)
                                              7 = Meters per second squared (m/s^2)
                                              8 = Newton (N)
                                              9 = Watt (W)                              */
   /*54==213-214*/  short deviceid;       /*Device id.                                  */
   /*54==215-216*/  short scalar_time;    /*Scalar to be applied to times.
                                              specified in Trace Header bytes 95-114 to
                                              give the true time value in milliseconds.
                                              Scalar = 1, +10, +100, +1000, or +10,000. */
   /*55==217-218*/  short source_type;    /*Source Type/Orientation.                    */
   /*55==219-220*/  short source_orie1;   /*Source Energy Direction.
                                              with respect to the source orientation    */
   /*56==221-224*/  int   source_orie2;   /*The positive orientation direction.
                                              is defined in Bytes 217-218 of the Trace
                                              Header.  The energy direction is encoded in
                                              tenths of degrees (i.e. 347.8(degr) is
                                              encoded as 3478).                         */
   /*57==225-228*/  int   source_mantissa; /*Source Measurement.
                                               Describes the source effort used to
                                               generate the trace.                       */
   /*58==229-230*/  short source_power;   /*Source Measurement weight.
                                              (i.e. Bytes 225-228 * 10**Bytes 229-230). */
   /*58==231-232*/  short source_unit;    /*Source Measurement unit.                    */
   /*59-60==233-240*/short unass[4];      /*Unassigned.                                 */
} SEGYTR240;;
#pragma pack()
/******* SEGY  240 trace header  ***************************************/

/**
      
     about this class:\n

      1: input and output:\n

         if input and output we use the same ltr,si and sample , \n 
         we can do it in the sam class.the output format is 2:fix(default)\n
         otherwise we use two class one for input ,other for output\n
         when write:\n
         before use write400();we fill m_infoGHD information,\n
                      or copy from other class use GetBuf400();\n
         we can use SetWriteFormat to change the output format :2,5\n
        

      2: input\n

        sgy = new QSegyADI();\n
        sgy->OpenRead(str);\n
        sgy->ReadTraces();\n
        ....\n
        sgy->CloseRead();\n


      3: output

        sgy = new QSegyADI();
        sgy->OpenWrite(str);\n
        sgy->Write3200();\n
        sgy->GetBuf400();\n
        change 400 information\n
        sgy->Write400();\n
        sgy->WriteTraces();\n
        ....\n
                sgy->CloseWrite();\n


    */

#include "QSegCommon.h"

class QSegyADI;
class segyThread;
#if 1
class segyThread : public QThread
{
    Q_OBJECT
public:
    segyThread();
  
    QString _file;
    int _hd,_ret;

    void setFile(QString file,int hd){_file = file; _hd = hd;};
    void run();
    int getRet(){return _ret;};
    QString getFile(){return _file;}; 

                        
};
#endif
class QSegyADI : public QSegCommon
{
    Q_OBJECT
public:

   QSegyADI();
   ~QSegyADI();
/**
    The inportant message in segy 400 bytes

*/


   typedef struct
   {
      int iJobNo;            //1-4
      int iLineNo;           //5-8
      int iReelNo;           //9-12
      int iTrs;              //13-14
      int iAuxs;             //15-16
      int iSi;              //17-18
      int iOSi;             //19-20
      int iSams;            //21-22
      int iOSams;           //23-24
      int iFormat;           //25-26//1:ibm,2:long,3:short,4:5:ieee
      int iFold;         //27-28 yzy cmp cover num
      int iUnit;
      int iDataType;        //29-30 ;1:shot,2:cmp,3:single stack,4:stack
      int iVerCode;          //31-32 yzy
      int iStartFreq;        //33-34 yzy
      int iEndFreq;          //35-36 yzy
      int iScanWindowTime;   //37-38 yzy
      int iScanType;         //39-40
      int iScanChTrc;       //41-42
      int iTScanStartTime;   //43-44
      int iTScanEndTime;     //45-46
      int iTScanType;        //47-48
      int iRelTrs;           //49-50
      int iGain;             //51-52 yzy
      int iAmpType;          //53-54;1:non,2:serphial 3,agc,4:other
      int iPer;              //55-56
      int iDirection;        //57-58
      int iFrDirection;      //59-60
   }SEGY_400_INFO;



/**

 About index file:

 - hdnumber,2:all gathers:3:
 - INDEX_INFO:idx,num.grp:
     - When read:
        - if we read segy read sequentially,We donot need IDX file:
        - OpenRead--->ReadTrace()---->CloseRead();
        - if we read segy data and want to access the Specified gather,We need need IDX file:
        - use CreateIdxFile() to creat a index file
        - OpenRead---Seetxxxx----->>ReadTrace()---->CloseRead();
     - When write:
        - we only can write segy data sequentially
*/
   typedef struct
   {
      int idx;
      int num;
      int grp;
   }INDEX_INFO;

//public:


   //bool m_bDiskFlag; //true:disk,false :tape;

//	QFile ifile,ofile;
   QString m_strFilename,m_strFilenameIdx;
   QString m_strFilenameO;
   QString getKey();//gather key:cmp,shot,line



   int m_piHead[SEGY_HEAD_WORDS];
//	unsigned char m_pBuf[MAX_SAMPLES*4];//read trace buffer;
   unsigned char m_pBuf400[400+10]; ///read info buffer;
   unsigned char m_pBuf3200[3200+10]; ///read info buffer;

//	MAINHD_INFO m_infoMHD;
   SEGY_400_INFO m_infoGHD;
   //SEGY_400_INFO m_infoGHD;
   SEGYTR240 *traceHead;

   INDEX_INFO *m_pIdx;

   int m_iIdxHD;
   bool m_bIdxFile;
   int m_iCurrentIdx; //,m_iCurrentGrp;
   int m_iCurGather; //,m_iCurrentGrp;

   int m_iBytes;
   int m_iAllTraces;
   int m_iAllGathers;
   int m_iWriteFormat;
   int m_iMaxTrsOfGather;

   int m_iReelCounter;
   int m_iLineCounter;
   int m_ic;// counter of read trace

//public:
   int openRead(int unit);// for pc not used now
   /**open read return
   function: openRead(file); >0:ok,all traces 
   -1:open error 
   -2:read 3200 error;
   -3:read 400 error; 
   -4:get file info error; 
    
   == 0 tape; not a disk file 
   >0 ok idxfile fine; 
   m_bIdxFile = false: no index file ; 
   */
   int openRead(QString filename); 
    /**
     * try openRead, 
     * if m_bIndexFile is false: 
     * return and  run a thread to createThIdxFile;
     *     if OK in slot closeRead,and openRead()
     */
   int openReadFile(QString filename);//for interact read;


   int openWrite(QString filename);
   /**
     return 0:ok,
     */

   int readTrace(int *head, float *buf);
   int readTraces(int *head, float *buf, int trs);// return 0 if ok
   int readGather(int *head, float *buf);//return trs if ok

   void setWriteFormat(int f);

   int write3200(unsigned char *buf);
   int write400(unsigned char *buf);

   int writeTrace(float *buf); //det head and write trace;
   int writeTrace(int *head, float *buf);
   int writeTraces(int *head, float *buf, int trs);
   /*
   0: OK
   -1: read error;
   */
   int closeRead();
   int closeWrite();
   // close;
   /// we know the gather header word ,find the index
   int getGatherIndex(int gather);
   /// we know index of gather , get the gather header word
   int getIndexGather(int idx);
   /// get header word value ,from1,consider byter order.
   int getHeaderWord(int hd,int *head);
   /// get header word value ,from a pointer.
   int getHeaderWord(int *head);
   /**
    * test the segy file gather, 
    * shot :3 
    * cmp: 6
    * line: 48,49
    */
   int testGather(QString n);

   void init();


   int setTraceHeader(); // fill data head to piHead,from main head message,
   int setTraceHeader(char *head); // fill data head to piHead,from main head message,
   int setMainHeader(); // fill main head message,from segy header
   int setMainHeader(MAINHD_INFO &mhd,unsigned char * hd);
 
   //index file functions:
   segyThread th;
   int createIdxFile(QString file, int hd);// start a thread to creat indexfile
   void createThIdxFile(QString file, int hd);// create ifile
public slots:
   void slotIFile();
signals:
   void sigThreadDown();        
public:
   int getIdxInfo();
   // when we use idx file, we can seek trs or gather directly
   //
   int seekTR(int tridx);
   int reSeek();
   int seekGather(int grp);
   int seekGatherIdx(int idx);
   int seekNextGather();
   int seekPreGather();
   int seekFirstGather();
   int seekLastGather();

   int write3200();
   int write400();
   int set3200(unsigned char *buf);
   int set3200();

   //
   int setFileInfo();
   int getFileInfo(); // get information from 400 to m_infoGHD
   int wetFileInfo(); //write information from m_infoGHD to 400
   int getBuf400(unsigned char *buf);
   SEGYTR240 buildHead(int *p);
   void buildHead(int *buf, SEGYTR240 th);
private:

//
   void short_to_float(short from[], float to[], int n, int endian); //format3
   void ibm_to_float(int from[], int to[], int n, int endian); //format 1
   void int_to_float(int from[], float to[], int n, int endian); //format 2
   void ieee_to_float(float from[], float to[], int n, int endian); //format 2



};

#endif // !defined(AFX_QSEGYADI_H__4DCB378F_03EF_4586_8EC3_3B07792B5D97__INCLUDED_)
/*           FILE LABEL (400 bytes)                                                                                                
*================================================                                                                                  
I  BYTE   I        DESCRIPTION                  I                                                                                  
*================================================                                                                                  
I  1-4    I Job number                          I                                                                                  
I  5-8    I Line number                         I                                                                                  
I  9-12   I Volume of tape                      I                                                                                  
I 13-14   I Number of trace per record          I                                                                                  
I 15-16   I Number of aid traces/record         I                                                                                  
I 17-20   I Sample interval (in microseconds)   I                                                                                  
I 21-22   I Number of samples per trace         I                                                                                  
I 23-24   I Field record Number of samples      I                                                                                  
I 25-26   I Data format 1==IBM float 32         I                                                                                  
I         I             2==Fixed32              I                                                                                  
I         I             3==Fixed16              I                                                                                  
I         I             4==IEEE 32              I                                                                                  
I 27-28   I CMP fold                            I                                                                                  
I 29-54   I 0                                   I                                                                                  
I 55-56   I 1  (Units meters )                  I                                                                                  
I 57-400  I 0                                   I                                                                                  
*================================================                                                                                  
                                                                                                                                 
                                                                                                                                 
                  TRACE  LABEL  (240 bytes)                                                                                      
*==================================================*                                                                               
I SEGY HD I          DESCRIPTION         I GRI HD  I                                                                               
I  byte   I                              I  word   I                                                                               
*==================================================*                                                                               
I 1-4     I Sequence number within line  I         I                                                                               
I 5-8     I Sequence number within tape  I         I                                                                               
I 9-12    I Shot point number            I HD56  =3  file                                                                               
I 13-16   I Trace number in the record   I HD17  =4  trace                                                                               
I 17-20   I Energy source point number   I HD2   =5  shot                                                                            
I 21-24   I CDP number                   I HD4   =6  cmp                                                                               
I 25-28   I Trace number within the CMP  I HD52  =7  fold                                                                               
I 29-30   I 1 (Trace identification code)I   1     I                                                                               
I 37-40   I Distance from source point toI         I                                                                               
I         I     receiver group           I HD20  =10 offset                                                                               
I 41-44   I Receiver group elevation     I HD39    I                                                                               
I 45-48   I Surface elevation at source  I HD38    I                                                                               
I 49-52   I Source depth below surface   I HD36    I                                                                               
I 53-56   I Datum elevation at receiver  I         I                                                                               
I         I                group (meters)I HD46    I                                                                               
I 61-64   I Water depth at source        I HD58    I                                                                               
I 65-68   I Water depth at group         I HD59    I                                                                               
I 69-70   I Integer data.A scaler        I   10    I                                                                               
I 73-76   I Source coordinate - X.       I HD71/10 =19I                                                                               
I 77-80   I Source coordinate - Y.       I HD72/10 =20I                                                                               
I 81-84   I Group coordinate - X.        I HD73/10 =21I                                                                               
I 85-88   I Group coordinate - Y.        I HD74/10 =22I                                                                               
I 89-90   I Units:1-length(meters)       I   1     I                                                                               
I 95-96   I Uphole time at source        I HD35    I                                                                               
I 99-100  I Source static correction     I HD41    I                                                                               
I 101-102 I Group  static correction     I HD42    I                                                                               
I 103-104 I Total static applied         I HD43    I                                                                               
I 109-110 I Delay recording time         I HD16    I                                                                               
I 115-116 I Number of samples            I HD10/HD9I                                                                               
I 117-118 I Samples interval             I         I                                                                               
I         I             (in microseconds)I HD9*1000I                                                                               
I 181-182 I Recording traces number      I HD124   I                                                                               
I 185-188 I Source remainder correction  I HD48    I                                                                               
I 189-192 I Group remainder correction   I HD47    I                                                                               
I 193-194 I Total static applied         I HD43    I                                                                               
I 195-196 I Recording point starte numberI HD18   =49 I                                                                               
I 197-198 I Line number of 3D            I HD68   = I                                                                               
I 201-204 I CMP coordinate - X.          I HD75/10 I                                                                               
I 205-208 I CMP coordinate - Y.          I HD76/10 I                                                                               
I 217-218 I Field file number.           I HD56    I                                                                               
I 219-220 I Shot point station number.   I HD21/4  I                                                                               
I 221-222 I CMP point elevation.         I HD40    I                                                                               
I 223-224 I CMP point station number.    I HD5/4   I                                                                               
I 239-240 I Flag of 1--3D; 0---2D.       I  1;0    I                                                                               
*==================================================*   
*/
