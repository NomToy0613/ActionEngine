CXX      = x86_64-w64-mingw32-g++
CXXFLAGS = -std=c++17 -Wall -MMD -MP
LDFLAGS  = -municode -mwindows -static
LIBS     = -lgdi32 -luser32
TARGET   = ActionEngine.exe
BUILDDIR = build

SRCS = $(wildcard *.cpp */*.cpp */*/*.cpp)
OBJS = $(SRCS:%.cpp=$(BUILDDIR)/%.o)

INCDIRS  = $(sort $(dir $(wildcard *.h */*.h */*/*.h)))
CXXFLAGS += $(addprefix -I,$(INCDIRS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean

clean:
	rm -rf $(BUILDDIR) $(TARGET)

-include $(OBJS:.o=.d)