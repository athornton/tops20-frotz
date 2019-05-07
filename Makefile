# Your C compiler
CC=gcc
#CC=clang

# Enable compiler warnings. This is an absolute minimum.
CFLAGS += -Wall -std=c99 #-Wextra

# If compiling on OS/X add the following
# CFLAGS += -D_DARWIN_C_SOURCE -_XOPEN_SOURCE=600

# strdup, strndup
CFLAGS += -D_POSIX_C_SOURCE=200809L

# Define your optimization flags.
#
# These are good for regular use.
#CFLAGS += -O2 -fomit-frame-pointer -falign-functions=2 -falign-loops=2 -falign-jumps=2
# These are handy for debugging.
CFLAGS += -g

# This keeps make(1) output understandable when using -j for parallel
# building If your version of make(1) can't do parallel builds, comment
# this out.
MAKEFLAGS += -Orecurse

# Define where you want Frotz installed
PREFIX ?= /usr/local
MANDIR ?= $(PREFIX)/share/man
SYSCONFDIR ?= /etc
INCLUDEDIR ?= $(PREFIX)/include
LIBDIR ?= $(PREFIX)/lib

## INCLUDEDIR path for Apple MacOS Sierra 10.12 plus MacPorts
#INCLUDEDIR ?= /opt/local/include
## LIBDIR path for Apple MacOS Sierra 10.12 plus MacPorts
#LIBDIR ?= /opt/local/lib

# Choose your sound support
# OPTIONS: ao, none
SOUND ?= ao


##########################################################################
# The configuration options below are intended mainly for older flavors
# of Unix.  For Linux, BSD, and Solaris released since 2003, you can
# ignore this section.
##########################################################################

# Default sample rate for sound effects.
# All modern sound interfaces can be expected to support 44100 Hz sample
# rates.  Earlier ones, particularly ones in Sun 4c workstations support
# only up to 8000 Hz.
SAMPLERATE ?= 44100

# Audio buffer size in frames
BUFFSIZE ?= 4096

# Default sample rate converter type
DEFAULT_CONVERTER ?= SRC_SINC_MEDIUM_QUALITY

# Comment this out if you don't want UTF-8 support
USE_UTF8 ?= yes

# The OE dipthong is not a latin1 character, but it seems the zmachine
# can handle it, and it is tested for in terpetude.  Comment this out
# if you don't want to support the OE dipthong
HANDLE_OE_DIPTHONG ?= yes

# Comment this out if your machine's version of curses doesn't support color.
COLOR ?= yes

# Select your chosen version of curses.  Unless something old is going
# on, ncursesw should be used because that's how UTF8 is supported.
#CURSES ?= curses
#CURSES ?= ncurses
CURSES ?= ncursesw

# Uncomment this if you want to disable the compilation of Blorb support.
#NO_BLORB = yes

# These are for enabling local version of certain functions which may be
# missing or behave differently from what's expected in modern system.
# If you're running on a system made in the past 20 years, you should be
# safe leaving these alone.  If not or you're using something modern,
# but very strange intended for very limited machines, you probably know
# what you're doing.  Therefore further commentary on what these
# functions do is probably not necessary.

# For missing memmove()
#NO_MEMMOVE = yes

# For missing strdup() and strndup()
#NO_STRDUP = yes

# For missing strrchr()
#NO_STRRCHR = yes


#########################################################################
# This section is where Frotz is actually built.
# Under normal circumstances, nothing in this section should be changed.
#########################################################################

CFLAGS += -I$(INCLUDEDIR)
LDFLAGS += -L$(LIBDIR)

RANLIB ?= $(shell which ranlib)
AR ?= $(shell which ar)

export CC
export CFLAGS
export CURSES_CFLAGS
export MAKEFLAGS
export AR
export RANLIB
export PREFIX
export MANDIR
export SYSCONFDIR
export INCLUDEDIR
export LIBDIR
export COLOR

NAME = frotz
VERSION = 2.45pre


# If we're working from git, we have access to proper variables. If
# not, make it clear that we're working from a release.
#
GIT_DIR ?= .git
ifneq ($(and $(wildcard $(GIT_DIR)),$(shell which git)),)
	GIT_BRANCH = $(shell git rev-parse --abbrev-ref HEAD)
	GIT_HASH = $(shell git rev-parse HEAD)
	GIT_HASH_SHORT = $(shell git rev-parse --short HEAD)
	GIT_DATE = $(shell git show -s --format=%ci)
else
	GIT_BRANCH = $(shell echo "$Format:%D$" | sed s/^.*\>\\s*//)
	GIT_HASH = "$Format:%H$"
	GIT_HASH_SHORT = "$Format:%h$"
	GIT_DATE = "$Format:%ci$"
endif
BUILD_DATE = $(shell date "+%Y-%m-%d %H:%M:%S %z")
export CFLAGS


# Compile time options handling
#
ifeq ($(CURSES), curses)
  CURSES_LDFLAGS += -lcurses
  CURSES_DEFINE = USE_CURSES_H
endif
ifeq ($(CURSES), ncurses)
  CURSES_LDFLAGS += -lncurses
  CURSES_DEFINE = USE_NCURSES_H
endif
ifeq ($(CURSES), ncursesw)
  CURSES_LDFLAGS += -lncursesw -ltinfo
  CURSES_CFLAGS += -D_XOPEN_SOURCE_EXTENDED
  CURSES_DEFINE = USE_NCURSES_H
endif

ifeq ($(SOUND), ao)
  CURSES_LDFLAGS += -lao -ldl -lpthread -lm \
	-lsndfile -lvorbisfile -lmodplug -lsamplerate
endif


# Source locations
#
SRCDIR = src
COMMON_DIR = $(SRCDIR)/common
COMMON_LIB = $(COMMON_DIR)/frotz_common.a
COMMON_STRINGS = $(COMMON_DIR)/version.c
COMMON_DEFINES = $(COMMON_DIR)/defs.h
HASH = $(COMMON_DIR)/git_hash.h

CURSES_DIR = $(SRCDIR)/curses
CURSES_LIB = $(CURSES_DIR)/frotz_curses.a
CURSES_DEFINES = $(CURSES_DIR)/ux_defines.h

DUMB_DIR = $(SRCDIR)/dumb
DUMB_LIB = $(DUMB_DIR)/frotz_dumb.a

ifndef NO_BLORB
BLORB_DIR = $(SRCDIR)/blorb
BLORB_LIB = $(BLORB_DIR)/blorblib.a
endif

SDL_DIR = $(SRCDIR)/sdl
SDL_LIB = $(SDL_DIR)/frotz_sdl.a
export SDL_PKGS = libpng libjpeg sdl2 SDL2_mixer freetype2 zlib
SDL_LDFLAGS = `pkg-config $(SDL_PKGS) --libs` -lm


SUBDIRS = $(COMMON_DIR) $(CURSES_DIR) $(SDL_DIR) $(DUMB_DIR) $(BLORB_DIR)
SUB_CLEAN = $(SUBDIRS:%=%-clean)


FROTZ_BIN = frotz$(EXTENSION)
DFROTZ_BIN = dfrotz$(EXTENSION)
SFROTZ_BIN = sfrotz$(EXTENSION)


# Build recipes
#
curses: $(FROTZ_BIN)
ncurses: $(FROTZ_BIN)
$(FROTZ_BIN): $(COMMON_LIB) $(CURSES_LIB) $(BLORB_LIB) $(COMMON_LIB)
	$(CC) $(CFLAGS) $(CURSES_CFLAGS) $+ -o $@$(EXTENSION) $(LDFLAGS) $(CURSES_LDFLAGS)
	@echo "** Done building Frotz with curses interface"

dumb: $(DFROTZ_BIN)
$(DFROTZ_BIN): $(COMMON_LIB) $(DUMB_LIB) $(BLORB_LIB) $(COMMON_LIB)
	$(CC) $(CFLAGS) $+ -o $@$(EXTENSION)
	@echo "** Done building Frotz with dumb interface."

sdl: $(SFROTZ_BIN)
$(SFROTZ_BIN): $(COMMON_LIB) $(SDL_LIB) $(BLORB_LIB) $(COMMON_LIB)
	$(CC) $(CFLAGS) $+ -o $@$(EXTENSION) $(LDFLAGS) $(SDL_LDFLAGS)
	@echo "** Done building Frotz with SDL interface."

all: $(FROTZ_BIN) $(DFROTZ_BIN) $(SFROTZ_BIN)

common_lib:	$(COMMON_LIB)
curses_lib:	$(CURSES_LIB)
sdl_lib:	$(SDL_LIB)
dumb_lib:	$(DUMB_LIB)
blorb_lib:	$(BLORB_LIB)

$(COMMON_LIB): $(COMMON_DEFINES) $(COMMON_STRINGS) $(HASH) $(COMMON_DIR);
$(CURSES_LIB): $(HASH) $(COMMON_DEFINES) $(CURSES_DEFINES) $(CURSES_DIR);
$(SDL_LIB): $(HASH) $(COMMON_DEFINES) $(SDL_DIR);
$(DUMB_LIB): $(HASH) $(COMMON_DEFINES) $(DUMB_DIR);
$(BLORB_LIB): $(BLORB_DIR);

$(SUBDIRS):
	$(MAKE) -C $@

$(SUB_CLEAN):
	-$(MAKE) -C $(@:%-clean=%) clean


# Compile-time generated defines and strings
#
common_strings:	$(COMMON_STRINGS)
$(COMMON_STRINGS):
	@echo "** Generating $@"
	@echo "#include \"frotz.h\"" > $@
	@echo "const char build_timestamp[] = \"$(BUILD_DATE)\";" >> $@

common_defines: $(COMMON_DEFINES)
$(COMMON_DEFINES):
	@echo "** Generating $@"
	@echo "#ifndef COMMON_DEFINES_H" > $@
	@echo "#define COMMON_DEFINES_H" >> $@
ifdef NO_BLORB
	@echo "#define NO_BLORB" >> $@
endif
ifdef NO_STRRCHR
	@echo "#define NO_STRRCHR" >> $@
endif
ifdef NO_MEMMOVE
	@echo "#define NO_MEMMOVE" >> $@
endif
ifdef NO_STRDUP
	@echo "#define NO_STRDUP" >> $@
endif
ifdef HANDLE_OE_DIPTHONG
	@echo "#define HANDLE_OE_DIPTHONG" >> $@
endif
ifdef USE_UTF8
	@echo "#define USE_UTF8" >> $@
endif
	@echo "#endif /* COMMON_DEFINES_H */" >> $@

curses_defines: $(CURSES_DEFINES)
$(CURSES_DEFINES):
	@echo "** Generating $@"
	@echo "#ifndef CURSES_DEFINES_H" > $@
	@echo "#define CURSES_DEFINES_H" >> $@
	@echo "#define $(CURSES_DEFINE)" >> $@
	@echo "#define CONFIG_DIR \"$(SYSCONFDIR)\"" >> $@
	@echo "#define SOUND \"$(SOUND)\"" >> $@
	@echo "#define SAMPLERATE $(SAMPLERATE)" >> $@
	@echo "#define BUFFSIZE $(BUFFSIZE)" >> $@
	@echo "#define DEFAULT_CONVERTER $(DEFAULT_CONVERTER)" >> $@
ifeq ($(SOUND), none)
	@echo "#define NO_SOUND" >> $@
endif
ifndef SOUND
	@echo "#define NO_SOUND" >> $@
endif
ifdef COLOR
	@echo "#define COLOR_SUPPORT" >> $@
endif
ifdef USE_UTF8
	@echo "#define USE_UTF8" >> $@
endif
ifdef HANDLE_OE_DIPTHONG
	@echo "#define HANDLE_OE_DIPTHONG" >> $@
endif
	@echo "#endif /* CURSES_DEFINES_H */" >> $@

hash: $(HASH)
$(HASH):
	@echo "** Generating $@"
	@echo "#define VERSION \"$(VERSION)\"" > $@
	@echo "#define GIT_BRANCH \"$(GIT_BRANCH)\"" >> $@
	@echo "#define GIT_HASH \"$(GIT_HASH)\"" >> $@
	@echo "#define GIT_HASH_SHORT \"$(GIT_HASH_SHORT)\"" >> $@
	@echo "#define GIT_DATE \"$(GIT_DATE)\"" >> $@


# Administrative stuff
#
install: install_frotz
install_frotz: $(FROTZ_BIN)
	install -d "$(DESTDIR)$(PREFIX)/bin" "$(DESTDIR)$(MANDIR)/man6"
	install "frotz$(EXTENSION)" "$(DESTDIR)$(PREFIX)/bin/"
	install -m 644 doc/frotz.6 "$(DESTDIR)$(MANDIR)/man6/"

uninstall: uninstall_frotz
uninstall_frotz:
	rm -f "$(DESTDIR)$(PREFIX)/bin/frotz"
	rm -f "$(DESTDIR)$(MANDIR)/man6/frotz.6"

install_dumb: install_dfrotz
install_dfrotz: $(DFROTZ_BIN)
	install -d "$(DESTDIR)$(PREFIX)/bin" "$(DESTDIR)$(MANDIR)/man6"
	install "$(DFROTZ_BIN)" "$(DESTDIR)$(PREFIX)/bin/"
	install -m 644 doc/dfrotz.6 "$(DESTDIR)$(MANDIR)/man6/"

uninstall_dumb: uninstall_dfrotz
uninstall_dfrotz:
	rm -f "$(DESTDIR)$(PREFIX)/bin/dfrotz"
	rm -f "$(DESTDIR)$(MANDIR)/man6/dfrotz.6"

install_sdl: install_sfrotz
install_sfrotz: $(SFROTZ_BIN)
	install -d "$(DESTDIR)$(PREFIX)/bin" "$(DESTDIR)$(MANDIR)/man6"
	install "$(SFROTZ_BIN)" "$(DESTDIR)$(PREFIX)/bin/"
	install -m 644 doc/sfrotz.6 "$(DESTDIR)$(MANDIR)/man6/"

uninstall_sdl: uninstall_sfrotz
uninstall_sfrotz:
	rm -f "$(DESTDIR)$(PREFIX)/bin/sfrotz"
	rm -f "$(DESTDIR)$(MANDIR)/man6/sfrotz.6"

install_all:	install_frotz install_dfrotz install_sfrotz

uninstall_all:	uninstall_frotz uninstall_dfrotz uninstall_sfrotz


dist: $(NAME)-$(VERSION).tar.gz
frotz-$(VERSION).tar.gz:
ifneq ($(and $(wildcard $(GIT_DIR)),$(shell which git)),)
	git archive --format=tgz --prefix $(NAME)-$(VERSION)/ HEAD -o $(NAME)-$(VERSION).tar.gz
else
	@echo "Not in a git repository or git command not found.  Cannot make a tarball."
endif

clean: $(SUB_CLEAN)
	rm -rf $(NAME)-$(VERSION)
	rm -f $(SRCDIR)/*.h \
		$(SRCDIR)/*.a \
		$(COMMON_DEFINES) \
		$(COMMON_STRINGS) \
		$(HASH) \
		$(CURSES_DEFINES) \
		$(NAME)*.tar.gz

distclean: clean
	rm -f frotz$(EXTENSION) dfrotz$(EXTENSION) sfrotz$(EXTENSION)

help:
	@echo "Targets:"
	@echo "    frotz: (default target) the standard curses edition"
	@echo "    dumb: for dumb terminals and wrapper scripts"
	@echo "    sdl: for SDL graphics and sound"
	@echo "    all: build curses, dumb, and SDL versions"
	@echo "    install"
	@echo "    uninstall"
	@echo "    install_dumb"
	@echo "    uninstall_dumb"
	@echo "    install_sdl"
	@echo "    uninstall_sdl"
	@echo "    install_all"
	@echo "    uninstall_all"
	@echo "    clean: clean up files created by compilation"
	@echo "    distclean: like clean, but also delete executables"
	@echo "    dist: create a source tarball"
	@echo ""

.SUFFIXES:
.SUFFIXES: .c .o .h

.PHONY: all clean dist curses ncurses dumb sdl hash help \
	common_defines curses_defines \
	blorb_lib common_lib curses_lib dumb_lib \
	install install_dfrotz install_sfrotz \
	$(SUBDIRS) $(SUB_CLEAN) \
	$(COMMON_DIR)/defines.h $(CURSES_DIR)/defines.h
