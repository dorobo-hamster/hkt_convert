AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = hkt_convert
hkt_convert_SOURCES = src/hkt_convert.cpp

#AM_CXXFLAGS = -Wall -Wl,-enable-auto-import -ffunction-sections -fdata-sections -s
#AM_LDFLAGS = -static-libgcc -static-libstdc++ -Wl,--gc-sections

AM_CXXFLAGS = -Wall -Wl,-enable-auto-import -s
AM_LDFLAGS = -static-libgcc -static-libstdc++

dist_noinst_SCRIPTS = autogen.sh