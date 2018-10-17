// VERSION 1.07 **************
/*
��������!!! ����� ��� ��� ������� ��������� ��������������� ��� �������
    ���������� ������ � ������� ����� �������:
        Build/Batch Build.../� ������ ����� �������� Win32Debuging
*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <dos.h>
#include <time.h>

#include "Python.h"
#include "PyWinTypes.h" //��� ���������� �� ������ pywin32

#include "db_error.h"
#define MEDIA 0.5 // ������� ��� �������� ������
#define TMP_NAME "@D$g&~^^.tmp"       // ��� ��� ���������� �����
#pragma hdrstop

int  db_create3( char* );   // ������� ����, �� ������� �������� (0-�����)
int  db_create1( void ); // �������� ������ �����
int  db_create2( char *db_name, char db_type, unsigned int lengt,
               unsigned int decimal); // �������� � ������ �����
int  db_open( char*);       // ������� ����, (������� ���) ������� 0-�����; �� ���� - ������
int  db_close( void );     // ������� ����,  0-�����
int  field_count(void); // �������� ���-�� �����
char*  get_num(const int field_num) ;// �������� �������� ���� � ���� ������� �� ��� ������
char*  get_name( char *name) ;// �������� �������� ���� � ���� ������� �� ��� �����
int  db_skip(long kol=1); // **** �������� �� ������� (��� �����- +1; n- ������ �� n; -n ����� �� n)
int  db_goto(long rec); // ��������������� �� ������ ������
int  db_eof(void); // ****** �������� �� ����� ����� (1-������� ����� �� �����;0-�����)
int  db_bof(void); // ****** �������� �� ������ ����� (1-������� ����� �� ������;0-�����)
int  db_delete(int flag=99); //*** ��������( ��� �����-��������; 1-���.��������; 0-�����)
int  set_delete(int flag=99);  // �����/���������/��������  ���������� ��������� �������
int  db_append(void); // �������� ������ (������) � ����� �����
int  put_num(char *value, int pole) ;// �������� �������� ���� � ���� ������� �� ��� ������
int  put_name(char *value, char *name) ;// �������� �������� ���� � ���� ������� �� ��� �����
int  db_error( void ); // �������� ������ (0- ��� ������) ��. ���� db_error.h
long  int db_lastrec( void ); // �������� ���-�� ������� � ����
int  find_field(char *name); // ����� ������ ���� �� ��� �����
int  find_sort(char *pole, char *isk_stroka) ; // ������� ??? ����� � ������������� ����
long  db_recno( void ) ; // �������� ������� ����� ������ (1...lastrec)
char  db_used( void ) ; // ��������� ���� �� �������� ����
int   db_zap( void ) ; // ������� ���� ;)
int   db_pack( void ) ; // �������� ���� �� ��������� �������
char*  db_getname( int num) ;
char   db_gettype( int num) ;
int    db_getlengt( int num) ;
int    db_getdecimal( int num) ;

//******************************************************************************************
struct date
{
    int     da_year;        /* Year - 1980 */
    char    da_day;     /* Day of the month */
    char    da_mon;     /* Month (1 = Jan) */
};

struct field {    // ��������� ���� ���������
          char name[11]; // ��� ����
          char type;                 // ��� ����
          unsigned int offset;       // �������� ���� ������������ ������ (�� ���������)
          unsigned int lengt;    // ����� ����
          unsigned int decimal;  // ������� �������. �����
          unsigned char flag;   // ��������� ����
          unsigned int sm;      // ����������� �������� ���� ������������ ������
         };
static unsigned char hdr_type; // ��� ����
static unsigned char hdr_date[3]; // ���� ��������� �����������
static unsigned long hdr_lastrec; // ���-�� ������� � ����
static unsigned int  hdr_begin;  // ������� ������ ������ ������
static unsigned int  hdr_lengt;  // ����� ������
static unsigned int  hdr_flag;   // ���� ���������
static unsigned int  hdr_code;   // ������ ��������������
static unsigned int  hdr_dlina=1; // ����������� ����� ������
static          int hdr_err='\x00'; // ��� ������

static struct field *table; // ������� �����
static FILE *hdr=NULL;     // ����� �����
static unsigned long recno=0;  // ������� ����� ������
static unsigned long recpos=0 ; // ������� � ����� ������� ������ (��������� �� ���� ��������)
static unsigned char flag_delete='\xff' ; // SET DELETE ON/OFF
static          char buffers[257]="\x00" ; // ������ ��� �������� �����
static          char *buf_rec=NULL ; // ������ ��� ������
static unsigned int fieldcount=0; // ���-�� �����
static unsigned char flag_eof='\x00',flag_bof='\x00'; // **** ���� EOF() BOF() ******
static          char file_name[256]; // ��� ����� ���� ������
static          char flag_ud; // ���� 0-���������� ������� ������ �� ���������
//******************************************************************************
//***  ��������� ���������
void skip_local(long kol);
int delete_local(void);

int db_fputc(int, FILE *);         //------------------------------------
int db_fgetc(FILE *);
int db_fseek(FILE *, long , int ); //-- ��������������� ��������� �������
long int db_ftell(FILE *);         //------------------------------------
char *padl(char *in,char *out, int lengt, char sym='\x20');
//******************************************************************************
/*
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
    return 1;
} */
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int  db_open(char *name)
{
   unsigned int i,filcount=0,ofst=1,tmp[15];

   if( hdr !=NULL ) db_close(); // ���� �������� ����, ������� ��

   hdr_dlina=1;
   recno=0;
   hdr_err=NOT_ERROR;

   if ((hdr = fopen(name, "r+b"))== NULL)
   {  table=NULL;
      hdr_err=NOT_OPEN;
      return 1;
   }
   for(i=0; (i<256) && (name[i] !='\x00'); i++) file_name[i]=name[i]; // ��������� ��� �����
   tmp[0]=db_fgetc(hdr);   
   hdr_type=char(tmp[0]); // ��� ����
   tmp[0]=db_fgetc(hdr);   
   tmp[1]=db_fgetc(hdr);   
   tmp[2]=db_fgetc(hdr); // ���� �����������
   hdr_date[0]=char(tmp[0]); 
   hdr_date[1]=char(tmp[1]) ; 
   hdr_date[2]=char(tmp[2]);
   tmp[0]=db_fgetc(hdr);   
   tmp[1]=db_fgetc(hdr);   
   tmp[2]=db_fgetc(hdr);   
   tmp[3]=db_fgetc(hdr);
   hdr_lastrec=tmp[0]+256*tmp[1]+(256*256)*tmp[2]+(256*256*256)*tmp[3]; // ����� ������� � ����
   tmp[0]=db_fgetc(hdr);
   tmp[1]=db_fgetc(hdr);
   hdr_begin=tmp[0]+256*tmp[1]; // ����� ������ ������ ������
   tmp[0]=db_fgetc(hdr);   
   tmp[1]=db_fgetc(hdr);
   hdr_lengt=tmp[0]+256*tmp[1]; // ����� ����� ������
   db_fseek(hdr,14,SEEK_CUR); //RESERVED
   tmp[0]=db_fgetc(hdr); 
   hdr_flag=tmp[0]; //���� �������
   tmp[0]=db_fgetc(hdr); 
   hdr_code=tmp[0]; // ������� �������� ��������
   db_fseek(hdr,4,SEEK_CUR); //RESERVED
//****************  ������ ������� ����� *****************************

   while(char(db_fgetc(hdr))!=0xd ) {   // ���� ��������� ���-�� �����
     db_fseek(hdr,31,SEEK_CUR);
     filcount++;
   } fieldcount=filcount;

   db_fseek(hdr,32,SEEK_SET); //  ������ ������� ����� *****************************
   table=new field [filcount];
   if(table == NULL) { hdr_err=NOT_MEMORY; return 1;}
      for(i=0; i<filcount ; i++) {

     table[i].name[0]=char(db_fgetc(hdr));
     table[i].name[1]=char(db_fgetc(hdr));
     table[i].name[2]=char(db_fgetc(hdr));
     table[i].name[3]=char(db_fgetc(hdr));
     table[i].name[4]=char(db_fgetc(hdr));
     table[i].name[5]=char(db_fgetc(hdr));
     table[i].name[6]=char(db_fgetc(hdr));
     table[i].name[7]=char(db_fgetc(hdr));
     table[i].name[8]=char(db_fgetc(hdr));
     table[i].name[9]=char(db_fgetc(hdr));
     table[i].name[10]=char(db_fgetc(hdr));

     table[i].type=char(db_fgetc(hdr));

     tmp[0]=db_fgetc(hdr);
     tmp[1]=db_fgetc(hdr);
     tmp[2]=db_fgetc(hdr);
     tmp[3]=db_fgetc(hdr);
     table[i].offset=tmp[0] ;
     table[i].offset+=256         * tmp[1];
     table[i].offset+=256*256     * tmp[2];
     table[i].offset+=256*256*256 * tmp[3];

     table[i].lengt=db_fgetc(hdr);
     table[i].decimal=db_fgetc(hdr);
     table[i].flag=char(db_fgetc(hdr));
     db_fseek(hdr,13,SEEK_CUR);  //RESERVED

     table[i].sm=ofst;        // ��������� �������� ���� ������������ ������
     ofst +=table[i].lengt ;
         hdr_dlina +=table[i].lengt; // ��������� ����� ������
      }
      //********** ������������� ������ � ���������� ****************
   buf_rec= (char*)malloc( hdr_dlina); // ������ ��� ������ ������
   if(buf_rec == NULL) { hdr_err=NOT_MEMORY; return 1;}

   recno=1;  db_fseek(hdr,hdr_begin,SEEK_SET); recpos=hdr_begin;
   flag_eof='\x00' ; flag_bof='\x00';
   flag_ud='\xff'; // ��������� ���������� �������
   return 0 ;
}
//*****************************************************************
static void getdate(struct date *_datep)
{
    time_t timer;
    struct tm *_cur_time_;

    /* �������� ����� ����� */
    timer = time(NULL);

    /* ��������� ���� � ����� � ��������� */
    _cur_time_=localtime(&timer);

    //�������� ������� ���� � �����
    _datep->da_year=_cur_time_->tm_year;
    _datep->da_mon=_cur_time_->tm_mon;
    _datep->da_day=_cur_time_->tm_mday;
}

//*****************************************************************
int  db_close( void ) { // �������� ����
     struct date d;

 hdr_err=NOT_ERROR;
 if(hdr != NULL ) {
   getdate(&d);                //
   d.da_year-=1900;
   db_fseek(hdr,1,SEEK_SET);
   db_fputc( d.da_year , hdr); //
   db_fputc( int(d.da_mon),hdr);
   db_fputc( int(d.da_day),hdr); // ���� �����������

   int buf;
   db_fseek(hdr, -1L , SEEK_END); // ���� ��� ������� EOF, �������� ���
   buf=db_fgetc(hdr);
   if(  buf!= int('\x1a') ) {
       db_fseek(hdr,0,SEEK_END);
       db_fputc( (int)('\x1a'), hdr);
   }
   delete table;
   free( buf_rec);
   fclose(hdr);
 } else    hdr_err=NOT_OPEN;
   hdr=NULL;
   fieldcount=0;
   recno=0;  // ������� ����� ������
   recpos=0 ; // ������� � ����� ������� ������ (��������� �� ���� ��������)
  return 0;
}
//*****************************************************************
int  field_count(void) // ������� ���-�� �����
{
   hdr_err=NOT_ERROR;
   if(hdr==NULL) {
      hdr_err=NOT_OPEN;
      return 0; // ���� �� ��������
   } else {
      return fieldcount; //������� ���-�� �����=������� �������
   }
}

//*****************************************************************
char*  get_num(const int field_num) // �������� �������� ���� � ���� ������� �� ��� ������
{
    unsigned int i,j;

    hdr_err=NOT_ERROR;
    if(hdr==NULL) { hdr_err=NOT_OPEN; return NULL;} // ���� �� �������
    if( (field_num < 0) || (field_num > field_count())  ) {
        hdr_err=NOT_SEEK;
        return NULL; // ��� ���������
    }
        if(flag_ud !='\x00'){ // ��������� ���������� �������
          fread(buf_rec,hdr_dlina,1,hdr); // ������ ���� ������ � ������
          flag_ud='\x00'; // ����� ����� ����������
        }
//--------------------
//        db_fseek(hdr, recpos+table[field_num].sm, SEEK_SET); // ������� ������ ���� � ������
/*    for(i=0; i <table[field_num].lengt ;i++ ) {
         buffers[i]=char(db_fgetc(hdr));
      } buffers[i]='\x00'; */
//          fread(buffers,table[field_num].lengt,1,hdr); // ������ ���� � ������
//          buffers[table[field_num].lengt+1]='\x00';
//-----------  ����� ������� ���� �� ������� -----------
          j=0;
      for(i=table[field_num].sm;
                       i <(table[field_num].sm+table[field_num].lengt) ;
                       i++ ) {
         buffers[j++]=buf_rec[i];
      }
          buffers[j]='\x00';

    return &buffers[0];
}

//**************************************************************************
int  db_skip(long kol) { // **** �������� �� �������
    hdr_err=NOT_ERROR;
    if(hdr==NULL) {hdr_err=NOT_OPEN; return 1; }// ���� �� �������
    skip_local(kol);
    if(flag_delete !=0 ) { // ���� ����������� �� ��������� �������
        while( (db_delete() !=0)&&(db_eof()==0)&&(db_bof()==0) ) {
           if(kol>0) skip_local(1); else skip_local(-1);
        }
    }
        flag_ud='\xff'; // ��������� ���������� �������
    return 0;
}
void skip_local(long kol) {
  long sm;

  flag_bof='\x00'; flag_bof='\x00';
  recno +=kol;
  if(recno <=0) {
     flag_bof='\xff';
     recno=1;
  }
  if(recno >hdr_lastrec) {
     flag_eof='\xff';
     recno=hdr_lastrec;
  }
  recpos=hdr_begin+hdr_lengt*(recno-1);
  sm=recpos-db_ftell(hdr);
  db_fseek(hdr,sm ,SEEK_CUR);
}

//**************************************************************************
int  db_eof(void) {
  hdr_err=NOT_ERROR;
  if(hdr==NULL) { hdr_err=NOT_OPEN; return 1;} // ���� �� �������
  return flag_eof;
}
int  db_bof(void) {
   hdr_err=NOT_ERROR;
   if(hdr==NULL) {   hdr_err=NOT_OPEN; return 1;} // ���� �� �������
   return flag_bof;
}

//**************************************************************************
int  db_delete( int flag ) { //******** flag = 0 ����� ��������
                      // ********      = 1 ���.  ��������
              //  ******** ��� �����.  �������� ��������
    hdr_err=NOT_ERROR;
    if( flag==99 ) return delete_local(); // ������ ��������� �� ��������
    if( flag== 0 ) { // ����� ������ ��������
       db_fputc('\x20',hdr);
       db_fseek(hdr,-1,SEEK_CUR);
       return 0;
    }
    if( flag> 0 ) { // ��������� ������ ��������
       db_fputc('\x2a',hdr);
       db_fseek(hdr,-1,SEEK_CUR);
       return 1;
    }
    return 0;
}
int delete_local( void ) { //====== �������� �� �������� ===========
    char del;
    del=char(db_fgetc(hdr));
    db_fseek(hdr, -1 , SEEK_CUR);
    if(del =='\x2a') return 1;
    else return 0;
}

//*******************************************************************
int  set_delete(int flag) {
    hdr_err=NOT_ERROR;
    if(flag == 99) { // ������ ��������� SET DELETE
    if(flag_delete !='\x00') return 0; else return 1;
    }
    // ���������� SET DELETE ON/OFF
    if( flag =='\x00' ) {
       flag_delete='\x00';
       return 0;
    } else {
       flag_delete='\xff';
       return 1;
    }
}

//******************************************************************************
int  db_append(void){ // �������� ������
     int unsigned i;
     void *pointer;

   hdr_err=NOT_ERROR;
   if(hdr==NULL) {
      hdr_err=NOT_OPEN;
      return 1;
   }

   db_fseek(hdr,4,SEEK_SET); // LASTREC()
   pointer= &(++hdr_lastrec);
   db_fputc( int(*(char *)pointer)       ,hdr);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer)       ,hdr);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer)       ,hdr);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer)       ,hdr); // LASTREC()****************

    recno=hdr_lastrec;

     db_fseek(hdr,0,SEEK_END);  // ���������� ����������
     recpos=db_ftell(hdr);
     db_fputc('\x20',hdr); // ������ ��������
     for(i=1; hdr_dlina > i; i++)     db_fputc('\x20',hdr);
     db_fseek(hdr,recpos,SEEK_SET);
         flag_ud='\xff'; // ��������� ���������� �������
     return 0;
}

//***************************************************************************
//*******  ������ � ���� ������� ������ �� ������ ���� **********************
int  put_num(char * value, int pole) {// ���������� �������� � ���� �� ��� ������
   unsigned int i;
   char *val;

   hdr_err=NOT_ERROR;
   if(hdr==NULL) {
      hdr_err=NOT_OPEN;
      return 1;
   }
   val=new char[table[pole].lengt+1]; //������������ ������� ��� ������
   padl( value, val,table[pole].lengt);

   db_fseek(hdr, recpos+table[pole].sm, SEEK_SET);
   for(i=0; (i<table[pole].lengt)&&(val[i]!='\x00'); i++)
       db_fputc(val[i],hdr);

   db_fseek(hdr, recpos, SEEK_SET);
   delete val;
   return 0;
}

//*--------------**********---------------***********--------------
//*  ***  ��������������� ��������� �������   ********************
int db_fputc(int a, FILE *b){
   int ret=fputc(a,b);
   if(ret==EOF) hdr_err=NOT_PUT;
   return ret;
}

int db_fgetc(FILE *a){
   int ret=fgetc(a);
      if(ret==EOF) hdr_err=NOT_GET;
   return ret;
}

int db_fseek(FILE *a, long b, int c){
   int ret=fseek(a,b,c);
      if(ret != 0)  hdr_err=NOT_SEEK;
   return ret;
}

long int db_ftell(FILE *a){
   long int ret=ftell(a);
      if(ret <0) hdr_err=NOT_TELL;
   return ftell(a);
}

// ************ ������� ��� ������ **********************************
int  db_error( void ) {
   return int(hdr_err);
}

//*-*-*-*-*-*----------------------------------------------------------------
long  int db_lastrec( void ) { // �������� ���-�� ������� � ����
    hdr_err=NOT_ERROR;
    return hdr_lastrec;
}

//---****************************************-------------------***************
int  find_field(char *name) { // ����� ������ ���� �� ��� �����
    int ret=-1;
    int unsigned i;

    hdr_err=NOT_ERROR;
    for(i=0 ; i< fieldcount ; i++ ) {
//��������� � ������ �������� !!!!!!!!!!!!!!!!!!!!!!!!!
       if(strcmp( (table[i].name) , (name)) ==0  ) {
           ret=i;
           i=fieldcount+1; // ��������� ����� �� �����
       }
    }
    if(ret <0) hdr_err=NOT_FIELD;
    return ret;
}

//*****************************************************************************
char*  get_name( char *name) {// �������� �������� ���� � ���� ������� �� ��� �����
   int pole;

   hdr_err=NOT_ERROR;
   pole=find_field(name) ;
   if(pole < 0) return NULL;
   return get_num(pole);
}
//******************************************************************************
int  put_name(char *value, char *name) {// �������� �������� ���� � ���� ������� �� ��� �����
   int pole;

   hdr_err=NOT_ERROR;
   pole=find_field(name) ;
   if(pole < 0) return NULL;
   return put_num( value , pole);
}
//*********************************************************************************
int  db_create1( void ) { // �������� ������ �����
        hdr_err=NOT_ERROR;
        db_close();
        //table=new( field[255]);
                table=(field*)calloc(255,sizeof(field));
        if(table == NULL) {hdr_err=NOT_MEMORY; return 1;}
        return 0;
}
int  db_create2( char *db_name, char db_type, unsigned int lengt,
               unsigned int decimal) { // �������� � ������ �����
      unsigned int i,kolvo  ;

      hdr_err=NOT_ERROR;
      kolvo=fieldcount;
      if(table == NULL) {hdr_err=NOT_ALLOC; return 1;}

//    db_name=strupr(db_name);
      for(i=0; i<sizeof(table[0].name) ;i++) table[kolvo].name[i]='\x00';
      for(i=0;db_name[i] !='\x00';i++) table[kolvo].name[i]=db_name[i];


      if( (db_type =='C') ||
          (db_type =='N') ||
          (db_type =='F') ||
          (db_type =='D')
        )    table[kolvo].type=db_type;
      else {
          hdr_err=NOT_TYPE;
          return 1;
      }
      if( db_type =='D') { lengt=8; decimal=0; }
      if( db_type =='C') {          decimal=0; }
      if((db_type =='F') &&(lengt<=(decimal+1))) {
          hdr_err=NOT_TYPE;
          return 1;
      }
      if( (lengt >0 ) && (lengt<=255))  table[kolvo].lengt=lengt;
      else {
          hdr_err=NOT_TYPE;
          return 1;
      }
      if(lengt>=(decimal+1)) table[kolvo].decimal=decimal;
      else {
          hdr_err=NOT_TYPE;
          return 1;
      }
//  ����  ����������, ���� ������ ��������� ����������
      table[kolvo].offset=1; // ������ ��������
      for(i=1; i<= kolvo; i++) {
         table[kolvo].offset +=table[i-1].lengt;
      }
      table[kolvo].sm=table[kolvo].offset;
      fieldcount =kolvo+1;     
      return 0;
}
//******************************************************************
//******************************************************************
int  db_create3(char* name)
{
   FILE  *out;
   int unsigned i,buf,begin,lengt;
   void *pointer;
   struct date d;

   hdr_err=NOT_ERROR;
   if ((out = fopen(name, "wb"))== NULL) { //
      hdr_err=NOT_OPEN;
      return 1;
   }

   db_fputc( '\x03'      ,out); // ��� ����-1

   getdate(&d);                //
   if(d.da_year<=1999) d.da_year-=1900; else d.da_year-=1900;
   db_fputc( d.da_year , out); //
   db_fputc( int(d.da_mon),out);
   db_fputc( int(d.da_day),out); // ���� �����������

   db_fputc( '\x00',out);  db_fputc( '\x00',out);
   db_fputc( '\x00',out);  db_fputc( '\x00',out); // ���-�� �������

   // ��������� ��� ������� ����� ���������
   begin=32; // 31-- ����� ��������� �� ������� �����
   lengt=1; // 1- ���� ��������
   for(i=0; i< fieldcount; i++ ) {
      begin +=32;  // 32-����� ������ ������� �����
      lengt +=table[i].lengt;
   }
//   begin +=264; // 264-����� ��������� ����� ������� �����
   begin +=1; // ��� �����������
   pointer =&begin  ;
   db_fputc( int(*(char *)pointer)       ,out);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer)       ,out); // ������� ������ �������
    // ����. ���� + ��.����

   pointer =&lengt  ;
   db_fputc( int(*(char *)pointer)       ,out);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer)       ,out); // ����� ������

   for(i=0; i< 16; i++) db_fputc( '\x00', out); // RESERVED
   db_fputc( int('\x00'),  out); // ���� ��������� ????
   db_fputc( int('\xc9'),  out); // ������� �������  Rusian Windows
   db_fputc( int('\x00'),  out);db_fputc( int('\x00'),  out); // RESERVED

   for(i=0; i< fieldcount; i++ ) {
   db_fputc( int(table[i].name[0]),  out);
   db_fputc( int(table[i].name[1]),  out);
   db_fputc( int(table[i].name[2]),  out);
   db_fputc( int(table[i].name[3]),  out);
   db_fputc( int(table[i].name[4]),  out);
   db_fputc( int(table[i].name[5]),  out);
   db_fputc( int(table[i].name[6]),  out);
   db_fputc( int(table[i].name[7]),  out);
   db_fputc( int(table[i].name[8]),  out);
   db_fputc( int(table[i].name[9]),  out);
   db_fputc( int(table[i].name[10]),  out);    // ��� ����

   db_fputc( int(table[i].type),  out); // ��� ����

   pointer= &(table[i].offset);
   db_fputc( int(*(char *)pointer)       ,out);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer)       ,out);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer)       ,out);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer)       ,out); // �������� ����

   db_fputc( int(table[i].lengt),    out); // ����� ����
   db_fputc( int(table[i].decimal),  out); // ������� �������. �����

   db_fputc( '\x00',out); // ���� ���� ???????
   for(buf=0; buf<13; buf++) db_fputc( '\x00', out); // RESERVED
   }
   db_fputc('\x0d',out); // ���������� ������� �����
//   for(i=0; i<263; i++) db_fputc('\x00', out); // RESERVSD
fclose(out);
db_close();
return 0;
}
//******************************************************************************
int  db_goto(long rec) { // ��������������� �� ������ ������

    hdr_err=NOT_ERROR;
    flag_bof='\x00'; flag_eof='\x00';

    if(hdr==NULL) {hdr_err=NOT_OPEN; return 1;} // ���� �� �������
    if(rec <1 ) { db_goto(1); flag_bof='\xff'; flag_eof='\x00'; return 0;}
    if(rec >long(hdr_lastrec)) {
         db_goto(hdr_lastrec);
         flag_bof='\x00'; flag_eof='\xff';
         return 0;
    }
    recno=rec;
    recpos=hdr_begin+hdr_lengt*(recno-1);
    db_fseek(hdr,recpos ,SEEK_SET);

    if(flag_delete !=0) {
        while( (db_delete() !=0)&&(db_eof()==0) )   db_skip();// ������ ���� ����. ����������� ������
    }
        flag_ud='\xff'; // ��������� ���������� �������
    return 0;
}
//**********************************************************************************
int  find_sort(char *pole, char *isk_stroka) { // ������� ??? ����� � ������������� ����
    long  unsigned pos,pos_begin,pos_end,sm;
    char *buf, *isk;
    int numpole,tmp,ret=0;
    char find_flag='\x00';

    numpole=find_field(pole);
    if(numpole<0) return 0; // ��� ������ ����

    isk=new char[table[numpole].lengt+1];
    padl(isk_stroka , isk , table[numpole].lengt);
    pos_begin=recno; pos_end=hdr_lastrec;
    while( find_flag ==0 ) { // �� ������ "�������!" � ������ ����� :)
        if( pos_begin==pos_end ) { // ������������ ���
          find_flag='\xff';        // ���� �� �����
        }
        sm=(pos_end-pos_begin) * MEDIA;
        pos=pos_begin +sm ;
        db_goto(pos);
        buf=get_num(numpole);
        tmp= strcmp( buf , isk );
        if( tmp > 0 ) { pos_end  =pos-1 ;}
        if( tmp < 0 ) { pos_begin=pos+1 ;}
        if( tmp ==0 ) { pos_end  =pos ;}
        if( (pos_end<0) || (pos_begin>hdr_lastrec)) {
          find_flag='\xff';
          ret=0;
        }
        if( (find_flag !=0) && (tmp==0) ) { // ������������ ��� � ���-�� �����
             ret=1;
        }
    }
    delete isk;

    db_goto(pos_end);

    return ret;
}
//*****************************************************************************
// ����������� � ����� ������(����� �������) � ���������� �����

char *padl(char *in,char *out, int lengt, char sym){
     int i,j;

     for(i=strlen(in)-1,j=lengt-1; (i>=0)&&(j>=0) ; i--,j--)
          out[j]=in[i];
     for( ;j>=0 ;j-- )
           out[j]=sym; //���������� ��������� �����
     out[lengt]='\x00';
     return out;
}
//***************************************************************************************
//******  ������ �������  CLIPPER RECNO()
long  db_recno( void ) { // �������� ������� ����� ������ (1...lastrec)
    return recno;
}
//****************************************************************************
char  db_used( void ) { // ��������� ���� �� �������� ����
     if(hdr == NULL) return '\x00';
     else            return '\xff';
}
//********************************************************************************************
char*  db_getname( int num) {
     if( db_used == 0) {hdr_err=NOT_OPEN; return NULL;}
     if( (num<0) || (num>=int(fieldcount))) {hdr_err=NOT_FIELD; return NULL;}

     return table[num].name;
} //-----------------------------------------------------------------------------
char  db_gettype( int num) {
     if( db_used == 0) {hdr_err=NOT_OPEN; return NULL;}
     if( (num<0) || (num>=int(fieldcount))) {hdr_err=NOT_FIELD; return NULL;}

     return table[num].type;
} //-----------------------------------------------------------------------------
int  db_getlengt( int num) {
     if( db_used == 0) {hdr_err=NOT_OPEN; return NULL;}
     if( (num<0) || (num>=int(fieldcount))) {hdr_err=NOT_FIELD; return NULL;}

     return table[num].lengt;
} //-----------------------------------------------------------------------------
int  db_getdecimal( int num) {
     if( db_used == 0) {hdr_err=NOT_OPEN; return NULL;}
     if( (num<0) || (num>=int(fieldcount))) {hdr_err=NOT_FIELD; return NULL;}

     return table[num].decimal;
} //*************************************************************************************
//---- ������ ��"������� ������� ��� VISUAL BASIC -------------------------------
int  vb_get_num(char* buf, int field_num) {// �������� �������� ���� � ���� ������� �� ��� ������
    int i=0;
    char *tmp;

    tmp=get_num(field_num);
    if( tmp !=NULL)   //  ������������ ���������� � ���������� VBA
       for(i=0; tmp[i]!='\x00' ; i++) buf[i]=tmp[i];
     buf[i]='\x00';
     return 1;
} //-----------------------------------------------------------------------------
int  vb_get_name( char *buf, char *name) {// �������� �������� ���� � ���� ������� �� ��� �����
    int i=0;
    char *tmp;

    tmp=get_name(name);
    if( tmp !=NULL)   //  ������������ ���������� � ���������� VBA
       for(i=0; tmp[i]!='\x00' ; i++) buf[i]=tmp[i];
     buf[i]='\x00';
     return 1;
}
//...........................................................................
int  vb_db_getname(char *buf, int num) {
     char *tmp;
     int i=0;

     tmp=db_getname(num);
    if( tmp !=NULL)   //  ������������ ���������� � ���������� VBA
       for(i=0; tmp[i]!='\x00' ; i++) buf[i]=tmp[i];
     buf[i]='\x00';
     return 1;
}
//********************************************************************************
//******* �������� ���� ������� ����
int  db_zap( void) {
   FILE  *out;
   unsigned long i;
   struct date d;

   hdr_err=NOT_ERROR;
   if( hdr==NULL) {
       hdr_err=NOT_OPEN;
       return 1;
   }
   if ((out = fopen(TMP_NAME, "wb"))== NULL) { //
      hdr_err=NOT_OPEN;
      return 1;
   }

   db_fseek(hdr,0,SEEK_SET);
   for(i=0; i < hdr_begin; i++)     db_fputc( db_fgetc(hdr), out); // ���������� ����� ���������

   db_fseek(out,1, SEEK_SET);
   getdate(&d);                // ------------------
   d.da_year-=1900;
   db_fputc( d.da_year , out); //
   db_fputc( int(d.da_mon),out);
   db_fputc( int(d.da_day),out); // ���� �����������

   db_fputc('\x00',out); // ����� ������� � ���� =0
   db_fputc('\x00',out);
   db_fputc('\x00',out);
   db_fputc('\x00',out); //------------------------

   fclose(out);
   db_close();

   if( db_error() ==0 ) {
    if( remove(file_name) !=0 ) {
       hdr_err=NOT_DEL;
       return 1;
    }
    if( rename( TMP_NAME, file_name) !=0 ) {
       hdr_err=NOT_RENAME;
       return 1;
    }
   }
   db_open(file_name);
        flag_ud='\xff'; // ��������� ���������� �������
    return 0;
}
//********************************************************************************
//******* �������� ������� ����
int  db_pack( void) {
   FILE  *out;
   unsigned long i,j,count=0;
   char buf;
   void *pointer;

   hdr_err=NOT_ERROR;
   if( hdr==NULL) {
       hdr_err=NOT_OPEN;
       return 1;
   }
   if ((out = fopen(TMP_NAME, "wb"))== NULL) { //
      hdr_err=NOT_OPEN;
      return 1;
   }

   db_fseek(hdr,0,SEEK_SET);
   for(i=0; i < hdr_begin; i++)     db_fputc( db_fgetc(hdr), out); // ���������� ����� ���������

   db_fseek(out, hdr_begin, SEEK_SET); // ���������� ������� �������
   for(j=0; j<hdr_lastrec; j++) {
    buf=char(db_fgetc(hdr));
    if( buf !='\x2a' ) {
       db_fputc(buf, out);
       for(i=1; i<hdr_dlina; i++) {
       buf=char(db_fgetc(hdr));
       db_fputc(buf, out);
       }
       count++;
    } else {
       db_fseek(hdr,hdr_dlina-1,SEEK_CUR);  // ������ �� ���������, ������� ��
    }
   }
   db_fseek(out, 4, SEEK_SET); // ������������� ����� ������� � ���������
   pointer= &(count);
   db_fputc( int(*(char *)pointer) ,out);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer) ,out);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer) ,out);  pointer=(char *)pointer +1;
   db_fputc( int(*(char *)pointer) ,out); // ����� ������� � ����

   fclose(out);
   db_close();
   if( db_error() ==0 ) {
     if( remove(file_name) !=0 ) {
       hdr_err=NOT_DEL;
       return 1;
     }
     if( rename( TMP_NAME, file_name) !=0 ) {
       hdr_err=NOT_RENAME;
       return 1;
     }
   }

   db_open(file_name);
        flag_ud='\xff'; // ��������� ���������� �������
    return 0;
}

//--- ��� ����� ����� ���������� � ������� ---
//������� DBF ����
static PyObject *ClearFields(PyObject* self,PyObject *args)
//�������� ������ �����.
{
    //��������� c-�������
    int c_result;


    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_create1();

    //���������� ���������
    return PyBool_FromLong((long)c_result);
}

static PyObject *AddField(PyObject* self,PyObject *args)
//�������� � ������ �����.
{
    //��������� c-�������
    char *db_name;  //��� ����
    char db_type;   //��� ����
    unsigned int length; //����� 
    unsigned int decimal; //�������� ���������� �����
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"scii",&db_name,&db_type,&length,&decimal))
        return NULL; //���������� ������

    c_result=db_create2(db_name,db_type,length,decimal);
    
    //���������� ���������
    return PyBool_FromLong((long)c_result);
}

static PyObject *Create(PyObject* self,PyObject *args)
//������� ������� ���� dbf �� �������� �����.
{
    //��������� c-�������
    char *dbf_name;  //����
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"s",&dbf_name))
        return NULL; //���������� ������
    
    c_result=db_create3(dbf_name);
    
    //���������� ���������
    return PyBool_FromLong((long)c_result);
}

static PyObject *Open(PyObject* self,PyObject *args)
//������� ���� dbf.
{
    //��������� c-�������
    char *dbf_name;  //���� DBF
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"s",&dbf_name))
        return NULL; //���������� ������
    
    c_result=db_open(dbf_name);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *Close(PyObject* self,PyObject *args)
//������� ���� dbf.
{
    //��������� c-�������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_close();
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *FieldCount(PyObject* self,PyObject *args)
//���������� �����.
{
    //��������� c-�������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=field_count();
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *GetFieldByNum(PyObject* self,PyObject *args)
//�������� ���� �� ������.
{
    //��������� c-�������
    int field_num; //������/����� ����
    char *c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"i", &field_num))
        return NULL; //���������� ������
    
    c_result=get_num(field_num);
    
    //���������� ���������
    return PyString_FromString(c_result);
}

static PyObject *GetFieldByName(PyObject* self,PyObject *args)
//�������� ���� �� ��� �����.
{
    //��������� c-�������
    char *field_name; //��� ����
    char *c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"s", &field_name))
        return NULL; //���������� ������
    
    c_result=get_name(field_name);
    
    //���������� ���������
    return PyString_FromString(c_result);
}

static PyObject *Skip(PyObject* self,PyObject *args)
//�������� �� �������.
{
    //��������� c-�������
    long step; //���
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"l", &step))
        return NULL; //���������� ������
    
    c_result=db_skip(step);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *Goto(PyObject* self,PyObject *args)
//������� �� ������.
{
    //��������� c-�������
    long rec; //����� ������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"l", &rec))
        return NULL; //���������� ������
    
    c_result=db_goto(rec);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *Eof(PyObject* self,PyObject *args)
//�������� �� ����� ����� (1-������� ����� �� �����;0-�����)
{
    //��������� c-�������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_eof();
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *Bof(PyObject* self,PyObject *args)
//�������� �� ������ ����� (1-������� ����� �� ������;0-�����)
{
    //��������� c-�������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_bof();
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *Delete(PyObject* self,PyObject *args)
//��������( ��� �����-��������; 1-���.��������; 0-�����)
{
    //��������� c-�������
    int flag; //���� ���������� ���������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"i", &flag))
        return NULL; //���������� ������
    
    c_result=db_delete(flag);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *SetDelete(PyObject* self,PyObject *args)
//�����/���������/��������  ���������� ��������� �������
{
    //��������� c-�������
    int flag; //���� ���������� ���������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"i", &flag))
        return NULL; //���������� ������
    
    c_result=set_delete(flag);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *Append(PyObject* self,PyObject *args)
// �������� ������ (������) � ����� �����
{
    //��������� c-�������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_append();
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *SetFieldByNum(PyObject* self,PyObject *args)
// �������� �������� ���� � ���� ������� �� ��� ������
//������� ������ ����� ��������
{
    //��������� c-�������
    int field_num;  //�����/������ ����
    char *field_val; //�������� ����
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"is", &field_num, &field_val))
        return NULL; //���������� ������
    
    c_result=put_num(field_val, field_num);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *SetFieldByName(PyObject* self,PyObject *args)
// �������� �������� ���� � ���� ������� �� ��� �����
//������� ��� ����� ��������
{
    //��������� c-�������
    char *field_name;  //��� ����
    char *field_val; //�������� ����
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"ss", &field_name, &field_val))
        return NULL; //���������� ������
    
    c_result=put_name(field_val, field_name);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *GetErrorCode(PyObject* self,PyObject *args)
// �������� ������ (0- ��� ������) ��. ���� db_error.h
{
    //��������� c-�������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_error();
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *GetRecCount(PyObject* self,PyObject *args)
// �������� ���-�� ������� � ����
{
    //��������� c-�������
    long int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_error();
    
    //���������� ���������
    return PyLong_FromLong(c_result);
}

static PyObject *GetFieldNum(PyObject* self,PyObject *args)
// ����� ������ ���� �� ��� �����
{
    //��������� c-�������
    char *field_name; //��� ����
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"s", &field_name))
        return NULL; //���������� ������
    
    c_result=find_field(field_name);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *FindSort(PyObject* self,PyObject *args)
// ������� ??? ����� � ������������� ����
{
    //��������� c-�������
    char *field_name; //��� ����
    char *find_txt; //������� ������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"ss", &field_name, &find_txt))
        return NULL; //���������� ������
    
    c_result=find_sort(field_name,find_txt);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *GetRecNo(PyObject* self,PyObject *args)
// �������� ������� ����� ������ (1...reccount)
{
    //��������� c-�������
    long c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_recno();
    
    //���������� ���������
    return PyLong_FromLong(c_result);
}

static PyObject *IsUsed(PyObject* self,PyObject *args)
// ��������� ���� �� �������� ����
{
    //��������� c-�������
    char c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_used();
    
    //���������� ���������
    return PyBool_FromLong((long)c_result);
}

static PyObject *Zap(PyObject* self,PyObject *args)
// ������� ���� ;)
{
    //��������� c-�������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_zap();
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *Pack(PyObject* self,PyObject *args)
// �������� ���� �� ��������� �������
{
    //��������� c-�������
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,""))
        return NULL; //���������� ������
    
    c_result=db_pack();
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *GetFieldName(PyObject* self,PyObject *args)
//��� ���� �� ������/�������.
{
    //��������� c-�������
    int field_num; //�����/������ ����
    char *c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"i", &field_num))
        return NULL; //���������� ������
    
    c_result=db_getname(field_num);
    
    //���������� ���������
    return PyString_FromString(c_result);
}

static PyObject *GetFieldType(PyObject* self,PyObject *args)
//��� ���� �� ������/�������.
{
    //��������� c-�������
    int field_num; //�����/������ ����
    char c_result; //��������� �-�������
	char py_result[2]; 

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"i", &field_num))
        return NULL; //���������� ������
    
    c_result=db_gettype(field_num);
    
    //���������� ���������
    py_result[0]=c_result;
	py_result[1]='\x00';
	return PyString_FromString(py_result);
}

static PyObject *GetFieldLength(PyObject* self,PyObject *args)
//����� ���� �� ������/�������.
{
    //��������� c-�������
    int field_num; //�����/������ ����
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"i", &field_num))
        return NULL; //���������� ������
    
    c_result=db_getlengt(field_num);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

static PyObject *GetFieldDecimal(PyObject* self,PyObject *args)
//�������� ���������� ����� ���� �� ������/�������.
{
    //��������� c-�������
    int field_num; //�����/������ ����
    int c_result; //��������� �-�������

    //������� ���������� � �-�������������
    if (!PyArg_ParseTuple(args,"i", &field_num))
        return NULL; //���������� ������
    
    c_result=db_getdecimal(field_num);
    
    //���������� ���������
    return PyInt_FromLong((long)c_result);
}

//������ � ��������� ������/������� �����������
static struct PyMethodDef pyDBF_methods[]=
{
    {"ClearFields",ClearFields,1},
    {"AddField",AddField,1},
    {"Create",Create,1},
    {"Open",Open,1},
    {"Close",Close,1},
    {"FieldCount",FieldCount,1},
    {"GetFieldByNum",GetFieldByNum,1},
    {"GetFieldByName",GetFieldByName,1},
    {"Skip",Skip,1},
    {"Goto",Goto,1},
    {"Eof",Eof,1},
    {"Bof",Bof,1},
    {"Delete",Delete,1},
    {"SetDelete",SetDelete,1},
    {"Append",Append,1},
    {"SetFieldByNum",SetFieldByNum,1},
    {"SetFieldByName",SetFieldByName,1},
    {"GetErrorCode",GetErrorCode,1},
    {"GetRecCount",GetRecCount,1},
    {"GetFieldNum",GetFieldNum,1},
    {"FindSort",FindSort,1},
    {"GetRecNo",GetRecNo,1},
    {"IsUsed",IsUsed,1},
    {"Zap",Zap,1},
    {"Pack",Pack,1},
    {"GetFieldName",GetFieldName,1},
    {"GetFieldType",GetFieldType,1},
    {"GetFieldLength",GetFieldLength,1},
    {"GetFieldDecimal",GetFieldDecimal,1},
    {NULL,NULL} //�������� ���������
};

//����� �������: ���������� ���������������� ������!!!
extern "C" __declspec(dllexport) void initpyDBF(void)
{
  // Initialize PyWin32 globals (such as error objects etc)
  PyWinGlobals_Ensure();

  PyObject *module;
  module = Py_InitModule("pyDBF",pyDBF_methods);
  if (!module)
    return;
}


