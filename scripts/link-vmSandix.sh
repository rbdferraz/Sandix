#!/bin/bash
#
# vmSandix is linked from the objects selected by $(KBUILD_VMLINUX_INIT) and
# $(KBUILD_VMLINUX_MAIN). Most are built-in.o files from top-level directories
# in the kernel tree, others are specified in arch/$(ARCH)/Makefile.
# Ordering when linking is important, and $(KBUILD_VMLINUX_INIT) must be first.
#
# vmSandix
#   ^
#   |
#   +-< $(KBUILD_VMLINUX_INIT)
#   |   +--< arch/${SRCARCH}/kernel/head_${BITS}.o + init/built-in.o
#   |
#   +--< $(KBUILD_VMLINUX_MAIN)
#   |    +--< mm/built-in.o + drivers/built-in.o + more

# Error out on error
set -e

# Beautiful output in kbuild format
# Will be suppressed by "make -s"
info()
{
	if [ "${quiet}" != "silent_" ]; then
		printf "${COLOR_BEGIN}  %-7s %s${COLOR_END}\n" ${1} ${2}
	fi
}

# Link vmSandix
vmSandix_ld()
{
	local lds="${objtree}/${KBUILD_VMSANDIX_LDS}"

	${LD} -T ${lds} -o ${1}			\
		${KBUILD_VMSANDIX_INIT}		\
		--start-group			\
		${KBUILD_VMSANDIX_MAIN}		\
		--end-group
}

# System.map is used by module-init tools and some debugging
# tools to retrieve the actual addresses of symbols in the kernel.
#
# The second row specify the type of the symbol:
#   A = Absolute
#   B = Uninitialised data (.bss)
#   C = Common symbol
#   D = Initialised data
#   G = Initialised data for small objects
#   I = Indirect reference to another symbol
#   N = Debugging symbol
#   R = Read only
#   S = Uninitialised data for small objects
#   T = Text code symbol
#   U = Undefined symbol
#   V = Weak symbol
#   W = Weak symbol
#   Corresponding small letters are local symbols
#
# For System.map filter away:
#   a - local absolute symbols
#   U - undefined global symbols
#   N - debugging symbols
#   w - local weak symbols

mksysmap()
{
	$NM -n $1 > $2 
	#$NM -n $1 | grep -v '\( [aNUw] \)\|\(__crc_\)\|\( \$[adt]\)\|\( .L\)' > $2
}

cleanup()
{
	rm -f .vmSandix.cmd
	rm -f System.map
	rm -f vmSandix
	rm -f arch/${SRCARCH}/boot/System.map
	rm -f arch/${SRCARCH}/boot/vmSandix
}

# Enable Debug Mode; Print commands.
if [ "${KBUILD_VERBOSE}" = "1" ]; then
	set -x;
fi

# Link vmSandix
if [ "$1" == "LD" ]; then
	info LD vmSandix
	vmSandix_ld vmSandix

	info GEN .version
	if [ ! -r .version ]; then
		rm -f .version
		echo 1 > .version
	else
		mv .version .version.old
		expr 0$(cat .version.old) + 1 >.version
	fi

	info SYSMAP System.map
	mksysmap vmSandix System.map
	
	# Copy to make arch/$(SRCARCH)/boot/Makefile happy
	cp vmSandix arch/${SRCARCH}/boot/
	cp System.map arch/${SRCARCH}/boot/
fi

if [ "$1" == "clean" ]; then
	info CLEAN vmSandix
	cleanup
	exit 0
fi
