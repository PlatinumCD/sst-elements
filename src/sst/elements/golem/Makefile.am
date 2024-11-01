# -*- Makefile -*-
#
#

AM_CPPFLAGS += -fmax-errors=1 \
	-I$(top_srcdir)/src \
	$(MPI_CPPFLAGS)

compdir = $(pkglibdir)
comp_LTLIBRARIES = libgolem.la
libgolem_la_SOURCES = \
	golem.cc \
	array/computeArray.h \
	array/emulatedComputeArray.h \
	array/emulatedFloatArray.h \
	array/emulatedIntArray.h \
	array/crossSimComputeArray.h \
	array/crossSimFloatArray.h \
	rocc/roccAnalog.h \
	rocc/roccAnalogFloat.h \
	rocc/roccAnalogInt.h


#EXTRA_DIST = \
#	tests/mmioArrayTest.py
#	floatarray/mmioTile.h \
#	floatarray/mmioTile.cc

libgolem_la_LDFLAGS = -module -avoid-version

install-exec-hook:
	$(SST_REGISTER_TOOL) SST_ELEMENT_SOURCE     golem=$(abs_srcdir)
#	$(SST_REGISTER_TOOL) SST_ELEMENT_TESTS      golem=$(abs_srcdir)/tests

##########################################################################
##########################################################################
##########################################################################
