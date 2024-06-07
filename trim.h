unsigned int trim_flag, do_trim_flag, trimming_flag, cmd_by_trim
unsigned int nr, nr_sum, trimDevAddr;
unsigned int trim_index;

//trim index : position of present trim (remember trim index)

void GetTrimData();
void DoTrim();
int TRIM(unsigned int LPN, unsigned int BLK0, unsigned int BLK1, unsigned int BLK2, unsigned int BLK3);
