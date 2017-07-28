@echo off
SET BINPATH=C:\Borland\BCC55\Bin
SET INCPATH=C:\Borland\BCC55\Include
SET LIBPATH=C:\Borland\BCC55\Lib
SET OUTPATH=openMerc
rmdir %OUTPATH%
echo Compiling Source
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c dd.c -o dd.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c engine.c -o engine.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c main.c -o main.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c inter.c -o inter.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c socket.c -o socket.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c sound.c -o sound.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c conv.c -o conv.obj
%BINPATH%\bcc32.exe -I%INCPATH% -Izlib -O2 -5 -d -ff -k- -OS -Q -X -W -WM -c options.c -o options.obj
echo Compiling RES
%BINPATH%\brcc32 merc.rc
echo Linking
%BINPATH%\ilink32 /aa -L%LIBPATH% -Llpng -Lzlib -L%LIBPATH%\psdk %LIBPATH%\c0w32.obj dd.obj engine.obj main.obj inter.obj socket.obj sound.obj conv.obj options.obj,merc.exe,,cw32mt.lib import32.lib zlib.lib libpng.lib ddraw.lib dsound.lib,merc.def,merc.res

echo Cleaning Up
del *.obj
del merc.ilc
del merc.ild
del merc.ilf
del merc.ils
del merc.map
del merc.RES
del merc.tds

echo Packaging Client
mkdir %OUTPATH%
mkdir %OUTPATH%\gfx
mkdir %OUTPATH%\sfx
move  merc.exe %OUTPATH%
copy "..\..\Resources\Packaged Graphics\*.dat" %OUTPATH%
copy "..\..\Resources\Packaged Graphics\*.idx" %OUTPATH%
copy "..\..\Resources\Sound\sfx\*.*" %OUTPATH%\sfx
copy "..\..\Resources\DLLs\NVIDIA\*.dll" %OUTPATH%
echo Finished
pause
