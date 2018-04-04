CC=gcc

THISMACHINE := $(shell uname -srm | sed -e 's/ /-/g')
THISSYSTEM	:= $(shell uname -s)

VERSION     ?= "0.1.0"
TARGETLIB   ?= libcmdtab.$(THISSYSTEM).a
PACKAGEDIR  ?= ./../_hbpkg/$(THISMACHINE)/cmdtab.$(VERSION)

SRCDIR      := .
INCDIR      := .
BUILDDIR    := build
PRODUCTDIR  := bin
RESDIR      := 
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

#CFLAGS      := -std=gnu99 -O -g -Wall
CFLAGS      := -std=gnu99 -O3
LIB         := 
INC         := -I$(INCDIR)
INCDEP      := -I$(INCDIR)

#SOURCES     := $(shell find $(SRCDIR) -type f -name "*.$(SRCEXT)")
SOURCES     := $(shell ls $(SRCDIR)/*.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))



all: lib
lib: resources $(TARGETLIB)
remake: cleaner all
	
#Make the Directories
install:
	@mkdir -p $(PACKAGEDIR)
	@cp ./cmdtab.h $(PACKAGEDIR)
	@cp $(PRODUCTDIR)/* $(PACKAGEDIR)
	@rm -rf $(PACKAGEDIR)/../cmdtab
	@ln -s cmdtab.$(VERSION) ./$(PACKAGEDIR)/../cmdtab


#Copy Resources from Resources Directory to Target Directory
resources: directories

#Make the Directories
directories:
	@mkdir -p $(PRODUCTDIR)
	@mkdir -p $(BUILDDIR)

#Clean only Objects
clean:
	@$(RM) -rf $(BUILDDIR)

#Full Clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf $(PRODUCTDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Build the static library
libcmdtab.Darwin.a: $(OBJECTS)
	ar -rcs $(PRODUCTDIR)/libcmdtab.a $(OBJECTS)
	ranlib $(PRODUCTDIR)/libcmdtab.a

libcmdtab.Linux.a: $(OBJECTS)
	ar -rcs $(PRODUCTDIR)/libcmdtab.a $(OBJECTS)
	ranlib $(PRODUCTDIR)/libcmdtab.a

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

#Non-File Targets
.PHONY: all lib remake clean cleaner resources


