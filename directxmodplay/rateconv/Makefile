#
# $Id$
#
# Makefile fore sample rate converter
# ===================================
#
# Mum, 10.12.95/4
#
#
# CHECK THE FOLLOWING SETTINGS ...
# ---------------------------------
#
#	Uncomment this if your on a Motorola/Sparc/Mips/PDP-machine
#	otherwise we assume your Intel (if you make the wrong decision
#	rateconv will produce noise on the output, regardless of the
#	byte order - see below - you might select)
#
# OPTION1 = -DI_AM_BIG_ENDIAN
#
#
#	Uncomment this if the byte order of the sound data stream
#	is expected highbyte-lowbyte by DEFAULT. If you are not sure
#	uncomment if you did so above (you can use command line options
#	later to select a different order).
#
# OPTION2 = -DHBYTE1ST_DEFAULT
#
#
#	Uncomment this if you want stereo instead of mono mode operation
#	by DEFAULT (again you can use command line options to change mode).
#
# OPTION3 = -DSTEREO_DEFAULT
#
#
#	use optimizing compiler for maximum speed in FIR-loop
#
CC = gcc
CFLAGS = -O2
TARGET = rateconv
LIBS = -lm
OPTIONS = $(OPTION1) $(OPTION2) $(OPTION3)

#
# ... NO CONFIGURABLE OPTIONS BELOW
# ---------------------------------
#

rateconv: Makefile rateconv.c
	$(CC) $(OPTIONS) $(CFLAGS) rateconv.c $(LIBS) -o $(TARGET)

clean: 
	rm -f $(TARGET)
.PHONEY: clean


#
# only distribuition-administrative stuff below ...
# -------------------------------------------------

VERSION = `echo $$Revision$$Revision | sed 's/: //g'`
DISTFILES = COPYING README HISTORY Makefile rateconv.c rateconv.txt rcv examples/[!R]*
DISTDIR = rateconv-$(VERSION)
TARFILE = rateconv$(VERSION).tz
TARCMD = gtar cvhzf


dist: $(DISTFILES) distclean 
	@( echo `id` | ggrep '(mummert)' >/dev/null ) || ( echo "--- YOU ARE NOT mummert - PLEASE CHECK FILE COPYING ---" && exit 1 )
	mkdir $(DISTDIR)
	echo $(DISTFILES) | tr ' ' '\012' | cpio -pvmda $(DISTDIR)/.
	( cd $(DISTDIR); gls -l > MANIFEST ) || ( cd $(DISTDIR); ls -l > MANIFEST )
	$(TARCMD) $(TARFILE) $(DISTDIR)
	rm -rf  $(DISTDIR)
.PHONEY: dist

distclean: 
	rm -rf $(DISTDIR) $(DISTDIR)$(VERSION)
.PHONEY: distclean

realclean: distclean clean
	rm -f *.bin 
.PHONEY: realclean

uu: dist
	uuencode $(TARFILE) rateconv-$(VERSION).tar.gz > uu

#
# EOT
#
