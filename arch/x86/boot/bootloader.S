#	Copyright (C) 2015-2016 Yizhou Shan <shan13@purdue.edu>
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License along
#	with this program; if not, write to the Free Software Foundation, Inc.,
#	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# Ugly boot loader for Sandix.
#
# 1) Load kernel setup to 0x9000:0000
# 2) Load kernel system to 0x1000:0000 first,
# 3) Then move kernel system from 0x1000:0000 to 0x100000
# 4) Jump to 0x9000:0200 setup.

BOOTSEG		= 0x07c0	# Bootloader Segment.
SETUPSEG	= 0x9000	# Setup Image Segment.
SYSSEG		= 0x1000	# System Image Segment.
SETUP_OFFSET	= 512		# Offset of entry point in setup

#define Bytes_For_Stack		512
#define ESP_VALUE		(512 + Bytes_For_Stack)

#define MSG(x)		movw $x, %si; call message
#define ERR(x)		movw $x, %si; jmp error_message

	.code16

	.text
	.globl start

start:
# We are not safe here, so disable interrupts
	cli

# Normalize [cs:ip] to [0x07c0:_start]
	ljmp	$BOOTSEG, $_start

_start:

# Segements are as follows: %cs = %ds = %ss = BOOTSEG
# %es = %fs = SETUPSEG, %gs is not used.

	movw	%cs, %ax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	$SETUPSEG, %ax
	movw	%ax, %fs
	movw	%ax, %es
	cld

# Set a reasonable stack.
# Use movl to clear upper 16-bit portion of %esp
	movl	$ESP_VALUE, %esp

# We are safe again
	sti

# Get Disk Geometry Parameters.
# In CHS scheme, Cylinder and Head start from 0, Sector start form 1,
# which means the fisrt sector is: 0/0/1. In LBA scheme, the first
# sector starts from 0.
#
# Using Cylinder/Head/Sector to calculate LBA:
#	LBA = (Cylinder * hPc + Head) * sPt + (Sector - 1)
# Where hPc is heads/cylinder, sPt is sectors/track
#
# In QEMU with SeaBIOS, normally, hPc=15, sPt=63, means one cylinder
# has capacity of 16*63*0.5 KB = 504KB. So, normally, it is NOT
# possible to load cross-cylinder, if you do cross, call 110.

	movb	%dl, boot_drive			# Save boot drive number

	movb	$8,%ah
	int	$0x13				# Get Disk Parameters

	xorw	%ax, %ax
	movb	%dh, %al
	movw	%ax, hPc			# Store heads/cylinder

	andw	$0x3f, %cx
	movw	%cx, sPt			# Store sectors/track

# Load Kernel Setup
# Load one sector first, then we read the sectors_of_setup
# and sys_size to decide how many sectors we need to load.
# We have %es = %fs = SETUPSEG = 0x9000

	xorw	%ax, %ax
	xorw	%dx, %dx
	movb	boot_drive, %dl
	int	$0x13				# Reset FDC

	xorw	%bx, %bx			# 0x9000:0000
	movw	$0x0201, %ax			# Service 2, nr_of_sectors = 1
	movw	$2, %cx				# Cylinder = 0, Sector = 2
	movb	boot_drive, %dl			# Head = 0, Boot Drive
	int	$0x13
	jc	bs_fail

	xorw	%ax, %ax
	movw	$0x1f1, %bx			# Defined in header.S
	movb	%fs:(%bx), %al			# sectors_of_setup, 1 byte
	pushw	%ax				# In case BIOS touching

# Load the remaining Kernel Setup
# Note that we have confined setup image to 64 sectors(It should be
# 62 sectors, in my opinion), so it is very safe to load it in the
# same head, of course, the same cylinder too.
# Note that we have %al=number of sectors to read already.

	movw	$0x200, %bx			# 0x9000:0200
	movb	$0x2, %ah			# Service 2
	movw	$3, %cx				# Cylinder = 0, Sector = 3
	movb	boot_drive, %dl			# Head = 0, Boot Drive
	int	$0x13
	jc	bs_fail

# Before loading system, calculating the size of system image first.
# Note that the sys_size field is in 16-bytes unit, therefore:
# sectors_of_system = (sys_size * 16 + 511) / 512

	xorl	%eax, %eax
	movw	$0x1f4, %bx			# Defined in header.S
	movl	%fs:(%bx), %eax			# sys_size, 4 bytes long
	movl	%eax, sys_size			# Store it

	sall	$4, %eax			# * 16
	addl	$511, %eax			# + 511
	shrl	$9, %eax			# / 512
	movw	%ax, sremain			# Store it

# Loading System in an ugly way.
# First, load the first portion that sleep in head 0. After that, we load
# the remaining. I am NOT good at assembly code, here you go.

	movw	$SYSSEG, %bx
	movw	%bx, %es
	xorw	%bx, %bx			# 0x1000:0000

	popw	%cx				# Restore sectors_of_setup
	addw	$3, %cx				# Starting sector number
	movw	%cx, sread			# I am pretty sure setup is small
	movw	$0, head			# So move to sread directly :)

	movw	sPt, %cx
	subw	sread, %cx
	addw	$1, %cx				# %cx = sPt - sread + 1
	cmpw	sremain, %cx
	jnl	1f				# JUMP if (%cx - sremain) >= 0
	subw	%cx, sremain			# if (%cx < sremain)
	movw	%cx, %ax			# then do sremain -= %cx
	jmp	2f				#      move %cx to %ax
1:
	movw	sremain, %ax			# if (%cx >= sremain)
	movw	$0, sremain			# then move sremain to %ax
						#      move 0 to sremain
2:
	movb	$0x2, %ah			# Service 2
	movw	sread, %cx			# Starting sector number
	movb	boot_drive, %dl			# Head = 0, Boot Drive
	int	$0x13
	jc	bs_fail

	movw	sremain, %ax
	testw	%ax, %ax			# Quick Test
	jz	bs_copy				# No more? Jump to setup

	movw	sPt, %bx			# Adjust loading base address
	subw	sread, %bx
	addw	$1, %bx				# %bx = sPt - sread + 1
	salw    $9, %bx				# %bx *= 512

# Now the remaining sectors after head 0
# (Should be merged with the code above) :(

	movw	$1, sread
loop:
	addw	$1, head
	movw	hPc, %cx
	cmpw	head, %cx			# Quick Test if cross-cylinder
	jl	bs_fail				# JUMP if (hPc - head) < 0

	movw	sremain, %cx
	cmpw	sPt, %cx
	jnl	1f				# JUMP if (sremain - sPt) >= 0
	movw	sremain, %ax			# if (sremain < sPt)
	movw	$0, sremain			# then move sremain to %al
	jmp	2f				#      move 0 to sremain
1:
	movw	sPt, %ax			# if (sremain >= sPt)
	subw	%ax, sremain			# then move sPt to %al
						#      sub sPt from sremain
2:
	movb	$0x2, %ah			# Service 2
	movw	sread, %cx			# Starting sector number
	movb	head, %dh			# Head X
	movb	boot_drive, %dl			# Boot drive
	int	$0x13
	jc	bs_fail
	
	movw	sremain, %ax
	testw	%ax, %ax			# Quick Test
	jz	bs_copy				# No more? Jump to setup
	
	movw	sPt, %ax			# Adjust loading base address
	salw    $9, %ax
	addw	%ax, %bx			# %bx += (sPt * 512)

	jmp	loop				# Next head

##
# Copy kernel from 0x10000 to 0x100000
# In real-mode, the destination of movsl is %es:%di,
# set %es to 0xffff, set %di to 0x0010, so the address of
# first byte is 0x100000. Using this trick, the upper boundary
# is 0xffff0+0xffff= 0x10ffef.
#
# Hope A20 is enabled?
#
# WARNING
# Be careful if kernel is bigger than 0xffef(65519) bytes,
# in that case, we need another round!

bs_copy:
	movl	sys_size, %ecx
	shll	$2, %ecx

# Obsolete: As I said above, this mechanism can only move 0xffef bytes.
# This is fine at my initial development, since the kernel image is small.
# But oneday, the kernel image exceeds 64KB, so the kernel_sig check fails
# at head_32.S (many thanks!). Then I have to find a solution to this.
#
# I use the address prefix 0x67 and use %ds:%esi and %ds:%edi to store address.
# This will work just like 32-bit model! This is cool and simple.
#
# BUT the CAVEAT is: I do not know whether all recent Intel CPUs can do this.
#
#	movw	$SYSSEG, %ax			# %ds:%si points to
#	movw	%ax, %ds			# 0x10000
#	xorl	%esi, %esi
#
#	movw	$0xffff, %ax			# %es:%di points to
#	movw	%ax, %es			# 0x100000
#	movl	$0x10, %edi

	xorw	%ax, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movl	$0x10000, %esi
	movl	$0x100000, %edi

	cld
	rep
	.byte 0x67				# The address prefix
	movsl					# including operand prefix

	ljmp	$SETUPSEG, $SETUP_OFFSET

	.align 4
# Some variables for disk operation
hPc:		.word 0				# Heads/Cylinder
sPt:		.word 0				# Sectors/Track
sread:		.word 0				# Starting sector number to read
head:		.word 0				# Current head
cylinder:	.word 0				# Current cylinder(not used)
sremain:	.word 0				# Sectors remained to read
sys_size:	.long 0				# sys_size of system image
boot_drive:	.byte 0				# Boot drive number

bs_fail:
	ERR(fail_to_load_sandix)

error_message:
	call	message
general_error:
	MSG(general_error_string)

	xorw	%ax, %ax
	int	$0x16
	int	$0x19

fail_to_load_sandix:	.asciz "Fail to load Sandix..."
general_error_string:	.asciz " Press any key to reboot\n\r"

##
# Use BIOS "int 10H Function 0Eh" to write character in teletype mode
# %ah = 0xe	%al = character
# %bh = page	%bl = foreground color (graphics modes)
#
# WARNING: trashes %si, %ax, and %bx
1:
	movw	$0x0001, %bx
	movb	$0xe, %ah
	int	$0x10
message:
	lodsb
	cmpb	$0, %al
	jne	1b
	ret

#
# Seriously, make sure the loader is < 512 bytes, this is MBR.
#

	.org 510
bootflag:	.word 0xAA55
