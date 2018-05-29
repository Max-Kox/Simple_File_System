// eFile.c
// Runs on TM4C123
// High-level implementation of the file system implementation.

#include <stdint.h>
#include "eDisk.h"

uint8_t Buff[512];							// temporary buffer used during file I/O
uint8_t Directory[256], FAT[256];
int32_t bDirectoryLoaded = 0;		// 0 means disk on ROM is complete, 1 means RAM version active

// Return the larger of two integers.
int16_t max(int16_t a, int16_t b)
{
	if(a > b)
	{
		return a;
	}
	return b;
}
//*****MountDirectory******
// if directory and FAT are not loaded in RAM,
// bring it into RAM from disk
void MountDirectory(void)
{
	if(bDirectoryLoaded == 0)
	{
		eDisk_ReadSector(Buff, 255);
		for(uint16_t i=0; i<256; i++)
		{
			Directory[i] = Buff[i];
			FAT[i] = Buff[i+256];
		}
		bDirectoryLoaded = 1;
	}	
}

// Return the index of the last sector in the file
// associated with a given starting sector.
// Also return 255 for file with name 255 (which doesn't exist)
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t lastsector(uint8_t start)
{
	uint8_t m;
	if(start == 255)
	{
		return 255;				// file does not exist
	}
	else
	{
		while(1)
		{
			m = FAT[start];
			if(m == 255) break;
			else start = m;
		}
		return start;
	}
}

// Return the index of the first free sector.
// Note: This function will loop forever without returning
// if a file has no end or if (Directory[255] != 255)
// (i.e. the FAT is corrupted).
uint8_t findfreesector(void)
{
	int16_t fs = -1;
	uint8_t ls;
	uint8_t i = 0;
	while(1)
	{
		ls = lastsector(Directory[i]);
		if(ls == 255) break;
		fs = max(fs, ls);
		i++;
	}
	return fs + 1;
}

// Append a sector index 'n' at the end of file 'num'.
// This helper function is part of OS_File_Append(), which
// should have already verified that there is free space,
// so it always returns 0 (successful).
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t appendfat(uint8_t num, uint8_t n)
{
	uint8_t m, i;
	i = Directory[num];
	if(i == 255)
	{
		Directory[num] = n;
	}
	else
	{
		m = FAT[i];
		while(m != 255)
		{
			i = m;
			m = FAT[i];
		}
		FAT[i] = n;
	}
	return 0;
}

//********OS_File_New*************
// Returns a file number of a new file for writing
// Inputs: none
// Outputs: number of a new file
// Errors: return 255 on failure or disk full
uint8_t OS_File_New(void)
{
	uint8_t i = 0;
	MountDirectory();
	while(Directory[i] != 255)
	{
		i++;
		if(i==255) return 255;
	}
	return i;
}

//********OS_File_Size*************
// Check the size of this file
// Inputs:  num, 8-bit file number, 0 to 254
// Outputs: 0 if empty, otherwise the number of sectors
// Errors:  none
uint8_t OS_File_Size(uint8_t num)
{
	uint8_t counter = 0;
	uint8_t m = Directory[num];
	if(m == 255) return 0;
	else
	{
		while(1)
		{
			counter++;
			m = FAT[m];
			if(m == 255) break;
		}
		return counter;
	}
}

//********OS_File_Append*************
// Save 512 bytes into the file
// Inputs:  num, 8-bit file number, 0 to 254
//          buf, pointer to 512 bytes of data
// Outputs: 0 if successful
// Errors:  255 on failure or disk full
uint8_t OS_File_Append(uint8_t num, uint8_t buf[512])
{
	uint8_t n;
	MountDirectory();
	n = findfreesector();
	if(n == 255) return 255;
	eDisk_WriteSector(buf, n);
	appendfat(num, n);
	return 0;
}

//********OS_File_Read*************
// Read 512 bytes from the file
// Inputs:  num, 8-bit file number, 0 to 254
//          location, logical address, 0 to 254
//          buf, pointer to 512 empty spaces in RAM
// Outputs: 0 if successful
// Errors:  255 on failure because no data
uint8_t OS_File_Read(uint8_t num, uint8_t location,
                     uint8_t buf[512])
{
	uint8_t counter = 0;
	uint8_t m = Directory[num];
	if(m == 255) return 255;				// file is empty
	else
	{
		while(1)
		{
			if(counter == location)
			{
				eDisk_ReadSector(buf, m);
				return 0;
			}
			m = FAT[m];
			counter++;
			if(m == 255) return 255;
		}
	}
}

//********OS_File_Flush*************
// Update working buffers onto the disk
// Power can be removed after calling flush
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Flush(void)
{
	for(uint16_t i=0; i<256; i++)
		{
			Buff[i] = Directory[i];
			Buff[i+256] = FAT[i];
		}
	eDisk_WriteSector(Buff, 255);
	return 0;
}

//********OS_File_Format*************
// Erase all files and all data
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Format(void)
{
	eDisk_Format();
	bDirectoryLoaded = 0;
	return 0;
}
