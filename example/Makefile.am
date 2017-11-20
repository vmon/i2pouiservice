AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I config-aux --install
AM_CXXFLAGS = --pedantic -Wall --std=c++14  -I$(srcdir)/../i2pd/libi2pd -I$(srcdir)/../i2pd/libi2pd_client -I$(srcdir)/../src
AM_LDFLAGS = -Li2pd -lm -lboost_filesystem -lboost_unit_test_framework -lboost_coroutine -lboost_thread -lboost_date_time -lboost_system -lboost_program_options  -lssl -lcrypto -lz $(PTHREAD)

noinst_PROGRAMS = i2pouichat

#TESTS = i2pouichat

i2pouichat_LDADD = \
	./../src/libi2pouiservice.a \
	./../i2pd/libi2pdclient.a \
	./../i2pd/libi2pd.a

# i2pouichat_LDFLAGS = \
# 	$(PTHREAD)

i2pouichat_SOURCES = \
	chat.cpp

