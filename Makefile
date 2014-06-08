COMPILER=g++
RUNSTRING=./${TARGET}
OBJECTS=main.o soundengine.o
LIBS=-ljack -lpthread
FLAGS=-g -std=c++11 -Ofast 

TARGET=generic-audio


all: .depend ${TARGET}

#Calculating dependincies
.depend: $(wildcard ./*.cpp ./*.h) Makefile
	$(CXX) $(CFLAGS) -MM *.cpp > ./.dependtmp
	cat ./.dependtmp | sed 's/h$$/h \n\t \$(CXX) -c $(FLAGS) $$< -o $$@/' > ./.depend
	rm ./.dependtmp

${TARGET}: ${OBJECTS} #cleancpp
	${COMPILER} ${FLAGS} -o ${TARGET} ${OBJECTS} ${LIBS}

include .depend

#För att kunna köra filen direkt
run: ${TARGET}
	${RUNSTRING}

clean:
	rm *.o
	rm .depend

rebuild: clean ${TARGET}
