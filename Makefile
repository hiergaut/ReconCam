# CC = g++
EXEC = a.out

# OBJDIR = obj

SRC := $(wildcard *.cpp)
OBJ := $(patsubst %.cpp,%.o,$(SRC))
# OBJ := $(addprefix $(OBJDIR)/, $(OBJ))
# DEP := $(patsubst %.cpp,%.d,$(SRC))
# DEP := $(addprefix $(OBJDIR)/, $(DEP))

# DEBUG ?= 1
# ifeq (DEBUG, 1)
# CFLAGS = -g3 -gdwarf2 -DDEBUG
# CFLAGS = -g3 -gdwarf-3 -DDEBUG
# else
CFLAGS= -Ofast -DNDEBUG -DPROJECT_DIR="" -DTINY_YOLO
# CFLAGS= -Ofast -DNDEBUG -DPROJECT_DIR=""

# endif

CXX = g++ $(CFLAGS)
CC = gcc $(CFLAGS)

LIBRARIES = -pthread `pkg-config --cflags opencv4` `pkg-config --libs opencv4`

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) -o $@ $^ $(LIBRARIES)

%.o: %.cpp
	$(CXX) -c $< $(LIBRARIES)

clean:
	@rm -vrf $(OBJ) $(EXEC)


# WARNING := -Wall -Wextra -Wpedantic --pedantic-errors -Weffc++
#WARNING := -pedantic -Wall -Wextra -Wfatal-errors -Werror -Wshadow
# WARNING := -pedantic -Wall -Wextra -Wpedantic
# OPENCV := `pkg-config --cflags opencv4` `pkg-config --libs opencv4`
# CXXFLAGS := -g -pthread $(OPENCV)  # -std=c++11
#
# .PHONY: all clean
#
# all: obj $(EXEC)
#
# obj:
#     @mkdir -v obj
#
# clean:
#     @rm -vrf $(OBJDIR) $(EXEC)
#
# $(EXEC): $(OBJ)
#     $(CXX) $(WARNING) $^ -o $@ $(CXXFLAGS)
#
# # -include $(DEP)
#
# $(OBJDIR)/%.o: %.cpp Makefile
#     $(CXX) $(WARNING) $(CXXFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@
#
#CPPFLAGS=-g -pthread -I/sw/include/root


