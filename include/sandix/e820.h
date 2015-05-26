#ifndef _SANDIX_X86_E820_H
#define _SANDIX_X86_E820_H

#include <sandix/types.h>

#define E820MAP			0x2d0	/* our map */
#define E820MAX			128		/* number of entries in E820MAP */
#define E820NR			0x1e8	/* # entries in E820MAP */

#define E820_RAM		1		/* Available RAM */
#define E820_RESERVED	2		/* Reserved */
#define E820_ACPI		3
#define E820_NVS		4
#define E820_UNUSABLE	5


struct e820entry {
	__u64 addr;	/* start of memory segment */
	__u64 size;	/* size of memory segment */
	__u32 type;	/* type of memory segment */
} __attribute__((packed));

struct e820map {
	__u32 nr_map;
	struct e820entry map[E820MAX];
};

#define ISA_START_ADDRESS	0xa0000
#define ISA_END_ADDRESS		0x100000

#define BIOS_BEGIN		0x000a0000
#define BIOS_END		0x00100000

#define BIOS_ROM_BASE		0xffe00000
#define BIOS_ROM_END		0xffffffff

#endif /* _SANDIX_X86_E820_H */
