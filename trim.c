#include "trim.h"
#include "data_buffer.h"
#include "request_allocation.h"
#include "request_transform.h"
#include "address_translation.h"

void GetTrimData()
{
	trim_flag = 0;
	unsigned int tempAddr = trimDevAddr;
	unsigned int tail = dmRangePtr->tail;
	unsigned int sLPN, length ;
	xil_printf("Get Trim Data nr %d\r\n",nr);
	for (int i=tail; i<tail+nr; i++)
	{
		int j = i%3000;
		unsigned int *ptr = (unsigned int*)tempAddr;

		dmRangePtr->dmRange[j].ContextAttributes.value = *(ptr);
		dmRangePtr->dmRange[j].lengthInLogicalBlocks = *(ptr+1);
		dmRangePtr->dmRange[j].startingLBA[0] = *(ptr + 2);
		dmRangePtr->dmRange[j].startingLBA[1] = *(ptr + 3);
//		xil_printf("real lba : %d\r\n", *(ptr + 2));
//		xil_printf("real length : %d\r\n", *(ptr + 1));

		if (dmRangePtr->dmRange[j].startingLBA[0] < 4)
		{
			sLPN = 0;
		}
		else
		{
			sLPN = dmRangePtr->dmRange[j].startingLBA[0] / 4;
		}

		if (dmRangePtr->dmRange[j].lengthInLogicalBlocks < 4)
		{
			length = 2;
		}
		else
		{
			length = dmRangePtr->dmRange[j].lengthInLogicalBlocks / 4 + 1;
		}
		for (int j=sLPN; j<sLPN+length; j++)
		{
			trimValidBitMapPtr->valid_bit_map[j/64] |= (1ULL << (j % 64));
		}

		dmRangePtr->tail = dmRangePtr->tail + 1;
		if(dmRangePtr->tail==dmRangePtr->head)
		{
			if(dmRangePtr->head==2999)
			{
				dmRangePtr->head = 0;
			}
			else
			{
				dmRangePtr->head += 1;
			}
		}
		tempAddr += sizeof(unsigned int)*4;
	}
	nr_sum += nr;
	do_trim_flag = 1;
	return_flag = 1;
}

int DoTrim(int forced)
{
	if (forced == 1)
	{
		xil_printf("idle trim start\r\n");
	}

	int nlb, startLba, tempLsa, nvmeBlockOffset, BLK0, BLK1, BLK2, BLK3, tempLength, tempsLba, temp_nr_sum;
	temp_nr_sum = nr_sum;
	return_flag = 0;

	unsigned int head = dmRangePtr->head;
	for (int l=0; l<temp_nr_sum; l++)
	{
		int i = head + l;
		nlb = dmRangePtr->dmRange[i].lengthInLogicalBlocks;
		startLba = dmRangePtr->dmRange[i].startingLBA[0];
		nvmeBlockOffset = (startLba % NVME_BLOCKS_PER_SLICE);	
		tempLsa = startLba / NVME_BLOCKS_PER_SLICE;
		tempLength = nlb;
		tempsLba = startLba;
		BLK0 = 0;
		BLK1 = 0;
		BLK2 = 0;
		BLK3 = 0;	
		if (tempLength == 1)
		{
			if (nvmeBlockOffset == 0)
			{
				BLK0 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempLength -= 1;
			}
			else if (nvmeBlockOffset == 1)
			{
				BLK1 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempLength -= 1;
			}
			else if (nvmeBlockOffset == 2)
			{
				BLK2 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempLength -= 1;
			}
			else
			{
				BLK3 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempLength -= 1;
			}
		}
		else if (tempLength == 2)
		{
			if (nvmeBlockOffset == 0)
			{
				BLK0 = 1;
				BLK1 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempLength -= 2;
			}
			else if (nvmeBlockOffset == 1)
			{
				BLK1 = 1;
				BLK2 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempLength -= 2;
			}
			else if (nvmeBlockOffset == 2)
			{
				BLK2 = 1;
				BLK3 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempLength -= 2;
			}
			else
			{
				BLK3 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempsLba += 1;
				tempLength -= 1;
			}
		}
		else if (tempLength == 3)
		{
			if (nvmeBlockOffset == 0)
			{
				BLK0 = 1;
				BLK1 = 1;
				BLK2 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempLength -= 3;
			}
			else if (nvmeBlockOffset == 1)
			{
				BLK1 = 1;
				BLK2 = 1;
				BLK3 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempLength -= 3;
			}
			else if (nvmeBlockOffset == 2)
			{
				BLK2 = 1;
				BLK3 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempsLba += 2;
				tempLength -= 2;
			}
			else
			{
				BLK3 = 1;
				TRIM(tempLsa, BLK0, BLK1, BLK2, BLK3, forced);
				tempsLba += 1;
				tempLength -= 1;
			}
		}
		while (tempLength >= 4)
		{
			tempLsa = tempsLba / NVME_BLOCKS_PER_SLICE;
			TRIM(tempLsa, 1, 1, 1, 1, forced);
			tempsLba += 4;
			tempLength -= 4;

			if (forced == 1)
			{
				cmd_by_trim = check_nvme_cmd_come();
				if(cmd_by_trim == 1)
				{
					dmRangePtr->dmRange[i].startingLBA[0] = tempsLba;
					dmRangePtr->dmRange[i].lengthInLogicalBlocks = tempLength;
					trim_index = i;
					xil_printf("new cmd come\r\n");
					return 1;
				}
			}
		}
		tempLsa = tempsLba / NVME_BLOCKS_PER_SLICE;
		if (tempLength == 3)
		{
			TRIM(tempLsa, 1, 1, 1, 0, forced);
		}
		else if (tempLength == 2)
		{
			TRIM(tempLsa, 1, 1, 0, 0, forced);
		}
		else if (tempLength == 1)
		{
			TRIM(tempLsa, 1, 0, 0, 0, forced);
		}

		if(dmRangePtr->head==2999)
		{
			dmRangePtr->head = 0;
		}
		else
		{
			dmRangePtr->head += 1;
		}
		nr_sum -= 1;
	}

	do_trim_flag = 0;
	dmRangePtr->head = 0;
	dmRangePtr->tail = 0;

	if (forced == 1)
	{
		xil_printf("idle trim done\r\n");
	}
	return 0;
}

void TRIM(unsigned int LPN, unsigned int BLK0, unsigned int BLK1, unsigned int BLK2, unsigned int BLK3, unsigned int forced) {
	if (trimValidBitMapPtr->valid_bit_map[LPN / 64] & (1ULL << (LPN % 64)))
	{
		unsigned int bufEntry = CheckDataBufHitByLSA(LPN);
		if (bufEntry != DATA_BUF_FAIL) {
			if (BLK0 == 1) {
				dataBufMapPtr->dataBuf[bufEntry].blk0 = 0;
			}
			if (BLK1 == 1) {
				dataBufMapPtr->dataBuf[bufEntry].blk1 = 0;
			}
			if (BLK2 == 1) {
				dataBufMapPtr->dataBuf[bufEntry].blk2 = 0;
			}
			if (BLK3 == 1) {
				dataBufMapPtr->dataBuf[bufEntry].blk3 = 0;
			}
			if ((dataBufMapPtr->dataBuf[bufEntry].blk0 == 0) &&
				(dataBufMapPtr->dataBuf[bufEntry].blk1 == 0) &&
				(dataBufMapPtr->dataBuf[bufEntry].blk2 == 0) &&
				(dataBufMapPtr->dataBuf[bufEntry].blk3 == 0))
			{
				unsigned int prevBufEntry, nextBufEntry;
				prevBufEntry = dataBufMapPtr->dataBuf[bufEntry].prevEntry;
				nextBufEntry = dataBufMapPtr->dataBuf[bufEntry].nextEntry;

				if (prevBufEntry != DATA_BUF_NONE && nextBufEntry != DATA_BUF_NONE) {
					dataBufMapPtr->dataBuf[prevBufEntry].nextEntry = nextBufEntry;
					dataBufMapPtr->dataBuf[nextBufEntry].prevEntry = prevBufEntry;
					nextBufEntry = DATA_BUF_NONE;
					prevBufEntry = dataBufLruList.tailEntry;
					dataBufMapPtr->dataBuf[dataBufLruList.tailEntry].nextEntry = bufEntry;
					dataBufLruList.tailEntry = bufEntry;
				} else if (prevBufEntry != DATA_BUF_NONE && nextBufEntry == DATA_BUF_NONE) {
					dataBufLruList.tailEntry = bufEntry;
				} else if (prevBufEntry == DATA_BUF_NONE && nextBufEntry != DATA_BUF_NONE) {
					dataBufMapPtr->dataBuf[nextBufEntry].prevEntry = DATA_BUF_NONE;
					dataBufLruList.headEntry = nextBufEntry;
					prevBufEntry = dataBufLruList.tailEntry;
					dataBufMapPtr->dataBuf[dataBufLruList.tailEntry].nextEntry = bufEntry;
					dataBufLruList.tailEntry = bufEntry;
				} else {
					prevBufEntry = DATA_BUF_NONE;
					nextBufEntry = DATA_BUF_NONE;
					dataBufLruList.headEntry = bufEntry;
					dataBufLruList.tailEntry = bufEntry;
				}
				SelectiveGetFromDataBufHashList(bufEntry);
				dataBufMapPtr->dataBuf[bufEntry].blockingReqTail = REQ_SLOT_TAG_NONE;
				dataBufMapPtr->dataBuf[bufEntry].dirty = DATA_BUF_CLEAN;
				dataBufMapPtr->dataBuf[bufEntry].reserved0 = 0;
			}
		}

		unsigned int virtualSliceAddr = logicalSliceMapPtr->logicalSlice[LPN].virtualSliceAddr;
		if (virtualSliceAddr != VSA_NONE) {
			virtualSliceMapPtr->virtualSlice[virtualSliceAddr].logicalSliceAddr = LSA_NONE;
			if (BLK0 == 1) {
				logicalSliceMapPtr->logicalSlice[LPN].blk0 = 0;
			}
			if (BLK1 == 1) {
				logicalSliceMapPtr->logicalSlice[LPN].blk1 = 0;
			}
			if (BLK2 == 1) {
				logicalSliceMapPtr->logicalSlice[LPN].blk2 = 0;
			}
			if (BLK3 == 1) {
				logicalSliceMapPtr->logicalSlice[LPN].blk3 = 0;
			}
			if ((logicalSliceMapPtr->logicalSlice[LPN].blk0 == 0) &&
				(logicalSliceMapPtr->logicalSlice[LPN].blk1 == 0) &&
				(logicalSliceMapPtr->logicalSlice[LPN].blk2 == 0) &&
				(logicalSliceMapPtr->logicalSlice[LPN].blk3 == 0))
			{
				InvalidateOldVsa(LPN);
			}
		}
	}
}

