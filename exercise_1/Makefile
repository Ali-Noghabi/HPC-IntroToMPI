CC      = mpicc
CFLAGS  = -O3 -Wall -Iinclude
LDLIBS  = -lm
TARGET  = bc_norm

# sources from src/ and (if the libraries ship .c files) from include/
SRC     = $(wildcard src/*.c) $(wildcard include/*.c)
OBJ     = $(addprefix obj/,$(notdir $(SRC:.c=.o)))

vpath %.c src include

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

obj/%.o: %.c | obj
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -rf obj $(TARGET)

.PHONY: all clean
-include $(OBJ:.o=.d)