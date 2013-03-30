include build/top.make

BINARY = ExactImage
BINARY_EXT = $(X_DYNEXT)
DEPS = $(lib_BINARY) $(bardecode_BINARY) $(codecs_BINARY) $(api_BINARY) $($(X_MODULE)_OUTPUT)/api-perl-wrap.cc

objdir/api/perl/CXXFLAGS := -I api $(PERLINCS)

$($(X_MODULE)_OUTPUT)/api-perl-wrap.cc: $(X_MODULE)/../api.hh $(X_MODULE)/../api-swig.hh
	$(Q)echo "  SWIG      $(dir $@)"
	$(Q)swig -perl -c++ -outdir $(dir $@) -o '$@' $(X_MODULE)/../api-swig.hh

# we have an own install
_X_BUILD_IMPLICIT := $(_X_BUILD_IMPLICIT)
X_BUILD_IMPLICIT := 0
include build/bottom.make
X_BUILD_IMPLICIT := $(_X_BUILD_IMPLICIT)

# install
all:: $($(X_MODULE)_BINARY)
install:: $($(X_MODULE)_BINARY)
	$(Q)echo "INSTALL PERL module $^"
	$(Q)mkdir -p $(DESTDIR)$(libdir)/perl5/site_perl/
	$(Q)install $^ $(dir $^)/ExactImage.pm $(DESTDIR)$(libdir)/perl5/site_perl/
