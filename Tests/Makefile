CCOPTS= -Wall -g -std=gnu99 -Wstrict-prototypes
LIBS= 
CC=gcc
AR=ar


BINS= bitmap_test\
	diskdriver_test\

OBJS = #add here your object files

HEADERS=bitmap_test_util.h\
	diskdriver_test_util.h\

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

.phony: clean all


all:	$(BINS) 

bitmap_test: bitmap_test.c $(OBJS) 
	$(CC) $(CCOPTS)  -o $@ $^ $(LIBS)
	
diskdriver_test: diskdriver_test.c $(OBJS) 
	$(CC) $(CCOPTS)  -o $@ $^ $(LIBS)

clean:
	rm -rf *.o *~  $(BINS)
