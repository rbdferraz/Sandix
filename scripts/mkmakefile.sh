#!/bin/sh
# Generates a small Makefile used in the root of the output
# directory, to allow make to be started from there.
# The Makefile also allow for more convinient build of external modules

# Usage
# $1 - Kernel src directory
# $2 - Output directory
# $3 - version
# $4 - patchlevel


test ! -r $2/Makefile -o -O $2/Makefile || exit 0
# Only overwrite automatically generated Makefiles
# (so we do not overwrite kernel Makefile)
if test -e $2/Makefile && ! grep -q Automatically $2/Makefile
then
	exit 0
fi
if [ "${quiet}" != "silent_" ]; then
	echo "  GEN     $2/Makefile"
fi

cat << EOF > $2/Makefile
# Automatically generated by $0: don't edit

VERSION = $3
PATCHLEVEL = $4

lastword = \$(word \$(words \$(1)),\$(1))
makedir := \$(dir \$(call lastword,\$(MAKEFILE_LIST)))

ifeq ("\$(origin V)", "command line")
VERBOSE := \$(V)
endif
ifneq (\$(VERBOSE),1)
Q := @
endif

MAKEARGS := -C $1
MAKEARGS += O=\$(if \$(patsubst /%,,\$(makedir)),\$(CURDIR)/)\$(patsubst %/,%,\$(makedir))

MAKEFLAGS += --no-print-directory

.PHONY: __sub-make \$(MAKECMDGOALS)

__sub-make:
	\$(Q)\$(MAKE) \$(MAKEARGS) \$(MAKECMDGOALS)

\$(filter-out __sub-make, \$(MAKECMDGOALS)): __sub-make
	@:
EOF