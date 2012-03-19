VERSION = 0.01
CC      = gcc
STRIP   = strip
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\"
INCDIR	= -I/usr/include/ -I.
LIBDIR  = /usr/lib/
LDFLAGS = -L$(LIBDIR) -static
RM      = rm -f

TRG = misc_bricked
OBJ = gopt.o misc_version.o 

all: $(OBJ)
	$(CC) $(CFLAGS) $(INCDIR) -o $(TRG) $(OBJ) $(LDFLAGS)
	$(STRIP) --strip-all misc_bricked

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) misc_version
