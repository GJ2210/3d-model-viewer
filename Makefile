CC := clang
TARGET := model-viewer
SRC := main.c model.c renderer.c

GLFW_PREFIX := $(shell brew --prefix glfw 2>/dev/null)

CFLAGS := -std=c11 -Wall -Wextra -pedantic -Wno-newline-eof -g
CPPFLAGS := -I$(GLFW_PREFIX)/include
LDFLAGS := -L$(GLFW_PREFIX)/lib -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -lm

.PHONY: all run clean check-glfw

all: check-glfw $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: all
	./$(TARGET)

run-file: all
	@test -n "$(MODEL)" || (echo "Usage: make run-file MODEL=/path/to/model.obj" && exit 1)
	./$(TARGET) "$(MODEL)"

clean:
	rm -rf $(TARGET) $(TARGET).dSYM

check-glfw:
	@test -n "$(GLFW_PREFIX)" || (echo "GLFW was not found. Install it with: brew install glfw" && exit 1)
