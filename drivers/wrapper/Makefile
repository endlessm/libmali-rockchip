all: libMaliWrapper.so

libMaliWrapper.so: libMaliWrapper.c
	$(CC) -c -fPIC -O2 libMaliWrapper.c
	$(LD) -Bshareable -o libMaliWrapper.so libMaliWrapper.o -L../meson8 -lMali

clean:
	rm -f libMaliWrapper.o libMaliWrapper.so

.PHONY: all clean
