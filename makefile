src = $(wildcard src/*.c)
obj = $(src:.c=.o)

#gcc -o hic-viz *.c htslib/libhts.a -lgd -lpng -lfreetype -pthread -lcrypto -lcurl -lbz2 -lz -lm
CFLAGS  += -Wall -std=c99
LDFLAGS += -Lsrc/htslib/ -lhts -lgd -lpng -lfreetype -lm -std=c99

# Dynamic libs to include for static htslib
LDFLAGS += -pthread -lcrypto -lcurl -lbz2 -lz -llzma

# Optimizations
CFLAGS  += -O3 -fgnu89-inline -std=c99 -march=native -mtune=native
LDFLAGS += -O3 -std=c99 -march=native -mtune=native

hic-viz: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) polycat
