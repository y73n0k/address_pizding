CC = gcc

WORDLEN ?= 4
SUBSTR ?= \"libc.so\"
SKIPS ?= 1

SRC = address_pizding.c disable_aslr.c test.c

EXE1 = address_pizding
EXE2 = disable_aslr
EXE3 = test

all: $(EXE1) $(EXE2) $(EXE3)

$(EXE1): address_pizding.c
ifdef IS32
	$(CC) $< -o $@ -DWORDLEN=$(WORDLEN) -DSUBSTR=$(SUBSTR) -DSKIPS=$(SKIPS) -DIS32
else
	$(CC) $< -o $@ -DWORDLEN=$(WORDLEN) -DSUBSTR=$(SUBSTR) -DSKIPS=$(SKIPS)
endif

$(EXE2): disable_aslr.c
	$(CC) $< -o $@

$(EXE3): test.c
	$(CC) $< -o $@

run: all
	./$(EXE1) ./$(EXE2) ./$(EXE3)

clean:
	rm -f $(EXE1) $(EXE2) $(EXE3)
