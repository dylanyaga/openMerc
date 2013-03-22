rem del full.zip
rem pkzip -ex -P full.zip merc.exe gx*.* pnglib.* sfx\?.wav sfx\1?.wav sfx\click.wav gfx\*.bmp gfx\*.png

del upd.zip
pkzip -ex -P upd.zip merc.exe gfx\*.png gfx\*.bmp
