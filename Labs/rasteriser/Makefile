FILE=rasteriser

########
#   Directories
S_DIR=Source
B_DIR=Build

########
#   Output
EXEC=$(B_DIR)/$(FILE)

# default build settings
CC_OPTS=-c -pipe -Wall -Wno-switch -ggdb -g3 -O2
LN_OPTS=
CC=g++

########
#       SDL options 
SDL_CFLAGS := $(shell sdl2-config --cflags)
GLM_CFLAGS := -I../glm/
SDL_LDFLAGS := $(shell sdl2-config --libs)

########
#   This is the default action
all:Build


########
#   Object list
#
OBJ = $(B_DIR)/$(FILE).o
SRC_FILES := $(wildcard $(S_DIR)/*.cpp)
OBJS = $(patsubst $(S_DIR)/%.cpp,$(B_DIR)/%.o,$(SRC_FILES))


########
#   Objects
$(B_DIR)/%.o : $(S_DIR)/%.cpp
	$(CC) $(CC_OPTS) -o $@ $^ $(SDL_CFLAGS) $(GLM_CFLAGS)


########
#   Main build rule     
Build : $(OBJS) Makefile
	$(CC) $(LN_OPTS) -o $(EXEC) $(OBJS) $(SDL_LDFLAGS)


clean:
	rm -f $(B_DIR)/* 
