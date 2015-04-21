/*
 *	2015/04/11. Created by Shan Yizhou.
 *
 *	main.c: 16-bit image main body, is responsible for getting the
 *	system data from the BIOS, and putting them into the appropriate
 *	places in system memory. 
 *
 *	This code asks the bios for memory/disk/other parameters, and
 *	puts them in a "safe" place: 0x90000-0x901FF, ie where the
 *	boot-block used to be. It is then up to the protected mode
 *	system to read them from there before the area is overwritten
 *	for buffer-blocks.
 *	
 *	BIG FAT NOTE: We're in real mode using 64k segments.  Therefore segment
 *	addresses must be multiplied by 16 to obtain their respective linear
 *	addresses. To avoid confusion, linear addresses are written using leading
 *	hex while segment addresses are written as segment:offset.
 */

/*
 * I'm using i386-elf-gcc-4.7.2 which do not support -m16 opinion.
 * The -m16 opinion shows up in gcc-4.9+.
 * So, insert this stub in the begining to generate 16-bit object
 * code. Remember, gcc do NOT generate real 16-bit code, it only 
 * generate 32-bit code. It's the assembler's job to add prefix
 * like 0x66 before instructions which use 32-bit data or address.
 */
asm(".code16gcc");

#include "boot.h"


void main(void){
/*
	puts("Hello World\n");
	printf("%u %d %u %d\n", 0xFFFFFFFF, 0xFFFFFFFF, 0x0000000F, 0x0);
	printf("Test %%p: %p\n", 0xABCDEF);
	printf("%x %X %o %s %c \n",0xABCD,0xaBdc,255,"HELLO", 'A');
*/
	puts("DEBUG: Now in main()...\n");
	enable_a20_bios();
}
