OBJS=run_tests.o socket_server_test.o
SOCKET_SERVER_LOC=..
CPPUNIT_CONTRIB=${HOME}/sandbox/cppunit_contrib
EXE=run_cppunit

CFLAGS += -g --std=c++11 -I${SOCKET_SERVER_LOC} -I${CPPUNIT_CONTRIB} -pthread
LFLAGS = -lcppunit -lpthread

.cpp.o:
	g++ -c ${CFLAGS} $<

all: ${OBJS}
	g++ -o ${EXE} ${OBJS} ${LFLAGS}

clean:
	rm -f *~ ${EXE} ${OBJS} core
