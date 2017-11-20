AM_CXXFLAGS = --pedantic -Wall --std=c++14  -I$(srcdir)/../i2pd/libi2pd -I$(srcdir)/../i2pd/libi2pd_client 
AM_LDFLAGS = -Li2pd -lm -lboost_system -lboost_date_time -lboost_filesystem -lboost_program_options -lssl -lcrypto -lz $(PTHREAD)

TESTS = i2pouichat

i2pouichat_SOURCES = \
	chat.cpp

i2pouichat_LDADD = \
	./../src/libi2pouiservice.a \
	./../i2pd/libi2pdclient.a \
	./../i2pd/libi2pd.a

libnp1sec_test_LDFLAGS = \
	$(PTHREAD)
