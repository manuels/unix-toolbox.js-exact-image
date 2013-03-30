include build/top.make

BINARY = ExactImage
BINARY_EXT = $(X_DYNEXT)
DEPS = $(lib_BINARY) $(bardecode_BINARY) $(codecs_BINARY) $(api_BINARY) $($(X_MODULE)_OUTPUT)/api-php-wrap.cc

objdir/api/php/CXXFLAGS := -I api $(PHPINCS) -fno-strict-aliasing

$($(X_MODULE)_OUTPUT)/api-php-wrap.cc: $(X_MODULE)/../api.hh $(X_MODULE)/../api-swig.hh
	$(Q)echo "  SWIG      $(dir $@)"
	$(Q)swig -php5 -c++ -outdir $(dir $@) -o '$@' $(X_MODULE)/../api-swig.hh

# we have an own install
_X_BUILD_IMPLICIT := $(_X_BUILD_IMPLICIT)
X_BUILD_IMPLICIT := 0
include build/bottom.make
X_BUILD_IMPLICIT := $(_X_BUILD_IMPLICIT)

# install
all:: $($(X_MODULE)_BINARY)
install:: $($(X_MODULE)_BINARY)
	$(Q)echo "INSTALL PHP module $^"
	$(Q)mkdir -p $(DESTDIR)$(libdir)/php/
	$(Q)install $^ $(dir $^)/ExactImage.php $(DESTDIR)$(libdir)/php/
