About
=====
overlaydemo is a PRX seplugin that shows how to display items as an overlay over regular framebuffer output.
It is a kernel-mode module, and therefore requires a custom PSP firmware (CFW) in version 5.00 or higher.

How to compile
==============
A PSP toolchain is required, type make to compile overlaydemo.prx.

Tested on:
Minimalist PSPSDK (pspsdk-setup-0.11.1.exe)

How to use
==========

Extract seplugins folder contents locally.
Copy to the root PSP:\seplugins\
A pre-populated game.txt is provided for guidance, you may append the line if you have an existing one, pops/vsh etc.

To disable the plugin, remove the relevant line from game.txt.


cmlibMenu is Copyright (c) Team Otwibaco and maxem
project: http://code.google.com/p/prx-common-libraries/
[License]
 GNU GENERAL PUBLIC LICENSE Version 2
