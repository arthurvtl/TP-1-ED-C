# Detect OS for mkdir
ifeq ($(OS),Windows_NT)
    MKDIR_P = mkdir
else
    MKDIR_P = mkdir -p
endif

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -O2
INCLUDES = -Iinclude
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
TARGET = $(BIN_DIR)/tp_parte1

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/bd_times.c $(SRC_DIR)/bd_partidas.c $(SRC_DIR)/utils.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	-$(MKDIR_P) $(OBJ_DIR)
	-$(MKDIR_P) $(BIN_DIR)

run: all
	@$(TARGET) $(ARGS)

debug: CFLAGS += -g
debug: clean all

clean:
	-$(RM) -r $(OBJ_DIR) $(BIN_DIR)