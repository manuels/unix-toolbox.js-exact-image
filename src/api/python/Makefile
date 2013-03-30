include build/top.make

BINARY = _ExactImage
BINARY_EXT = $(X_DYNEXT)
DEPS = $(lib_BINARY) $(bardecode_BINARY) $(codecs_BINARY) $(api_BINARY) $($(X_MODULE)_OUTPUT)/api-python-wrap.cc

objdir/api/python/CXXFLAGS := -I api $(PYTHONINCS)

$($(X_MODULE)_OUTPUT)/api-python-wrap.cc: $(X_MODULE)/../api.hh $(X_MODULE)/../api-swig.hh
	$(Q)echo "  SWIG      $(dir $@)"
	$(Q)swig -python -c++ -outdir $(dir $@) -o '$@' $(X_MODULE)/../api-swig.hh

# we have an own install
_X_BUILD_IMPLICIT := $(_X_BUILD_IMPLICIT)
X_BUILD_IMPLICIT := 0
include build/bottom.make
X_BUILD_IMPLICIT := $(_X_BUILD_IMPLICIT)

PYTHON_LIBDIR := $(shell python -c 'from distutils.sysconfig import get_python_lib; print get_python_lib()')

# install
all:: $($(X_MODULE)_BINARY)
install:: $($(X_MODULE)_BINARY)
	$(Q)echo "INSTALL PYTHON module $^"
	$(Q)mkdir -p $(DESTDIR)$(PYTHON_LIBDIR)
	$(Q)install $(dir $^)/ExactImage.py $^ $(DESTDIR)$(PYTHON_LIBDIR)/
