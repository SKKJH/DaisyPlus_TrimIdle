unsigned int trim_flag, do_trim_flag, trimming_flag, cmd_by_trim;
unsigned int nr, trimDevAddr;
int nr_sum;
unsigned int trim_index;

//trim index : position of present trim (remember trim index)

void GetTrimData();
int DoTrim(int forced);
void TRIM(unsigned int LPN, unsigned int BLK0, unsigned int BLK1, unsigned int BLK2, unsigned int BLK3);
