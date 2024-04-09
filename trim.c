#include "xil_printf.h"
#include <assert.h>
#include "nvme/nvme.h"
#include "nvme/host_lld.h"
#include "memory_map.h"
#include "address_translation.h"
#include "ftl_config.h"
#include "data_buffer.h"
#include "trim.h"

void TrimOperation() {
	trim_flag = 0;
	DATASET_MANAGEMENT_RANGE* dmRange = malloc(sizeof(DATASET_MANAGEMENT_RANGE) * nr);

	unsigned int tempaddr = trimdevAddr;
	//xil_printf("num of ranges = %d\r\n",nr);
	
	for (unsigned int i = 0; i < nr; i++) {
        unsigned int *ptr = (unsigned int *)tempaddr;
        dmRange[i].ContextAttributes.value = *ptr;
        dmRange[i].lengthInLogicalBlocks = *(ptr + 1);
        dmRange[i].startingLBA[0] = *(ptr + 2);
        dmRange[i].startingLBA[1] = *(ptr + 3);
        tempaddr += sizeof(unsigned int)*4;
    }

	for (unsigned int i = 0; i < nr; i++) {
		unsigned int tempStartLBA = dmRange[i].startingLBA[0];
		unsigned int tempLength = dmRange[i].lengthInLogicalBlocks;
       		unsigned int remainLBA = dmRange[i].startingLBA[0] % 4;
		
		switch (remainLBA) {
			case 1:
				tempStartLBA += 3;
				tempLength -= 3;
				break;
			case 2:
				tempStartLBA += 2;
				tempLength -=2;
				break;

			case 3:
				tempStartLBA += 1;
				tempLength -= 1;
				break;
		}

		while (tempLength >= 4) {
			//xil_printf("Trim Start LBA = %d\r",tempStartLBA);
			unsigned int BufEntry = CheckDataBufHitByLBA(tempStartLBA);
			//xil_printf("Trim Buf Entry = %d\r\n",BufEntry);
			if(BufEntry != DATA_BUF_FAIL){
				//SelectiveGetFromDataBufHashList(BufEntry);
                		//dataBufMapPtr->dataBuf[BufEntry].logicalSliceAddr = LSA_NONE;
                		dataBufMapPtr->dataBuf[BufEntry].blockingReqTail = REQ_SLOT_TAG_NONE;
               			//dataBufMapPtr->dataBuf[BufEntry].hashPrevEntry = DATA_BUF_NONE;
                		//dataBufMapPtr->dataBuf[BufEntry].hashNextEntry = DATA_BUF_NONE;
                		dataBufMapPtr->dataBuf[BufEntry].dirty = DATA_BUF_CLEAN;
                		dataBufMapPtr->dataBuf[BufEntry].reserved0 = 0;
			}	 
            unsigned int logicalSliceAddr = tempStartLBA / NVME_BLOCKS_PER_SLICE;
            InvalidateOldVsa(logicalSliceAddr);
			tempLength -= 4;
			tempStartLBA += 4;
		}

	}
	trim_flag = 0;
    free(dmRange);
}

