AUXILIARY_SOURCES := thread_control_block.cpp dispatcher.cpp scheduler.cpp utils.cpp globals.cpp
AUXILIARY_HEADERS := ${AUXILIARY_SOURCES:.cpp=.h}
SOURCES := uthreads.cpp $(AUXILIARY_SOURCES)
LIBRARY := libuthreads.a
OBJECTS := ${SOURCES:.cpp=.o}
TARGET := uthreads

CXX := g++
AR := ar
RANLIB := ranlib
CXXFLAGS := -Wextra -Wall -std=c++11 -g
CLANG_FORMAT_FLAGS := --style=Mozilla -i
FILES_TO_SUBMIT := Makefile README $(AUXILIARY_SOURCES) $(AUXILIARY_HEADERS) uthreads.cpp
SUBMISION_NAME := ex2
SUBMISION_FILE := ${SUBMISION_NAME:=.tar}

.PHONY: all clean format tar

$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@  $^
	$(RANLIB) $@

all: basic_test.o $(LIBRARY)
	$(CXX) $^ -o $(TARGET)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) main.o $(TARGET) $(LIBRARY) $(SUBMISION_FILE) basic_test.o

format:
	clang-format $(CLANG_FORMAT_FLAGS) $(SOURCES)

tar: $(FILES_TO_SUBMIT)
	tar cf $(SUBMISION_FILE) $(FILES_TO_SUBMIT)
