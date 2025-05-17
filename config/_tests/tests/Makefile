CXX			= g++
CXXFLAGS	= -std=c++17
LIBS		= -lcurl -lssl -lcrypto
TARGET		= test_basic
SRCS		= ../tester/test_basic.cpp
INCLUDES	= -I../tester

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)