
ALARM_APP = 1
COUNTDOWN_APP = 0
CALIBRATE_APP = 1
RPN_APP = 1
PHONEBOOK_APP = 1
HEX_APP = 1
STOPWATCH_APP = 1
PHRASE_APP = 0
HEBREW_APP = 1
SHABBAT_APP = 0
RNG_APP = 0
TUNER_APP = 0
MORSE_APP = 0
BEACON_APP = 0
OOK_APP = 1
SHADERS_APP = 0
COUNTER_APP = 1
DMESG_APP = 1
PAGER_APP = 0
JUKEBOX_APP = 1

#Don't forget to set your timezone for Beats in config.h.
BEATS_APP = 0


#set default flashing serial port, dont override if passed in as an argument
PORT ?= /dev/ttyUSB0
#Mandatory applets.
APPS_OBJ += apps/clock.o apps/settime.o apps/submenu.o 

#add optional apps
ifeq ($(ALARM_APP),1)
APPS_OBJ += apps/alarm.o 
APPS_DEFINES += ALARM_APP
endif
ifeq ($(COUNTDOWN_APP),1)
APPS_OBJ += apps/countdown.o 
APPS_DEFINES += COUNTDOWN_APP
endif
ifeq ($(SHABBAT_APP),1)
APPS_OBJ += apps/shabbat.o 
APPS_DEFINES += SHABBAT_APP
endif
ifeq ($(HEBREW_APP),1)
APPS_OBJ += apps/hebrew.o libs/hebrew.o
APPS_DEFINES += HEBREW_APP
endif
ifeq ($(DMESG_APP),1)
APPS_OBJ += apps/dmesg.o 
APPS_DEFINES += DMESG_APP
endif
ifeq ($(RPN_APP),1)
APPS_OBJ += apps/rpn.o 
APPS_DEFINES += RPN_APP
endif
ifeq ($(PHONEBOOK_APP),1)
APPS_OBJ += apps/phonebook.o libs/phonebook.o
APPS_DEFINES += PHONEBOOK_APP
endif
ifeq ($(HEX_APP),1)
APPS_OBJ += apps/hex.o 
APPS_DEFINES += HEX_APP
endif
ifeq ($(CALIBRATE_APP),1)
APPS_OBJ += apps/calibrate.o 
APPS_DEFINES += CALIBRATE_APP
endif 
ifeq ($(STOPWATCH_APP),1)
APPS_OBJ += apps/stopwatch.o 
APPS_DEFINES += STOPWATCH_APP
endif 
ifeq ($(PHRASE_APP),1)
APPS_OBJ += apps/phrase.o 
APPS_DEFINES += PHRASE_APP
endif
ifeq ($(RNG_APP),1)
APPS_OBJ += apps/rngapp.o 
APPS_DEFINES += RNG_APP
endif 
ifeq ($(TUNER_APP),1)
APPS_OBJ += apps/tuner.o
APPS_DEFINES += TUNER_APP 
endif
ifeq ($(MORSE_APP),1)
APPS_OBJ += apps/morse.o 
APPS_DEFINES += MORSE_APP
endif
ifeq ($(BEACON_APP),1)
APPS_OBJ += apps/beacon.o
APPS_DEFINES += BEACON_APP 
endif
ifeq ($(OOK_APP),1)
APPS_OBJ += apps/ook.o
APPS_DEFINES += OOK_APP 
endif
ifeq ($(SHADERS_APP),1)
APPS_OBJ += apps/shaders.o
APPS_DEFINES += SHADERS_APP 
endif
ifeq ($(COUNTER_APP),1)
APPS_OBJ += apps/counter.o
APPS_DEFINES += COUNTER_APP 
endif
ifeq ($(PAGER_APP),1)
APPS_OBJ += apps/pager.o
APPS_DEFINES += PAGER_APP 
endif
ifeq ($(JUKEBOX_APP),1)
APPS_OBJ += apps/jukebox.o
APPS_DEFINES += JUKEBOX_APP 
endif
ifeq ($(BEATS_APP),1)
APPS_OBJ += apps/beats.o
APPS_DEFINES += BEATS_APP
endif



#GCC8 from Texas Instruments, not the GCC4 that ships with Debian.
CC = msp430-elf-gcc -minrt -msmall -mmcu=cc430f6137 -Wall -I. -I/opt/msp430-gcc-support-files/include -Os  $(addprefix -D, $(APPS_DEFINES)) -Wl,--gc-sections,--print-gc-sections -fdata-sections -ffunction-sections -fno-asynchronous-unwind-tables -flto

BSL = ../bin/cc430-bsl.py -r 38400 -p $(PORT)

modules=rtcasm-r12.o lcd.o lcdtext.o rtc.o  keypad.o bcd.o apps.o\
	applist.o adc.o ref.o codeplugstr.o \
	sidebutton.o power.o uart.o monitor.o ucs.o buzz.o \
	radio.o packet.o dmesg.o codeplug.o rng.o descriptor.o \
	optim.o libs/assembler.o libs/morse.o libs/pocsag.o libs/beats.o \
	printf.o

apps= $(APPS_OBJ)

all: goodwatch.hex

*.c: githash.h buildtime.h

githash.h:
	echo "#define GITHASH" 0x`git rev-parse HEAD | head -c7` > githash.h
buildtime.h:
	../bin/buildtime.py >buildtime.h

goodwatch.elf: $(modules) $(apps) *.h main.c
	$(CC)  -T cc430f6137.ld -o goodwatch.elf main.c $(modules) $(apps)
	../bin/printsizes.py goodwatch.elf || echo "Please install python-pyelftools."
rftest.elf: $(modules) $(apps) *.h main.c
	$(CC) -DRFTEST  -T cc430f6137.ld -o rftest.elf main.c $(modules) $(apps)
	../bin/printsizes.py rftest.elf || echo "Please install python-pyelftools."

goodwatch.hex: goodwatch.elf
	msp430-elf-objcopy -O ihex goodwatch.elf goodwatch.hex
rftest.hex: rftest.elf
	msp430-elf-objcopy -O ihex rftest.elf rftest.hex

clean:
	rm -rf *~ */*~ *.hex *.elf *.o */*.o goodwatch githash.h buildtime.h html latex goodwatch.elf energytrace.png energytrace.txt codeplugstr.c dmesg.bin
	cd libs && make clean
erase:
	$(BSL) -e
sbwflash: goodwatch.hex codeplug.hex
	mspdebug tilib "prog goodwatch.elf" "load codeplug.hex"
sbwrftest: rftest.hex codeplug.hex
	mspdebug tilib "prog rftest.elf" "load codeplug.hex"

flash: goodwatch.hex
	$(BSL) -ef goodwatch.hex
dmesg:
	$(BSL) -P goodwatch.hex -uD
sbwdmesg:
	mspdebug tilib "save_raw 0x2400 2048 dmesg.bin"
	strings dmesg.bin

#Same as dmesg, but it gives the target some time to boot first.
run:
	sleep 5
	../bin/cc430-bsl.py -p $(PORT) -P goodwatch.hex -uD

codeplug.hex: codeplug.txt
	../bin/goodwatch-txt2cp.py -i codeplug.txt -o codeplug.hex
codeplugstr.c: codeplug.txt
	../bin/goodwatch-txt2cpstr.py -i codeplug.txt -o codeplugstr.c
flashcp: codeplug.hex
	$(BSL) -Ef codeplug.hex

dump:
	$(BSL) -ed

fmt:
	clang-format -i -style=LLVM `find . -name \*.c` `find . -name \*.h`


energytrace.txt: energytrace

energytrace: goodwatch.hex sbwflash
	energytrace 60 > energytrace.txt #This will take a minute.
	gnuplot energytrace.gnuplot
	gnuplot energytrace-txt.gnuplot
	../bin/batterylife.py <energytrace.txt

docs:
	doxygen
docsdeploy: docs
	rsync -ave ssh html/* goodwatch.org:/var/www/html/doxygen/
