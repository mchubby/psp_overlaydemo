/*--------------
  cmlibMenu
--------------*/
Author: Team Otwibaco
project: http://code.google.com/p/prx-common-libraries/

[Introduction]
 cmlibMenu is a common library for PSP. It provide a "Draw" and "create Menu" to your program.
 cmlibMenu based Libmenu.

[Feature]
 - Draw Ascii and Japanese(sjis) 8x8 bitmap font.
 - Draw 2bit color icon.
 - Can draw 16x16 mode(zoom).
 - Draw line, circle and frame.
 - Support alpha blending.
 - Can make a menu easily.
 - Support RadioButton, CheckBox, SelectBox, UpDownControl, Spacer, TriggerButton.

[Module information]
 file name: cmlibmenu.prx
 module name: cmlibMenu
 library name: cmlibMenu

[How to use]
1.Add in your source code,

 #include "cmlibmenu.h"

2.Your program check whether the cmlibMenu is loaded. If necessary load the cmlibMenu.
 (sample: checkmod.c)

3.Add in your Makefile,

 LIBS += libcmlibMenu.a

4.Copy "lib" to /seplugins.

[License]
 GNU GENERAL PUBLIC LICENSE Version 2

[Thanks]
 maxem: Author of Libmenu.
