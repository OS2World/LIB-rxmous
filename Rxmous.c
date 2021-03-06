//Filename: RXDLL.C
//Rexx DLL template for the C-Challenged
//C-Challenged author: Jan Lambert  2007
//License: Freeware
//Requirements: Open Watcom C/C++ ver 1.6
//Warrentee: If good, i did it; if bad, i deny any knowledge of this package.
//Credits: Jorge Martins article: Building a REXX DLL in C, EDM/2
//         IBM/RexxLA.org: rxMath.c used as an example for Load/DropFuncs

#define INCL_MOU
#define INCL_DOSPROCESS
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rexxsaa.h>

#define  FUNC_FAILED   40            /*Rexx internal return codes*/
#define  FUNC_OK        0

/*RexxFunctionHandler RxDllLoadFuncs;*/  /*Block function registration for multifuncion dlls*/
/*RexxFunctionHandler RxDllDropFuncs;*/  /*Delete if not used*/

RexxFunctionHandler MousInit;
RexxFunctionHandler MousClos;
RexxFunctionHandler ClickPos;

#define LFUNC_OK      "0"       /*load/drop returns*/
#define LFUNC_FAILED  "1"

static PCSZ  RxFuncTbl[] = {
        "Rxmous",
        "MousInit",
        "ClickPos",      /*list of user functions*/
        "MousClos"
};
static int FuncCnt = sizeof(RxFuncTbl)/sizeof(PCSZ);

HMOU mh = 0;


ULONG MousInit(CHAR *name,         /* Initialization Routine */
               ULONG numargs,      /* Number of arguments */
               RXSTRING args[],    /* Null-terminated RXSTRINGs array*/
               CHAR *queuename,    /* Current external data queue name */
               RXSTRING *retstr)   /* returning RXSTRING  */
{
  int       j;
  USHORT    rc;
  CHAR      buff[255];

  if (numargs > 0) {             /* check arguments:if any, something bad may be happening */
    strlcpy(retstr->strptr,LFUNC_FAILED,255);
    retstr->strlength = strlen(retstr->strptr);
    return FUNC_FAILED;
  }

  for (j = 2; j < FuncCnt; j++) {
    RexxRegisterFunctionDll(RxFuncTbl[j],RxFuncTbl[0], RxFuncTbl[j]);
  }
  rc = MouOpen("", &mh);

  utoa(rc, buff,10);

  strlcpy(retstr->strptr,buff,255);
  retstr->strlength = strlen(retstr->strptr);
  return FUNC_OK;
}

ULONG MousClos(CHAR *name,  /* Termination Routine */
               ULONG numargs,      /* Number of arguments */
               RXSTRING args[],    /* Null-terminated RXSTRINGs array*/
               CHAR *queuename,    /* Current external data queue name */
               RXSTRING *retstr)   /* returning RXSTRING  */
{
  USHORT    j;

  if (numargs > 0) { /* check arguments:if any, something bad may be happening */
    strlcpy(retstr->strptr, LFUNC_FAILED, 255);
    retstr->strlength = strlen(retstr->strptr);
    return FUNC_FAILED;
  }
  for (j = 2; j < FuncCnt; j++) {
    RexxDeregisterFunction(RxFuncTbl[j]);
  }

  MouClose(mh);

  strlcpy(retstr->strptr, LFUNC_OK, 255);
  retstr->strlength = strlen(retstr->strptr);
  return FUNC_OK;
}

/*-----------  User Functions --------------- */

ULONG ClickPos(CHAR *name,  /* User Routine name */
        ULONG numargs,      /* Number of arguments */
        RXSTRING args[],    /* Null-terminated RXSTRINGs array*/
        CHAR *queuename,    /* Current external data queue name */
        RXSTRING *retstr)   /* return RXSTRING  */
{

  MOUEVENTINFO mouseinfo;
  PMOUEVENTINFO mei = &mouseinfo;
  USHORT   waitf = MOU_WAIT;
  PUSHORT  pwf = &waitf;
  CHAR  col[10];
  CHAR  row[10];
  CHAR  button[3] = "0 ";
  CHAR  timest[32];
  CHAR  result[128];
  USHORT  rc;


  LONG timedblclick = 0; // time for doubleclick check
  #define dblclickinterval  400L // msecs to recognize doublclick

  waitf = MOU_NOWAIT;
  result[0] = '\0';
  do {
    if (numargs > 0 ) DosSleep(1);  // with argument test for dblclick

    rc = MouReadEventQue(mei, pwf, mh);
//  if (mei->fs)
//    printf("BreadQ=%d fs=%d t1=%d t=%d\n",rc, mei->fs, timedblclick, mei->time);
//    printf("numargs %d \n", numargs );
    if ( (mei->fs & (MOUSE_BN1_DOWN | MOUSE_BN2_DOWN | MOUSE_BN3_DOWN)) ){
      utoa(mei->time, timest, 10);
      utoa(mei->row, row, 10);
      utoa(mei->col, col, 10);
    }
    if (mei->fs & MOUSE_BN1_DOWN) {
      if ('0' == button[0]) {    // start of dbl click ??
        timedblclick = dblclickinterval;
        button[0] = '1';
//      printf("1readQ=%d fs=%d t1=%d t=%d\n",rc, mei->fs, timedblclick, mei->time);
        if (0 == numargs )   break; // no dblclick test required
      } else {
        if (dblclickinterval > timedblclick ) { // doubleclick
          button[0] = 'D';
          break;
        }
      }
    } else {
      if (mei->fs & MOUSE_BN2_DOWN) {
        button[0] = '2';
        break;
      } else if (mei->fs & MOUSE_BN3_DOWN) {
        button[0] = '3';
        break;
      }
    }
    timedblclick -= 50;
    if (timedblclick < 0 ) break;
  } while (('1' == button[0]) && (dblclickinterval > timedblclick) );

/* clear the que */
  while (mei->fs) {
    rc = MouReadEventQue(mei, pwf, mh);
  }

  if ('0' == button[0]) {
    strlcpy(result, "0 0 0 0", sizeof(result));
  } else {
      strlcat(result, " ", sizeof(result));
      strlcat(result, row, sizeof(result));
      strlcat(result, " ", sizeof(result));
      strlcat(result, col, sizeof(result));
      strlcat(result, " ", sizeof(result));
      strlcat(result, button, sizeof(result));
      strlcat(result, timest, sizeof(result));
  }
  strlcpy(retstr->strptr, result, 255);        /*trim string if over default */
                                               /*  return string length */
  retstr->strlength = strlen(retstr->strptr);  /*return actual length*/
  return FUNC_OK;                              /*successful return code(0)*/
}
