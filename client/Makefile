CC=gcc
OLIB=lib
OOBJ=obj
COMPOSITOR=$(XDG_SESSION_TYPE)
CFLAGS=-Wall -g -std=c99
DFLAGS=-D compositor=$(COMPOSITOR)
XFLAGS=-lX11 -lxcb -lxcb-randr 
DRMFLAGS=-ldrm -I/usr/include/libdrm
DRM_INC=-I/usr/include/libdrm
DRM_LIB=-ldrm


vtux: main.c
	$(CC) $(CFLAGS) $(DRM_INC) -o vtux main.c $(DRM_LIB)

#vtux: main.o libleasemanager.so
#	$(CC) $(CFLAGS) -o vtux $(OOBJ)/main.o ./$(OLIB)/libleasemanager.so
#
#main.o: out main.c 
#	$(CC) $(CFLAGS) $(DFLAGS) main.c -c -o $(OOBJ)/main.o $(DRMFLAGS)
#
#libleasemanager.so: out lease_manager.o
#	$(CC) -shared -o $(OLIB)/libleasemanager.so $(OOBJ)/lease_manager.o $(XFLAGS)
#
#lease_manager.o: out lease_manager/lease_manager.h lease_manager/lease_manager.c
#	$(CC) -fPIC $(CFLAGS) lease_manager/lease_manager.c -c -o $(OOBJ)/lease_manager.o
#
#out:
#	mkdir -p $(OLIB) $(OOBJ)

clean:
	rm -rf $(OLIB) $(OOBJ) vtux
