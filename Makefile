

SRC := $(wildcard *.c lib/*.c lib/*/*.c lib/*/*/*.c)
# OBJs are under out/ and out/lib directory
OBJ := $(patsubst %.c, out/%.o, $(SRC))
DIRS = $(subst /,/,$(sort $(dir $(OBJ))))
INC := -I/opt/homebrew/include 
LIB := -L/opt/homebrew/lib -liniparser

CC := clang
CFLAGS :=
LDFLAGS :=

EXE := out/main

.PHONY: all

all: clean out build config

# prepare out/ directory
out:
	@mkdir -p out
	@mkdir -p $(DIRS)

# build .c files to .o files
out/%.o: %.c
	@echo "[CC] $< -> $@"
	@$(CC) $(INC) $(CFLAGS) -c $< -o $@

# build sources in out/ directory
build: $(OBJ)
	@echo "[LD] $^ -> $(EXE)"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXE) $(OBJ) $(LIB)

config: 
	@echo "[CONFIG] config.ini"
	@cp config.ini out/config.ini

# clean up
clean:
	@rm -rf out