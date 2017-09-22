CC=gcc
LDFLAGS=-I. -I/usr/include/libxml2 
CXXFLAGS=-lxml2 -lm -lyajl

all: vmu_Scarf_CParsing

%.o : %.c
	$(CC) -c -o $@ $< $(LDFLAGS)

vmu_Scarf_CParsing : vmu_Scarf_CParsing.c ScarfXmlReader.o ScarfJsonWriter.o Scarf.h
	$(CC) $(LDFLAGS) $(CXXFLAGS) -g -o $@ vmu_Scarf_CParsing.c ScarfXmlReader.o ScarfJsonWriter.o

clean:
	rm *.o vmu_Scarf_CParsing *.json
