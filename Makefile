.PHONY: clean all

BINDIR = ./bin
LIBDIR = ./lib

ROOTCFLAGS	:= $(shell root-config --cflags)
ROOTLIBS	:= $(shell root-config --libs)
ROOTVER		:= $(shell root-config --version | head -c1)

CPP         := $(shell root-config --cxx)
CFLAGS      := -Wall -g $(ROOTCFLAGS) -fPIC

INCLUDES    := -I./

LIBS        := $(ROOTLIBS)

ifeq ($(ROOTVER),5)
	ROOTDICT  := rootcint
	DICTEXT   := .h
else
	ROOTDICT  := rootcling
	DICTEXT   := _rdict.pcm
endif

all: geff

OBJECTS = GlobalFitter.o \
          FitEff.o \
          geff_dict.o

geff: geff.cc $(OBJECTS)
	$(CPP) $(CFLAGS) $(INCLUDES) $< $(OBJECTS) -o $@ $(LIBS)
	$(AR) cru lib$@.a $(OBJECTS)
	cp $@ $(BINDIR)/
	cp lib$@.a $(LIBDIR)/

%.o: %.cc %.hh
	$(CPP) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o *Dict.cc *$(DICTEXT)

# Root stuff
DEPENDENCIES = GlobalFitter.hh \
               FitEff.hh \
               convert.hh \
               cxxopts.hh \
               RootLinkDef.h

%_dict.o: %_dict.cc
	$(CPP) $(CFLAGS) $(INCLUDES) -c $<

%_dict.cc: $(DEPENDENCIES)
	rm -f $@ *_dict$(DICTEXT)
	$(ROOTDICT) -f $@ -c $(INCLUDES) $(DEPENDENCIES)
	cp $*_dict$(DICTEXT) $(LIBDIR)/
