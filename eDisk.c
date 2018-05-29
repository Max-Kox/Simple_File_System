// eDisk.c
// Runs on TM4C123
// Mid-level implementation of the solid state disk device
// driver.  Below this is the low level, hardware-specific
// flash memory interface.  Above this is the high level
// file system implementation.

#include <stdint.h>
#include "eDisk.h"
#include "FlashProgram.h"

//*************** eDisk_ReadSector ***********
// Read 1 sector of 512 bytes from the disk, data goes to RAM
// Inputs: pointer to an empty RAM buffer
//         sector number of disk to read: 0,1,2,...255
// Outputs: result
//  RES_OK        0: Successful
//  RES_ERROR     1: R/W Error
//  RES_WRPRT     2: Write Protected
//  RES_NOTRDY    3: Not Ready
//  RES_PARERR    4: Invalid Parameter
enum DRESULT eDisk_ReadSector(
    uint8_t *buff,				// Pointer to a RAM buffer into which to store
    uint8_t sector)				// sector number to read from
{
	uint8_t *addr;					// address of byte to copy
	if(EDISK_ADDR_MIN + 512*sector > EDISK_ADDR_MAX)
	{
		return RES_PARERR;		// address error, sector is too big
	}							
	addr = (uint8_t *)(EDISK_ADDR_MIN + 512*sector);
	for(uint16_t i=0; i<512; i++)
	{
		*buff = *addr;
		buff++;
		addr++;
	}
	return RES_OK;
}

//*************** eDisk_WriteSector ***********
// Write 1 sector of 512 bytes of data to the disk, data comes from RAM
// Inputs: pointer to RAM buffer with information
//         sector number of disk to write: 0,1,2,...,255
// Outputs: result
//  RES_OK        0: Successful
//  RES_ERROR     1: R/W Error
//  RES_WRPRT     2: Write Protected
//  RES_NOTRDY    3: Not Ready
//  RES_PARERR    4: Invalid Parameter
enum DRESULT eDisk_WriteSector(
    const uint8_t *buff,		// Pointer to the data to be written
    uint8_t sector)					// sector number
{
	if(EDISK_ADDR_MIN + 512*sector > EDISK_ADDR_MAX)
	{
		return RES_PARERR;			// address error, sector is too big
	}
	Flash_WriteArray((uint32_t *)buff, EDISK_ADDR_MIN + 512*sector, 512/4);
	return RES_OK;
}

//*************** eDisk_Format ***********
// Erase all files and all data by resetting the flash to all 1's
// Inputs: none
// Outputs: result
//  RES_OK        0: Successful
//  RES_ERROR     1: R/W Error
//  RES_WRPRT     2: Write Protected
//  RES_NOTRDY    3: Not Ready
//  RES_PARERR    4: Invalid Parameter
enum DRESULT eDisk_Format(void)
{
	for(uint16_t i=0; i<128; i++)
	{
		Flash_Erase(EDISK_ADDR_MIN + i*1024);
	}
	return RES_OK;
}
