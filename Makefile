CC := clang
TARGET := model-viewer
SRC := main.c

GLFW_PREFIX := $(shell brew --prefix glfw 2>/dev/null)

CFLAGS := -std=c11 -Wall -Wextra -pedantic -Wno-newline-eof -g
DEFINES := -DGL_SILENCE_DEPRECATION
CPPFLAGS := -I$(GLFW_PREFIX)/include
LDFLAGS := -L$(GLFW_PREFIX)/lib -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

.PHONY: all run clean check-glfw

all: check-glfw $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(DEFINES) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

check-glfw:
	@test -n "$(GLFW_PREFIX)" || (echo "GLFW was not found. Install it with: brew install glfw" && exit 1)
