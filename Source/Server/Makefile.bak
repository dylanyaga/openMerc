all: server cgi/info.cgi respawn password cgi/acct.cgi 

CC=gcc
DEBUG=-g
CFLAGS=-Wall -Wshadow -Winline -Wno-pointer-sign -O3 $(DEBUG) -march=pentium4 -fno-strict-aliasing
LDFLAGS=-O $(DEBUG) -lm -lz -lcrypt

OBJS=.obj/server.o .obj/svr_disk.o .obj/svr_tick.o .obj/svr_act.o \
.obj/driver.o .obj/svr_god.o .obj/svr_do.o .obj/svr_glob.o .obj/build.o \
.obj/use_driver.o .obj/look_driver.o .obj/svr_effect.o \
.obj/driver_etc.o .obj/driver_generic.o .obj/populate.o \
.obj/helper.o .obj/skill.o .obj/skill_driver.o .obj/talk.o \
.obj/area.o .obj/path.o .obj/stunrun.o .obj/cityattack.o \
.obj/npc_malte.o .obj/lab9.o .obj/rdtsc.o .obj/ccp_driver.o 

SHEADER=client.h data.h funcs.h gendefs.h globals.h server.h driver.h \
lab9.h

server:	$(OBJS) 
	$(CC) $(LDFLAGS) -o server $(OBJS) 

cgi/info.cgi: info.c gendefs.h data.h
	$(CC) $(CFLAGS) -o info.cgi info.c
	cp info.cgi cgi/info.cgi

cgi/acct.cgi: acct.c gendefs.h data.h .obj/skill.o
	$(CC) $(CFLAGS) -Icgilib -o acct.cgi acct.c .obj/skill.o cgilib/cgi-lib.a
	cp acct.cgi cgi/acct.cgi

respawn: respawn.c $(SHEADER)
	$(CC) $(CFLAGS) $(LDFLAGS) -o respawn respawn.c

password: password.c $(SHEADER)
	$(CC) $(CFLAGS) $(LDFLAGS) -o password password.c

.obj/server.o:		server.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/server.o -c server.c

.obj/helper.o:		helper.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/helper.o -c helper.c

.obj/stunrun.o:		stunrun.c $(SHEADER) npc.h
	$(CC) $(CFLAGS) -o .obj/stunrun.o -c stunrun.c

.obj/npc_malte.o:	npc_malte.c $(SHEADER) npc.h
	$(CC) $(CFLAGS) -o .obj/npc_malte.o -c npc_malte.c

.obj/cityattack.o:	cityattack.c $(SHEADER) npc.h
	$(CC) $(CFLAGS) -o .obj/cityattack.o -c cityattack.c

.obj/svr_disk.o:	svr_disk.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/svr_disk.o -c svr_disk.c

.obj/svr_tick.o:	svr_tick.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/svr_tick.o -c svr_tick.c

.obj/svr_act.o:		svr_act.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/svr_act.o -c svr_act.c

.obj/driver.o:		driver.c $(SHEADER) npc.h
	$(CC) $(CFLAGS) -o .obj/driver.o -c driver.c

.obj/ccp_driver.o:	ccp_driver.c $(SHEADER) npc.h
	$(CC) $(CFLAGS) -o .obj/ccp_driver.o -c ccp_driver.c

.obj/talk.o:		talk.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/talk.o -c talk.c

.obj/path.o:		path.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/path.o -c path.c

.obj/area.o:		area.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/area.o -c area.c

.obj/driver_generic.o:	driver_generic.c $(SHEADER) 
	$(CC) $(CFLAGS) -o .obj/driver_generic.o -c driver_generic.c

.obj/driver_etc.o:	driver_etc.c $(SHEADER) 
	$(CC) $(CFLAGS) -o .obj/driver_etc.o -c driver_etc.c

.obj/skill.o:		skill.c $(SHEADER) 
	$(CC) $(CFLAGS) -o .obj/skill.o -c skill.c

.obj/look_driver.o:	look_driver.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/look_driver.o -c look_driver.c

.obj/skill_driver.o:	skill_driver.c $(SHEADER) 
	$(CC) $(CFLAGS) -o .obj/skill_driver.o -c skill_driver.c

.obj/use_driver.o:	use_driver.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/use_driver.o -c use_driver.c

.obj/svr_god.o:		svr_god.c $(SHEADER) 
	$(CC) $(CFLAGS) -o .obj/svr_god.o -c svr_god.c

.obj/svr_do.o:		svr_do.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/svr_do.o -c svr_do.c

.obj/svr_glob.o:	svr_glob.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/svr_glob.o -c svr_glob.c

.obj/svr_effect.o:	svr_effect.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/svr_effect.o -c svr_effect.c

.obj/build.o:	build.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/build.o -c build.c

.obj/populate.o:	populate.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/populate.o -c populate.c

.obj/lab9.o:		lab9.c $(SHEADER)
	$(CC) $(CFLAGS) -o .obj/lab9.o -c lab9.c

.obj/rdtsc.o:		rdtsc.S
	$(CC) $(CFLAGS) -o .obj/rdtsc.o -c rdtsc.S

clean:
	- rm server acct.cgi info.cgi .obj/*.o *~

dist:
	rm -f merc2.tgz
	tar -cvzf merc2.tgz *.c *.h *.S Makefile README COPYRIGHT LICENSE .dat/* badnames.txt badwords.txt cgilib cgi .obj
