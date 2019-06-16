CC = g++
EXEC = a.out

OBJDIR = obj

SRC := $(wildcard *.cpp)
OBJ := $(patsubst %.cpp,%.o,$(SRC))
OBJ := $(addprefix $(OBJDIR)/, $(OBJ))
DEP := $(patsubst %.cpp,%.d,$(SRC))
DEP := $(addprefix $(OBJDIR)/, $(DEP))

# WARNING := -Wall -Wextra -Wpedantic --pedantic-errors -Weffc++
WARNING := -pedantic -Wall -Wextra -Wfatal-errors -Werror -Wshadow
# WARNING := -pedantic -Wall -Wextra -Wpedantic
OPENCV := `pkg-config --cflags opencv4` `pkg-config --libs opencv4`
CXXFLAGS := -g -pthread $(OPENCV) # -std=c++11

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
	$(CXX) $(WARNING) $(CXXFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@
