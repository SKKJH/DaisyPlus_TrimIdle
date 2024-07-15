#include "xtime_l.h"

XTime trimCmdStart, trimCmdEnd, trimCmdTime;
XTime bitStart, bitEnd, bitTime;
XTime dataStart, dataEnd, dataTime;
XTime bufStart, bufEnd, bufTime;
XTime tableStart, tableEnd, tableTime;
XTime writeStart, writeEnd, writeTime;

XTime ingbufStart, ingbufEnd, ingbufTime;
XTime ingtableStart, ingtableEnd, ingtableTime;
XTime ingwriteStart, ingwriteEnd, ingwriteTime;
XTime gcStart, gcEnd, gcTime;
int gc_cnt;

unsigned int trim_flag, do_trim_flag, trimming_flag, cmd_by_trim;
unsigned int nr, trimDevAddr;
int nr_sum, return_flag;
unsigned int trim_index;

//trim index : position of present trim (remember trim index)

void GetTrimData();
int DoTrim(int forced);
void TRIM(unsigned int LPN, unsigned int BLK0, unsigned int BLK1, unsigned int BLK2, unsigned int BLK3, unsigned int forced);
