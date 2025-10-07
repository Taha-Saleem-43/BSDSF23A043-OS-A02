# ==============================
# Compiler and Flags
# ==============================
CC = gcc
CFLAGS = -Wall

# ==============================
# Directories
# ==============================
SRC  = src
OBJ  = obj
BIN  = bin

# ==============================
# Targets
# ==============================
TARGET = $(BIN)/lsv1.3.0
SRC_FILE = $(SRC)/lsv1.3.0.c
OBJ_FILE = $(OBJ)/lsv1.3.0.o

# ==============================
# Default target
# ==============================
# ==============================
all: $(TARGET)

# Link object file into final executable
$(TARGET): $(OBJ_FILE)
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "✅ Build successful! Executable: $(TARGET)"

# Compile .c file into .o file
$(OBJ)/lsv1.3.0.o: $(SRC)/lsv1.3.0.c
	@mkdir -p $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# ==============================
# Utility targets
# ==============================
run: all
	./$(TARGET)

clean:
	rm -rf $(OBJ)/*.o $(BIN)/*

.PHONY: all clean run

