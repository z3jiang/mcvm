
INCLUDE = -Ivendor/include -Ilib/include -Iinclude
LLVMLIBS = $(shell vendor/bin/llvm-config --libfiles)

SRCS =	$(shell find source -name '*.cpp')
OBJS =	$(SRCS:%.cpp=%.o)

CXX = g++
CXXFLAGS = $(INCLUDE) -Wall -fmessage-length=0 -g -std=c++11 -Wno-deprecated
CXXFLAGS += -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -DMCVM_USE_JIT -DMCVM_USE_LAPACKE
#CXXFLAGS += -DNDEBUG

# turning off heartbeat makes debugging easier
CXXFLAGS += -DMCVM_NO_HEARTBEAT

LIBS = vendor/lib/libgccpp.a  vendor/lib/libgc.a 
LIBS += -pthread -ldl -llapacke -lcblas
  

TARGET = mcvm

%.o : %.cpp
	g++ $(CXXFLAGS) $(INCLUDES) -MD -c -o $@ $<
	@cp $*.d $*.P; \
        sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
            -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
        rm -f $*.d

-include *.P

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LLVMLIBS) $(LIBS)
 
all: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TOBJS)
	rm -rf *.P
	
