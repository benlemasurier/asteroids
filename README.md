asteroids
=========

an attempt at an authentic asteroids clone


building
========

You'll need the 5.0 release of Allegro, availble here: http://alleg.sourceforge.net/download.html

then:

    gcc asteroids.c -o asteroids $(pkg-config --cflags --libs allegro-5.0 allegro_image-5.0)



legal
=======

I'm probably breaking all kinds of rules just calling this asteroids.
There is no commercial interest here, I just like asteroids and this is a great way to learn the allegro game library.
Don't hurt me :(
