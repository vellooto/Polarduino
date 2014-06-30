Polarduino
==========

This is an Arduino sketch to control old Land Camera Polaroids.

Hardware used:

- Arduino Micro.
- Groove OLED display 128x64.
- TSL2561 Luminosity Sensor.

Libraries used:

- TSL2561.
- U8glib.


I worte this sketch with the main goal of controlling the shutter speed of an old Polaroid
Automatic Land Camera 250, but it should work with other similar models as well.

There are two modes: Auto and Manual.

Actually only manual mode is fully working, for the auto operation i've still got to
calibrate the light sensor.


Sorry if the code is bad, but i'm not a programmer and this is one of my very first C
projects.