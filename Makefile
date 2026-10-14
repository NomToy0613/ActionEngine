CXX      = x86_64-w64-mingw32-g++
CXXFLAGS = -std=c++17 -Wall -MMD -MP
LDFLAGS  = -municode -mwindows -static
LIBS     = -lgdi32 -luser32
TARGET   = ActionEngine.exe

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGET)

-include $(OBJS:.o=.d)