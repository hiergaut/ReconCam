CC = g++
EXEC = a.out

OBJDIR = obj

SRC := $(wildcard *.cpp)
OBJ := $(patsubst %.cpp,%.o,$(SRC))
OBJ := $(addprefix $(OBJDIR)/, $(OBJ))
DEP := $(patsubst %.cpp,%.d,$(SRC))
DEP := $(addprefix $(OBJDIR)/, $(DEP))

# WARNING := -Wall -Wextra -Wpedantic --pedantic-errors -Weffc++
WARNING := -Wall
OPENCV := `pkg-config --cflags opencv4` `pkg-config --libs opencv4`
CXXFLAGS := -g $(OPENCV)

.PHONY: all clean

all: obj $(EXEC)

obj:
	@mkdir -v obj

clean:
	@rm -vrf $(OBJDIR) $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(WARNING) $(CXXFLAGS) $^ -o $@

-include $(DEP)

$(OBJDIR)/%.o: %.cpp Makefile
	$(CXX) $(WARNING) $(CXXFLAGS) -MMD -MP -c $< -o $@
