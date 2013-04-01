include config.make

X_OUTARCH := ./objdir

X_BUILD_IMPLICIT=0
include utility/Makefile
X_BUILD_IMPLICIT=1

# -s silcently corrupts binaries on OS X, sigh -ReneR
CFLAGS = -Wall -O2
CXXFLAGS = -Wall -O2 -Wno-sign-compare
#CFLAGS = -Wall -O0 -ggdb
#CXXFLAGS = -Wall -O0 -ggdb

# for config.h
CPPFLAGS += -I . -I ../antigraingeometry/src/include
CPPFLAGS += -I ../libjpeg/src
CPPFLAGS += -DHAVEJPEG=1 -D_MSC_VER -DWITHLIBJPEG=1

# -frename-registers and -funroll-loops brings a lot performance on
# my AMD Turion - about 20% time decrease (though it is included in -funroll-loops anyway) !!!

# from the linux-kernel build system:
cc-option = $(shell if $(CC) $(1) -S -o /dev/null -xc /dev/null \
            > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

ifeq "$(X_ARCH)" "i686"
CXXFLAGS += -march=i686
CXXFLAGS += $(call cc-option,-mtune=pentium4,)
endif

# TODO: improve to match i[3456]86
ifneq  "$(X_ARCH)" "i686"
CFLAGS += -fPIC
CXXFLAGS += -fPIC
endif

ifeq "$(X_ARCH)" "sparc64"
CXXFLAGS += -mcpu=ultrasparc
CXXFLAGS += $(call cc-option,-mtune=niagara,)
endif

CXXFLAGS += -funroll-loops -fomit-frame-pointer
CXXFLAGS += $(call cc-option,-funswitch-loops,)
CXXFLAGS += $(call cc-option,-fpeel-loops,)
CXXFLAGS += $(call cc-option,-ftracer,)
CXXFLAGS += $(call cc-option,-funit-at-a-time,)
CXXFLAGS += $(call cc-option,-frename-registers,)
CXXFLAGS += $(call cc-option,-ftree-vectorize,)

#CXXFLAGS += $(call cc-option,-mfpmath=sse,)

# we have some unimplemented colorspaces in the Image::iterator :-(
CXXFLAGS += $(call cc-option,-Wno-switch -Wno-switch-enum,)

ifeq "$(STATIC)" "1"
X_EXEFLAGS += -static
endif

MODULES = lib codecs bardecode frontends ContourMatching
include $(addsuffix /Makefile,$(MODULES))

ifeq "$(WITHX11)" "1"
ifeq "$(WITHEVAS)" "1"
include gfx/Makefile
include edisplay/Makefile
endif
endif

ifeq "$(WITHSWIG)" "1"
include api/Makefile

ifeq "$(WITHLUA)" "1"
include api/lua/Makefile
endif
ifeq "$(WITHPERL)" "1"
include api/perl/Makefile
endif
ifeq "$(WITHPHP)" "1"
include api/php/Makefile
endif
ifeq "$(WITHPYTHON)" "1"
include api/python/Makefile
endif
ifeq "$(WITHRUBY)" "1"
#include api/ruby/Makefile
endif

endif

check: $(X_OUTARCH)/econvert/econvert$(X_EXEEXT) $(X_OUTARCH)/edentify/edentify$(X_EXEEXT)
	$(Q)cd testsuite; ./run ../$(X_OUTARCH)/econvert/econvert$(X_EXEEXT)
