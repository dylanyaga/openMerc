@echo off
echo Compiling Source
C:\bcc\bin\bcc32.exe -IC:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c dd.c -o dd.obj
C:\bcc\bin\bcc32.exe -IC:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c engine.c -o engine.obj
C:\bcc\bin\bcc32.exe -IC:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c main.c -o main.obj
C:\bcc\bin\bcc32.exe -IC:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c inter.c -o inter.obj
C:\bcc\bin\bcc32.exe -IC:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c socket.c -o socket.obj
C:\bcc\bin\bcc32.exe -IC:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c sound.c -o sound.obj
C:\bcc\bin\bcc32.exe -IC:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c conv.c -o conv.obj
C:\bcc\bin\bcc32.exe -IC:\bcc\include -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c options.c -o options.obj
echo Compiling RES
C:\bcc\bin\brcc32 merc.rc
echo Linking
C:\bcc\bin\ilink32 /aa -LC:\bcc\lib -Llpng -Lzlib -LC:\bcc\lib\psdk C:\bcc\lib\c0w32.obj dd.obj engine.obj main.obj inter.obj socket.obj sound.obj conv.obj options.obj,merc.exe,,cw32mt.lib import32.lib zlib.lib libpng.lib ddraw.lib dsound.lib,merc.def,merc.res
echo Removing .objs
del *.obj
echo Finished
pause