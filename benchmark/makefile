#programe name
PRGM=bench_test

#VPATH =../../Common/proto

#wildcard : ��չͨ��� ��notdir ��ȥ��·�� patsubst ���滻ͨ���
CPP_SRC=$(wildcard ./*.cpp)
CPP_OBJ=$(patsubst %.cpp, %.o, $(notdir ${CPP_SRC}))

#PB_SRC=base.pb.cc trade_db_model.pb.cc basis.pb.cc quota.pb.cc safe.pb.cc etf_signal.pb.cc pq_quota.pb.cc \
	   future_trade.pb.cc stock_trade.pb.cc back_management.pb.cc position_graph.pb.cc \
	   error.pb.cc authorize.pb.cc
#PB_OBJ=$(patsubst %.cc, %.o, $(notdir ${PB_SRC}))

OBJS=$(CPP_OBJ) #$(PB_OBJ)


######################debug release#########################
#debug
CCFLAGS=-g -O3  -mavx2
LIBINC=-L/usr/lib/boost/ -L/usr/local/lib/
BINDIR=./debug

#release
#CCFLAGS=-g -O0
#LIBINC=-L/usr/lib64/mysql -L/usr/lib/boost/ -L../../bin/release/ -L/usr/local/lib/ -L.
#BINDIR=../../bin/release
######################debug release#########################

#LIBS=-lmysqlclient -lpthread -lz -lm -lssl -lcrypto -ldl -lprotobuf -lprotobuf-lite -lprotoc -lboost_log_setup -lboost_log -lboost_date_time \
	-lboost_system -lboost_filesystem -lpthread -lboost_thread -lboost_program_options \
	-lboost_locale

LIBS=-lpthread  -lfolly -lglog -lgflags -lfollybenchmark -lboost_system -lboost_filesystem

#INC=-I./ -I../../ -I../../Common/ground/ground_1_0_1 -I../../Common/proto -I../../Common/include -I$(HOME)/include
#INC = -I../../Common/boostex -I../../Common/include -I../../comm/linux-pb  -I../../comm/include
INC=-I. -I../
#����release�汾ִ������make R=-O2
#RELEASEFLAG=$R
#ifndef R
#CCFLAGS=-g
#else
#CCFLAGS=
#endif


#CCFLAGS += -c $(RELEASEFLAG) -m64 -std=c++11 -DBOOST_ALL_DYN_LINK -DBOOST_LOG_DYN_LINK -DGROUND_ENABLE_MULTI_THREAD -DGROUND_ENABLE_FILE_LOG
CCFLAGS += -c -Wall -m64 -std=c++1z
CCFLAGS2 = -Wall -m64 -std=c++1z -msse4.2 -O3

SCPP=g++ $(CCFLAGS)
SCPP2 = g++ $(CCFLAGS2)
#SCPP=gcc $(CCFLAGS)
SCLK=g++
#SCLK=gcc

all:$(OBJS)
	$(SCPP) -o  $(PRGM)   $(OBJS)  $(LIBINC) $(LIBS)
	$(SCPP2) -o $(PRGM)  $(OBJS)  $(LIBINC) $(LIBS)

clean:
	rm *.o $(PRGM)

.SUFFIXES: .cpp .cc .o

.cpp.o:
	$(SCPP) $(INC) $< -o $@

.cc.o:
	$(SCPP) $(INC) $<

$(PRGM):$(OBJS)
	$(SCLK) -o $(PRGM) $(OBJS) $(LIBINC) $(LIBS)
#	mv $(PRGM) $(BINDIR)
