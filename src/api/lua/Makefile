include build/top.make

BINARY = ExactImage
BINARY_EXT = $(X_DYNEXT)
DEPS = $(lib_BINARY) $(bardecode_BINARY) $(codecs_BINARY) $(api_BINARY) $($(X_MODULE)_OUTPUT)/api-lua-wrap.cc

objdir/api/lua/CXXFLAGS := -I api $(LUAINCS) -fno-strict-aliasing

$($(X_MODULE)_OUTPUT)/api-lua-wrap.cc: $(X_MODULE)/../api.hh $(X_MODULE)/../api-swig.hh
	$(Q)echo "  SWIG      $(dir $@)"
	$(Q)swig -lua -c++ -outdir $(dir $@) -o '$@' $(X_MODULE)/../api-swig.hh

# we have an own install
_X_BUILD_IMPLICIT := $(_X_BUILD_IMPLICIT)
X_BUILD_IMPLICIT := 0
include build/bottom.make
X_BUILD_IMPLICIT := $(_X_BUILD_IMPLICIT)

api/lua/libdir := $(shell pkg-config lua --variable INSTALL_CMOD)

# install
all:: $($(X_MODULE)_BINARY)
install:: $($(X_MODULE)_BINARY)
	$(Q)echo "INSTALL LUA module $^"
	$(Q)mkdir -p $(DESTDIR)$(api/lua/libdir)
	$(Q)install $^ $(DESTDIR)$(api/lua/libdir)
