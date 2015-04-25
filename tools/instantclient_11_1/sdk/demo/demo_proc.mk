#/bin/make
###############################################################################
#                       Make file for PROC demos
###############################################################################
#  Usage :
# For compiling proc demos
#    make -f demo_proc.mk
#
# For precompiling, compiling & linking the procdemo.pc file
#    make -f demo_proc.mk build EXE=procdemo OBJS=procdemo.o
#
# In general, for any proc program
#    make -f demo_proc.mk build EXE=<exename> OBJS="<list of dependent objs>"
#
# NOTE: Please change cc and CC to point to the appropiate location on your
#       machine.
#
###############################################################################


CC=/usr/bin/gcc
cc=/usr/bin/gcc

# InstantClient Directories.
ICINCHOME=../
ICLIBHOME=../../
ICLIBPATH=-L$(ICLIBHOME)

MKLINK=ln
REMOVE=rm -rf
CLNCACHE=cleancache
CACHEDIR=SunWS_cachea
MAKE=make
MAKEFILE=demo_proc.mk
PROCDEMO=procdemo

PROC=../proc
SO_EXT=.so
I_SYM=-I

CCINCLUDES= $(I_SYM)$(ICINCHOME)include

# Pre-compiler Flags.
PRECOMP_INCLUDE=$(PRECOMPPUBH) $(I_SYM). $(SYS_INCLUDE)
PRECOMPPUBH=$(CCINCLUDES)
SYS_INCLUDE='sys_include=($(ORACLE_HOME)/precomp/public,$(SYS_INCLUDE_PATH))'
SYS_INCLUDE_PATH=/usr/include,/usr/lib/gcc/i386-redhat-linux/4.1.1/include,/usr/lib/gcc/i386-redhat-linux/3.4.5/include,/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include,/usr/lib/gcc/i586-suse-linux/4.1.0/include

# Compiler Flags.
OPTIMIZE=-O2
LDPATHFLAG=-L
SPFLAGS=-DLINUX -D_GNU_SOURCE -D_LARGEFILE64_SOURCE=1 -D_LARGEFILE_SOURCE=1 -DSLTS_ENABLE -DSLMXMX_ENABLE -D_REENTRANT -DNS_THREADS
CCFLAGS= -fPIC -DPRECOMP
LDFLAGS=-g
LPFLAGS=
GFLAG=
CDEBUG=
USRFLAGS=
PFLAGS=$(CCINCLUDES) $(SPFLAGS) $(LPFLAGS)
CFLAGS=$(GFLAG) $(OPTIMIZE) $(CDEBUG) $(CCFLAGS) $(PFLAGS) $(USRFLAGS)

# Libraries.
PROLDLIBS=$(LDCLIENTLIBS) $(THREADLIBS)
LDCLIENTLIBS=$(ICLIBPATH) $(LLIBCLNTSH) $(LDLIBS)
LLIBCLNTSH=$(LDLIBFLAG)$(LIBCLNTSHNAME)
LDLIBFLAG=-l
LIBCLNTSHNAME=clntsh
LDLIBS=$(EXSYSLIBS) $(MATHLIB) $(USRLIBS)
EXSYSLIBS=-ldl
MATHLIB=-lm
THREADLIBS=-lpthread

C2O=$(CC) $(CFLAGS) -c $*.c
PCC2C=$(PROC) $(PROCFLAGS) iname=$(PCCSRC) $(PRECOMP_INCLUDE)
DEMO_PROC_BUILD=$(CC) -o $(EXE) $(OBJS) $(LDFLAGS) $(PROLDLIBS)

#-----------------------------------------------------------------------------
# Targets for building the proc sample programs.
all: clean $(PROCDEMO)

$(PROCDEMO):
	$(MAKE) -f $(MAKEFILE) build OBJS=$@.o EXE=$@
	
build: $(CLNCACHE) $(OBJS)
	$(MKLINK) $(ICLIBHOME)libclntsh$(SO_EXT).11.1 $(ICLIBHOME)libclntsh$(SO_EXT)
	$(DEMO_PROC_BUILD)
	$(REMOVE) $(ICLIBHOME)libclntsh$(SO_EXT)

#-----------------------------------------------------------------------------
# Here are some rules for converting .pc -> .c -> .o
.SUFFIXES: .pc .c .o

pc1:
	$(PCC2C)

.pc.c:
	$(MAKE) -f $(MAKEFILE) PROCFLAGS="$(PROCFLAGS)" PCCSRC=$* I_SYM=include= pc1

.pc.o:
	$(MAKE) -f $(MAKEFILE) PROCFLAGS="$(PROCFLAGS)" PCCSRC=$* I_SYM=include= pc1
	$(C2O)

.c.o:
	$(C2O)

#-----------------------------------------------------------------------------
# Clean up all executables, *.o and generated *.c files
clean: $(CLNCACHE)
	$(REMOVE) $(PROCDEMO) $(PROCDEMO).o $(PROCDEMO).c $(PROCDEMO).lis

cleancache:
	$(REMOVE) $(CACHEDIR)
	$(REMOVE) $(ICLIBHOME)libclntsh$(SO_EXT)

