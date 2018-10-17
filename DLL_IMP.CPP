//---------------------------------------------------------------------------
#include <vcl\condefs.h>
#include <vcl\system.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <Sysutils.h>


#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused
int main(int argc, char* argv[])
{
int i,j,k;
 

	HINSTANCE  hdr_dll; // ****** указатель на загруженную библиотеку
// ********** Об"явление указателей на функции **************************
int     (*db_create3)   ( char* );
int     (*db_create1)   ( void );
int     (*db_create2)   ( char *db_name, char db_type, unsigned int lengt,unsigned int decimal);
int     (*db_open)     ( char*);
int     (*db_close)    ( void );
int     (*field_count) (void);
char*   (*get_num)   (const int field_num) ;
char*   (*get_name)   ( char *name) ;
int     (*db_skip)     (long kol=1);
int     (*db_goto)     (long rec);
int     (*db_eof)      (void);
int     (*db_bof)      (void);
int     (*db_delete)   (int flag=99);
int     (*set_delete)  (int flag=99);
int     (*db_append)   (void);
int     (*put_num)   (char *value, int pole) ;
int     (*put_name)   (char *value, char *name) ;
int     (*db_error)    ( void );
long int(*db_lastrec)  ( void );
int     (*find_field)  (char *name);
int     (*find_sort)   (char *pole, char *isk_stroka) ;
long    (*db_recno)    ( void ) ;
char    (*db_used)     ( void ) ;
char*   (*db_getname)  ( int  ) ;
char    (*db_gettype)  ( int  ) ;
int     (*db_getlengt) ( int  ) ;
int     (*db_getdecimal)( int  ) ;
int     (*db_zap)     ( void ) ;
int     (*db_pack)    ( void ) ;
char proc_flag='\x00';

hdr_dll=LoadLibrary("dll_dbf.dll"); if(hdr_dll==NULL) proc_flag='\xff';

//********* Привоение указателям адресов функциям ****************************************************************
//***  Где hdr_dll - хэдер на загруженную dll_dbf.dll ************************************************************
//***
//****************************************************************************************************************
db_create3= (int(*)(char*)   )GetProcAddress( hdr_dll, "_db_create3"); if(db_create3==NULL) proc_flag='\xff';
db_create1= (int(*)(void)    )GetProcAddress( hdr_dll, "_db_create1"); if(db_create1==NULL) proc_flag='\xff';
db_create2= (int(*)( char *db_name, char db_type, unsigned int lengt,unsigned int decimal))GetProcAddress( hdr_dll, "_db_create2"); if(db_create2==NULL) proc_flag='\xff';
db_open  =  (int(*)( char*)   )GetProcAddress( hdr_dll, "_db_open"); if(db_open==NULL) proc_flag='\xff';
db_close =  (int(*)( void )   )GetProcAddress( hdr_dll, "_db_close"); if(db_close==NULL) proc_flag='\xff';
field_count=(int(*) (void)   )GetProcAddress( hdr_dll, "_field_count"); if(field_count==NULL) proc_flag='\xff';
get_num =   (char*(*)(const int field_num))GetProcAddress( hdr_dll, "_get_num"); if(get_num==NULL) proc_flag='\xff';
get_name=   (char*(*)( char *name))GetProcAddress( hdr_dll, "_get_name"); if(get_name==NULL) proc_flag='\xff';
db_skip  =  (int(*)(long kol=1))GetProcAddress( hdr_dll, "_db_skip"); if(db_skip==NULL) proc_flag='\xff';
db_goto  =  (int(*)(long rec)  )GetProcAddress( hdr_dll, "_db_goto"); if(db_goto==NULL) proc_flag='\xff';
db_eof   =  (int(*)(void)      )GetProcAddress( hdr_dll, "_db_eof"); if(db_eof==NULL) proc_flag='\xff';
db_bof   =  (int(*)(void)      )GetProcAddress( hdr_dll, "_db_bof"); if(db_bof==NULL) proc_flag='\xff';
db_delete=  (int(*)(int flag=99))GetProcAddress( hdr_dll, "_db_delete"); if(db_delete==NULL) proc_flag='\xff';
set_delete= (int(*)(int flag=99))GetProcAddress( hdr_dll, "_set_delete"); if(set_delete==NULL) proc_flag='\xff';
db_append = (int(*) (void)                )GetProcAddress( hdr_dll, "_db_append"); if(db_append==NULL) proc_flag='\xff';
put_num =   (int(*)(char *value, int pole) )GetProcAddress( hdr_dll, "_put_num"); if(put_num==NULL) proc_flag='\xff';
put_name =  (int(*)(char *value, char *name) )GetProcAddress( hdr_dll, "_put_name"); if(put_name==NULL) proc_flag='\xff';
db_error =  (int(*)( void ))GetProcAddress( hdr_dll, "_db_error"); if(db_error==NULL) proc_flag='\xff';
db_lastrec= (long int(*) ( void))GetProcAddress( hdr_dll, "_db_lastrec"); if(db_lastrec==NULL) proc_flag='\xff';
find_field= (int(*)(char *name))GetProcAddress( hdr_dll, "_find_field"); if(find_field==NULL) proc_flag='\xff';
find_sort = (int(*)(char *pole, char *isk_stroka))GetProcAddress( hdr_dll, "_find_sort"); if(find_sort==NULL) proc_flag='\xff';
db_recno  = (long(*)( void )) GetProcAddress( hdr_dll, "_db_recno"); if(db_recno==NULL) proc_flag='\xff';
db_used   = (char(*)( void )) GetProcAddress( hdr_dll, "_db_used" ); if(db_used==NULL) proc_flag='\xff';
db_getname= (char*(*)(int  )) GetProcAddress( hdr_dll, "_db_getname" ); if(db_getname==NULL) proc_flag='\xff';
db_gettype= (char (*)(int  )) GetProcAddress( hdr_dll, "_db_gettype" ); if(db_gettype==NULL) proc_flag='\xff';
db_getlengt= (int (*)(int  )) GetProcAddress( hdr_dll, "_db_getlengt" ); if(db_getlengt==NULL) proc_flag='\xff';
db_getdecimal=(int(*)(int  )) GetProcAddress( hdr_dll, "_db_getdecimal" ); if(db_getdecimal==NULL) proc_flag='\xff';
db_zap   = (int (*)( void )) GetProcAddress( hdr_dll, "_db_zap" ); if(db_zap==NULL) proc_flag='\xff';
db_pack  = (int (*)( void )) GetProcAddress( hdr_dll, "_db_pack" ); if(db_pack==NULL) proc_flag='\xff';

//****  Если proc_flag==0 присвоение всех указателей прошло успешно ****************************

// В конце укажите:
// FreeLibrary(hdr_dll);

char *buf=(char*)calloc(4,sizeof(char));
char *tmp;


db_create1();
tmp=char-1;
for( i=00; i<199; i++){
    buf="A00";
    itoa(i,buf,3);
    db_create2(buf,'C',20,0);
}

db_create3("del.dbf");

db_open("del.DBF");
int dec=2, sign=0;
   int ndig = 10;

for(k=0; k<100; k++){
printf("\n%i",(100-k));
for(i=0 ; i<=1000 ; i++) {
   db_append();
   for(j=0; j<200; j++){
     put_num(ecvt(rand(), ndig, &dec, &sign),j)  ;
   }
}
}
db_close();

FreeLibrary(hdr_dll);
return i;
}
//---------------------------------------------------------------------------

