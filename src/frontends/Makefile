include build/top.make

ifneq "$(WITHLIBTIFF)" "1"
NOT_SRCS += e2mtiff.cc
endif

# build each .cc file as executable - if this is not desired
# in the future a list must be supplied manually ...

BINARY = $(basename $(filter-out $(NOT_SRCS), $(notdir $(wildcard $(X_MODULE)/*.cc $(X_MODULE)/*.c))) $(SRCS))

BINARY_EXT = $(X_EXEEXT)
DEPS = $(lib_BINARY) $(codecs_BINARY) $(bardecode_BINARY) $(X_OUTARCH)/utility/ArgumentList$(X_OBJEXT) $(X_OUTARCH)/utility/File$(X_OBJEXT)

CPPFLAGS += -I utility

include build/bottom.make
