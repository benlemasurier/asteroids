TARGET = asteroids
CC = gcc
#CFLAGS += -g -std=c99 -Werror -Wall -Wextra -Wunreachable-code -Wcast-qual -Wshadow -Wpointer-arith -Wstrict-prototypes -pedantic $(shell pkg-config --cflags allegro-5.0 allegro_image-5.0 allegro_font-5.0 allegro_ttf-5.0)
CFLAGS += -g -std=c99 -Wall -Wextra -Wunreachable-code -Wcast-qual -Wshadow -Wpointer-arith -Wstrict-prototypes -pedantic $(shell pkg-config --cflags allegro-5.0 allegro_image-5.0 allegro_font-5.0 allegro_ttf-5.0 allegro_audio-5.0 allegro_acodec-5.0)
LIBS += $(shell pkg-config --cflags --libs allegro-5.0 allegro_image-5.0 allegro_font-5.0 allegro_ttf-5.0 allegro_audio-5.0 allegro_acodec-5.0)
UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	CFLAGS += $(shell pkg-config --cflags allegro_main-5.0)
	LIBS += $(shell pkg-config --libs allegro_main-5.0)
endif


.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
