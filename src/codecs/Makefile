include build/top.make

ifeq "$(WITHLIBJPEG)" "1"
LDFLAGS += -ljpeg
else
NOT_SRCS += jpeg.cc transupp.c
endif

ifeq "$(WITHLIBTIFF)" "1"
LDFLAGS += -ltiff # -ltiffxx
else
NOT_SRCS += tiff.cc
endif

ifeq "$(WITHLIBPNG)" "1"
LDFLAGS += -lpng
else
NOT_SRCS += png.cc
endif

ifeq "$(WITHLIBUNGIF)" "1"
LDFLAGS += -lungif
else
NOT_SRCS += gif.cc
endif

ifeq "$(WITHJASPER)" "1"
LDFLAGS += -ljasper
else
NOT_SRCS += jpeg2000.cc
endif

ifeq "$(WITHOPENEXR)" "1"
CPPFLAGS += $(OPENEXRINCS)
LDFLAGS += $(OPENEXRLIBS)
else
NOT_SRCS += openexr.cc
endif

ifeq "$(WITHLCMS)" "1"
LDFLAGS += $(LCMSLIBS)
endif

ifeq "$(WITHEXPAT)" "1"
LDFLAGS += $(EXPATLIBS)
else
NOT_SRCS += agg_svg_parser.cc agg_svg_path_renderer.cc \
            agg_svg_path_tokenizer.cc svg.cc
endif

ifeq "$(PCX)" "0"
NOT_SRCS += pcx.cc
endif

ifeq "$(TGA)" "0"
NOT_SRCS += tga.cc
endif 

BINARY = lib
BINARY_EXT = $(X_LIBEXT)
DEPS =

CPPFLAGS += -I codecs/
LDFLAGS += -lz # for the PDF compression, TODO: check for availability and disable the support code otherwise

include build/bottom.make
