CXX			= c++
CXXFLAGS	= -std=c++17 -Wall
TARGET		= test_basic
SRCS		= ../tester/test_basic.cpp
INCLUDES	= -I../tester

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
