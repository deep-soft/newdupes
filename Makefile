# jdupes Makefile

# Default flags to pass to the C compiler (can be overridden)
CFLAGS ?= -O2 -g

# PREFIX determines where files will be installed. Common examples
# include "/usr" or "/usr/local".
PREFIX ?= /usr/local

# PROGRAM_NAME determines the installation name and manual page name
PROGRAM_NAME = jdupes

# BIN_DIR indicates directory where program is to be installed.
# Suggested value is "$(PREFIX)/bin"
BIN_DIR = $(PREFIX)/bin

# MAN_DIR indicates directory where the jdupes man page is to be
# installed. Suggested value is "$(PREFIX)/man/man1"
MAN_BASE_DIR = $(PREFIX)/share/man
MAN_DIR = $(MAN_BASE_DIR)/man1
MAN_EXT = 1

# Required external tools
CC ?= gcc
INSTALL = install
RM      = rm -f
RMDIR   = rmdir -p
MKDIR   = mkdir -p
INSTALL_PROGRAM = $(INSTALL) -m 0755
INSTALL_DATA    = $(INSTALL) -m 0644

# Main object files
OBJS += jdupes.o libjodycode_check.o

# Configuration section
COMPILER_OPTIONS = -Wall -Wwrite-strings -Wcast-align -Wstrict-aliasing -Wstrict-prototypes -Wpointer-arith -Wundef
COMPILER_OPTIONS += -Wshadow -Wfloat-equal -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Wformat=2
COMPILER_OPTIONS += -std=gnu11 -D_FILE_OFFSET_BITS=64 -fstrict-aliasing -pipe
COMPILER_OPTIONS += -DNO_ATIME

# Remove unused code if requested
ifdef GC_SECTIONS
 COMPILER_OPTIONS += -fdata-sections -ffunction-sections
 LINK_OPTIONS += -Wl,--gc-sections
endif


UNAME_S=$(shell uname -s)


# Are we running on a Windows OS?
ifeq ($(OS), Windows_NT)
 ifndef NO_WINDOWS
  ON_WINDOWS=1
  CFLAGS += -DNO_PERMS
 endif
endif

# Debugging code inclusion
ifdef LOUD
 DEBUG=1
 COMPILER_OPTIONS += -DLOUD_DEBUG
endif
ifdef DEBUG
 COMPILER_OPTIONS += -DDEBUG
else
 COMPILER_OPTIONS += -DNDEBUG
endif
ifdef HARDEN
 COMPILER_OPTIONS += -Wformat -Wformat-security -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fPIE -fpie -Wl,-z,relro -Wl,-z,now
endif

# MinGW needs this for printf() conversions to work
ifdef ON_WINDOWS
 ifndef NO_UNICODE
  UNICODE=1
  COMPILER_OPTIONS += -municode
 endif
 SUFFIX=.exe
 LIBEXT=.dll
 COMPILER_OPTIONS += -D__USE_MINGW_ANSI_STDIO=1 -DON_WINDOWS=1
 ifeq ($(UNAME_S), MINGW32_NT-5.1)
  OBJS += winres_xp.o
 else
  OBJS += winres.o
 endif
 override undefine ENABLE_DEDUPE
 DISABLE_DEDUPE = 1
else
 LIBEXT=.so
endif

# Don't use unsupported compiler options on gcc 3/4 (Mac OS X 10.5.8 Xcode)
# ENABLE_DEDUPE by default - macOS Sierra 10.12 and up required
ifeq ($(UNAME_S), Darwin)
 GCCVERSION = $(shell expr `LC_ALL=C gcc -v 2>&1 | grep '[cn][cg] version' | sed 's/[^0-9]*//;s/[ .].*//'` \>= 5)
 ifndef DISABLE_DEDUPE
  ENABLE_DEDUPE = 1
 endif
else
 GCCVERSION = 1
 BDYNAMIC = -Wl,-Bdynamic
 BSTATIC = -Wl,-Bstatic
endif

ifeq ($(GCCVERSION), 1)
 COMPILER_OPTIONS += -Wextra -Wstrict-overflow=5 -Winit-self
endif


### Find and use nearby libjodycode by default
ifndef IGNORE_NEARBY_JC
 ifneq ("$(wildcard ../libjodycode/libjodycode.h)","")
  $(info Found and using nearby libjodycode at ../libjodycode)
  COMPILER_OPTIONS += -I../libjodycode -L../libjodycode
  ifeq ("$(wildcard ../libjodycode/version.o)","")
   $(error You must build libjodycode before building jdupes)
  endif
 endif
 STATIC_LDFLAGS += ../libjodycode/libjodycode.a
 ifdef ON_WINDOWS
  DYN_LDFLAGS += -l:../libjodycode/libjodycode$(LIBEXT)
 else
  DYN_LDFLAGS += -ljodycode
 endif
endif


CFLAGS += $(COMPILER_OPTIONS) $(CFLAGS_EXTRA)
LDFLAGS += $(LINK_OPTIONS) $(LDFLAGS_EXTRA)


all: libjodycode_hint $(PROGRAM_NAME) dynamic_jc

dynamic_jc: $(PROGRAM_NAME)
	$(CC) $(CFLAGS) $(OBJS) $(BDYNAMIC) $(LDFLAGS) $(DYN_LDFLAGS) -o $(PROGRAM_NAME)$(SUFFIX)

static_jc: $(PROGRAM_NAME)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(STATIC_LDFLAGS) $(BDYNAMIC) -o $(PROGRAM_NAME)$(SUFFIX)

static: $(PROGRAM_NAME)
	$(CC) $(CFLAGS) $(OBJS) -static $(LDFLAGS) $(STATIC_LDFLAGS) -o $(PROGRAM_NAME)$(SUFFIX)

static_stripped: $(PROGRAM_NAME) static
	strip $(PROGRAM_NAME)$(SUFFIX)

$(PROGRAM_NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(BDYNAMIC) $(LDFLAGS) $(DYN_LDFLAGS) -o $(PROGRAM_NAME)$(SUFFIX)

winres.o: winres.rc winres.manifest.xml
	./tune_winres.sh
	windres winres.rc winres.o

winres_xp.o: winres_xp.rc
	./tune_winres.sh
	windres winres_xp.rc winres_xp.o

installdirs:
	test -e $(DESTDIR)$(BIN_DIR) || $(MKDIR) $(DESTDIR)$(BIN_DIR)
	test -e $(DESTDIR)$(MAN_DIR) || $(MKDIR) $(DESTDIR)$(MAN_DIR)

install: $(PROGRAM_NAME) installdirs
	$(INSTALL_PROGRAM)	$(PROGRAM_NAME)$(SUFFIX)   $(DESTDIR)$(BIN_DIR)/$(PROGRAM_NAME)$(SUFFIX)
	$(INSTALL_DATA)		$(PROGRAM_NAME).1 $(DESTDIR)$(MAN_DIR)/$(PROGRAM_NAME).$(MAN_EXT)

uninstalldirs:
	-test -e $(DESTDIR)$(BIN_DIR) && $(RMDIR) $(DESTDIR)$(BIN_DIR)
	-test -e $(DESTDIR)$(MAN_DIR) && $(RMDIR) $(DESTDIR)$(MAN_DIR)

uninstall: uninstalldirs
	$(RM)	$(DESTDIR)$(BIN_DIR)/$(PROGRAM_NAME)$(SUFFIX)
	$(RM)	$(DESTDIR)$(MAN_DIR)/$(PROGRAM_NAME).$(MAN_EXT)

test:
	./test.sh

stripped: $(PROGRAM_NAME)
	strip $(PROGRAM_NAME)$(SUFFIX)

clean:
	$(RM) $(OBJS) $(OBJS_CLEAN) build_date.h $(PROGRAM_NAME)$(SUFFIX) *~ .*.un~ *.gcno *.gcda *.gcov

distclean: clean
	$(RM) -rf *.pkg.tar* jdupes-*-*/ jdupes-*-*.zip

chrootpackage:
	+./chroot_build.sh

package:
	+./generate_packages.sh $(ARCH)

libjodycode_hint:
	$(info hint: if ../libjodycode is built but jdupes won't run, try doing 'make static_jc')
