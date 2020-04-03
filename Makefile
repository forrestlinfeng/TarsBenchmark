#----------------------------------------------------------------------------------------------------------
TARGET		:= eab
CC          := gcc
CXX         := g++
CFLAGS      += -g -fPIC -Wall -pipe -fno-ident -fno-strict-aliasing -MMD -D_GNU_SOURCE -D_REENTRANT
CPPFLAGS    += -g -fPIC -Wno-deprecated -Wall -pipe -fno-ident -fno-strict-aliasing -MMD -D_GNU_SOURCE -D_REENTRANT -std=gnu++11

#----------------------------------------------------------------------------------------------------------
LOCAL_SRC   += $(sort $(wildcard *.cpp *.cc *.c))
LOCAL_SRC   += $(sort $(wildcard proto/*.cpp contrib/util/*.cpp contrib/licote/*.c))
LOCAL_OBJ 	+= $(patsubst %.cpp,%.o, $(patsubst %.cc,%.o, $(patsubst %.c,%.o, $(LOCAL_SRC))))
DEP_FILE    := $(foreach obj, $(LOCAL_OBJ), $(dir $(obj))$(basename $(notdir $(obj))).d)

INCLUDE     += -I. -I./proto/ -I./contrib/ -I./contrib/licote/include
#----------------------------------------------------------------------------------------------------------

LIB_32 		+= -lpthread -lrt
LIB_64 		+= -lpthread -lrt

DEP_FILE_32     := $(foreach obj, $(DEP_FILE),$(patsubst %.d,%.32.d, $(obj)))
DEP_FILE_64     := $(foreach obj, $(DEP_FILE),$(patsubst %.d,%.64.d, $(obj)))

LOCAL_OBJ_32    := $(foreach obj, $(LOCAL_OBJ),$(patsubst %.o,%.32.o,$(obj)))
LOCAL_OBJ_64    := $(foreach obj, $(LOCAL_OBJ),$(patsubst %.o,%.64.o,$(obj)))
CLEANFILE       := $(LOCAL_OBJ_32) $(LOCAL_OBJ_64)

PLATFORM := $(strip $(shell echo `uname -m`))
ifneq ($(MFLAGS),64)
	ifneq ($(MFLAGS),32)
		ifeq ($(PLATFORM),x86_64)
			MFLAGS := 64
		else
			MFLAGS := 32
		endif
	endif
else
	CFLAGS += -D_SYS_EPOLL_
endif

ifeq ($(MFLAGS),64)
    DEP_FILE    := $(DEP_FILE_64)
    LOCAL_OBJ   := $(LOCAL_OBJ_64)
    LIB         := $(LIB) $(LIB_64)
else
    DEP_FILE    := $(DEP_FILE_32)
    LOCAL_OBJ   := $(LOCAL_OBJ_32)
    LIB         := $(LIB) $(LIB_32)
endif

#-------------------------------------------------------------------------------
all : $(LOCAL_OBJ) $(TARGET)

clean:
	rm -vf $(LOCAL_OBJ) $(TARGET) ${CLEANFILE}
cleanall:
	rm -vf $(LOCAL_OBJ) $(TARGET) $(DEP_FILE) ${CLEANFILE} *.o *.d *.tgz *.d.tmp

$(filter %.a, $(TARGET)) : $(LOCAL_OBJ)
	ar r $@ $(LOCAL_OBJ)

$(filter-out %.so %.a, $(TARGET)) : $(LOCAL_OBJ)
	$(CXX) -m$(MFLAGS) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIB)
#-------------------------------------------------------------------------------
%.32.o: %.cpp
	$(CXX) -m32 $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

%.32.o: %.cc
	$(CXX) -m32 $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

%.32.o: %.c
	$(CC) -m32 $(CFLAGS) $(INCLUDE) -o $@ -c $<
#-------------------------------------------------------------------------------
%.64.o: %.cpp
	$(CXX) -m64 $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

%.64.o: %.cc
	$(CXX) -m64 $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

%.64.o: %.c
	$(CC) -m64 $(CFLAGS) $(INCLUDE) -o $@ -c $<
#----------------------------------------------------------------------------------
%.o: %.cpp
	$(CXX) -m$(MFLAGS) $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

%.o: %.cc
	$(CXX) -m$(MFLAGS) $(CPPFLAGS) $(INCLUDE) -o $@ -c $<

%.o: %.c
	$(CC) -m$(MFLAGS) $(CFLAGS) $(INCLUDE) -o $@ -c $<
#----------------------------------------------------------------------------------
