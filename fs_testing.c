// fs_testing.c
// Runs on TM4C123
// High level test of the file system will initialize variables, store files, and access files.
// BSP library (board support package) is used for interface to the MKII BoosterPack.

#include <stdint.h>
#include "BSP.h"
#include "CortexM.h"
#include "eDisk.h"
#include "eFile.h"

extern uint8_t Buff[512];
extern uint8_t Directory[256], FAT[256];

// Test function: Copy a NULL-terminated 'inString' into the
// 'Buff' global variable with a maximum of 512 characters.
// Uninitialized characters are set to 0xFF.
// Inputs:  inString  pointer to NULL-terminated character string
// Outputs: none
void testbuildbuff(char *inString)
{
	uint32_t i = 0;
	while((i < 512) && (inString[i] != 0))
	{
		Buff[i] = inString[i];
		i = i + 1;
	}
	while(i < 512)
	{
		Buff[i] = 0xFF;					// fill the remainder of the buffer with 0xFF
		i = i + 1;
	}
}

// Test function: Draw a visual representation of the file system to the screen.
// This function reads the contents of the flash memory, so
// first call OS_File_Flush() to synchronize.
// Inputs:  index  starting index of directory and FAT
// Outputs: none
#define COLORSIZE		9
#define LCD_GRAY		0xCE59				// 200, 200, 200
const uint16_t ColorArray[COLORSIZE] = {LCD_YELLOW, LCD_BLUE, LCD_GREEN, LCD_RED, LCD_CYAN, LCD_LIGHTGREEN, LCD_ORANGE, LCD_MAGENTA, LCD_WHITE};
// display 12 lines of the directory and FAT
// used for debugging
// Input:  index is starting line number
// Output: none
void DisplayDirectory(uint8_t index)
{
	uint16_t dirclr[256], fatclr[256];
	volatile uint8_t *diraddr = (volatile uint8_t *)(EDISK_ADDR_MAX - 511);		// address of directory
	volatile uint8_t *fataddr = (volatile uint8_t *)(EDISK_ADDR_MAX - 255);		// address of FAT
	int i, j;
	// set default color to gray
	for(i=0; i<256; i=i+1)
	{
		dirclr[i] = LCD_GRAY;
		fatclr[i] = LCD_GRAY;
	}
	// set color for each active file
	for(i=0; i<255; i=i+1)
	{
		j = diraddr[i];
		if(j != 255)
		{
			dirclr[i] = ColorArray[i%COLORSIZE];
		}
		while(j != 255)
		{
			fatclr[j] = ColorArray[i%COLORSIZE];
			j = fataddr[j];
		}
	}
	// clear the screen if necessary (very slow but helps with button bounce)
	if((index + 11) > 255)
	{
		BSP_LCD_FillScreen(LCD_BLACK);
	}
	// print the column headers
	BSP_LCD_DrawString(5, 0, "DIR", LCD_GRAY);
	BSP_LCD_DrawString(15, 0, "FAT", LCD_GRAY);
	// print the cloumns
	i = 0;
	while((i <= 11) && ((index + i) <= 255))
	{
		BSP_LCD_SetCursor(0, i+1);
		BSP_LCD_OutUDec4((uint32_t)(index + i), LCD_GRAY);
		BSP_LCD_SetCursor(4, i+1);
		BSP_LCD_OutUDec4((uint32_t)diraddr[index+i], dirclr[index+i]);
		BSP_LCD_SetCursor(10, i+1);
		BSP_LCD_OutUDec4((uint32_t)(index + i), LCD_GRAY);
		BSP_LCD_SetCursor(14, i+1);
		BSP_LCD_OutUDec4((uint32_t)fataddr[index+i], fatclr[index+i]);
		i = i + 1;
	}
}

#include <stdlib.h>

uint8_t *ptr, *ptr2 = 0;
uint16_t i = 0;

int main_debug(void)
{
	ptr = (uint8_t *)malloc(512);
	ptr2 = (uint8_t *)malloc(512);
	*(ptr+i) = 0;
	i++;
	*(ptr+i) = 1;
	i++;
	*(ptr+i) = 2;
	i++;
	*(ptr+i) = 3;
	i++;
	*(ptr+i) = 4;
	i++;
	*(ptr+i) = 5;
	i++;
	*(ptr+i) = 6;
	while(i<512)
	{
		*(ptr+i) = 10;
		i++;
	}
	eDisk_WriteSector(ptr, 0);
	eDisk_WriteSector(ptr, 1);
	eDisk_WriteSector(ptr, 2);
	eDisk_ReadSector(ptr2, 0);
	eDisk_WriteSector(ptr, 255);
	eDisk_Format();
	while(1);
}

int main(void)
{
	uint8_t m, n, p;							// file numbers
	uint8_t index = 0;						// row index
	volatile int i;
	DisableInterrupts();
	BSP_Clock_InitFastest();
	BSP_Button1_Init();
	BSP_Button2_Init();
	BSP_LCD_Init();
	BSP_LCD_FillScreen(LCD_BLACK);

	if(BSP_Button2_Input() == 0)	// erase if Button2 is pressed
	{
		BSP_LCD_DrawString(0, 0, "Erasing entire disk", LCD_YELLOW);
		OS_File_Format();
		while(BSP_Button2_Input() == 0) {};
		BSP_LCD_DrawString(0, 0, "                   ", LCD_YELLOW);
	}
	EnableInterrupts();
	n = OS_File_New();						// n = 0, 3, 6, 9, ...
	testbuildbuff("buf0");
	OS_File_Append(n, Buff);			// 0x00020000
	testbuildbuff("buf1");
	OS_File_Append(n, Buff);			// 0x00020200
	testbuildbuff("buf2");
	OS_File_Append(n, Buff);			// 0x00020400
	testbuildbuff("buf3");
	OS_File_Append(n, Buff);			// 0x00020600
	testbuildbuff("buf4");
	OS_File_Append(n, Buff);			// 0x00020800
	testbuildbuff("buf5");
	OS_File_Append(n, Buff);			// 0x00020A00
	testbuildbuff("buf6");
	OS_File_Append(n, Buff);			// 0x00020C00
	testbuildbuff("buf7");
	OS_File_Append(n, Buff);			// 0x00020E00
	m = OS_File_New();						// m = 1, 4, 7, 10, ...
	testbuildbuff("dat0");
	OS_File_Append(m, Buff);			// 0x00021000
	testbuildbuff("dat1");
	OS_File_Append(m, Buff);			// 0x00021200
	testbuildbuff("dat2");
	OS_File_Append(m, Buff);			// 0x00021400
	testbuildbuff("dat3");
	OS_File_Append(m, Buff);			// 0x00021600
	p = OS_File_New();						// p = 2, 5, 8, 11, ...
	testbuildbuff("arr0");
	OS_File_Append(p, Buff);			// 0x00021800
	testbuildbuff("arr1");
	OS_File_Append(p, Buff);			// 0x00021A00
	testbuildbuff("buf8");
	OS_File_Append(n, Buff);			// 0x00021C00
	testbuildbuff("buf9");
	OS_File_Append(n, Buff);			// 0x00021E00
	testbuildbuff("arr2");
	OS_File_Append(p, Buff);			// 0x00022000
	testbuildbuff("dat4");
	OS_File_Append(m, Buff);			// 0x00022200
	i = OS_File_Size(n);					// i = 10
	i = OS_File_Size(m);					// i = 5
	i = OS_File_Size(p);					// i = 3
	i = OS_File_Size(p+1);				// i = 0
	OS_File_Flush();							// 0x0003FE00
	while(1)
	{
		DisplayDirectory(index);
		while((BSP_Button1_Input() != 0) && (BSP_Button2_Input() != 0)) {};
		if(BSP_Button1_Input() == 0)
		{
			if(index > 11)
			{
				index = index - 11;
			}
			else
			{
				index = 0;
			}
		}
		if(BSP_Button2_Input() == 0)
		{
			if((index + 11) <= 255)
			{
				index = index + 11;
			}
		}
		while((BSP_Button1_Input() == 0) || (BSP_Button2_Input() == 0)) {};
	}
}
