asteroids
=========

an attempt at an authentic asteroids clone


building
========

You'll need the 5.0 release of Allegro, availble here: http://alleg.sourceforge.net/download.html

then:

    gcc -Wall -std=c99 -pedantic asteroids.c -o asteroids $(pkg-config --cflags --libs allegro-5.0 allegro_image-5.0 allegro_font-5.0 allegro_ttf-5.0)

*note:* if you're building on OSX, you'll need to add `allegro_main-5.0` to the pkg-config variables



legal
=======

I'm probably breaking all kinds of rules just calling this asteroids.
There is no commercial interest here, I just like asteroids and this is a great way to learn the allegro game library.
Don't hurt me :(


contributing
============

patches welcome! compile with the following flags before submitting a request:
    -std=c99
    -Wall
    -Wextra
    -Werror
    -Wunreachable-code
    -Wcast-qual
    -Wshadow
    -Wpointer-arith
    -Wstrict-prototypes
    -pedantic

