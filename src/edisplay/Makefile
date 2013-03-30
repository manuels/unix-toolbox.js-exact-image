
include build/top.make

BINARY = edisplay
BINARY_EXT = $(X_EXEEXT)

CPPFLAGS += -I . -I lib -I gfx
CPPFLAGS +=  $(shell pkg-config evas --atleast-version 0.9.9.050 && echo -D WITHEVAS_X11_CONNECTION)
CPPFLAGS +=  $(shell pkg-config evas --atleast-version 0.9.9.49898 && echo -D WITHEVAS_X11_SCREEN)

DEPS += $(X_OUTARCH)/gfx/X11Helper$(X_OBJEXT) $(X_OUTARCH)/gfx/EvasHelper$(X_OBJEXT) $(X_OUTARCH)/utility/Timer$(X_OBJEXT) $(X_OUTARCH)/utility/ArgumentList$(X_OBJEXT) $(lib_BINARY) $(codecs_BINARY)

include build/bottom.make

