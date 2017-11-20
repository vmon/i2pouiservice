# whatever flags you want to pass to the C compiler & linker
ACLOCAL_AMFLAGS = -I m4 -I config-aux --install

AM_CXXFLAGS = --pedantic -Wall --std=c++14  -I$(srcdir)/../i2pd/libi2pd -I$(srcdir)/../i2pd/libi2pd_client 
AM_LDFLAGS = -Li2pd -lm -lboost_system -lboost_date_time -lboost_filesystem -lboost_program_options -lssl -lcrypto -lz $(PTHREAD)

get_slash_LDADD = ./../i2pd/libi2pdclient.a ./../i2pd/libi2pd.a 

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = get_slash
get_slash_SOURCES = get_slash.cpp 

noinst_LIBRARIES=libi2pouiservice.a

libi2pouiservice_a_SOURCES = \
	i2pouichannel.cpp \
	service.cpp
