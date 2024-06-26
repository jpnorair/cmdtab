# Copyright 2020, JP Norair
#
# Licensed under the OpenTag License, Version 1.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.indigresso.com/wiki/doku.php?id=opentag:license_1_0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

CC := gcc
LD := ld

THISMACHINE ?= $(shell uname -srm | sed -e 's/ /-/g')
THISSYSTEM	?= $(shell uname -s)

EXT_INC     ?= 
EXT_LIBINC  ?= 
EXT_LIBFLAGS?=
EXT_LIBS    ?= 

VERSION     ?= 0.1.0
PACKAGEDIR  ?= ./../_hbpkg/$(THISMACHINE)/cmdtab.$(VERSION)

ifeq ($(THISSYSTEM),Darwin)
# Mac can't do conditional selection of static and dynamic libs at link time.
#	PRODUCTS := libcmdtab.dylib libcmdtab.a
	PRODUCTS := libcmdtab.a
	EXT_INC := -I/opt/homebrew/include $(EXT_INC)
	EXT_LIBINC := -L/opt/homebrew/lib $(EXT_LIBINC)
	
else ifeq ($(THISSYSTEM),Linux)
	PRODUCTS := libcmdtab.$(THISSYSTEM).so libcmdtab.a
else ifeq ($(THISSYSTEM),CYGWIN_NT-10.0)
	PRODUCTS := libcmdtab.a
else
	error "THISSYSTEM set to unknown value: $(THISSYSTEM)"
endif

SRCDIR      := .
INCDIR      := .
BUILDDIR    := build/$(THISMACHINE)
PRODUCTDIR  := bin/$(THISMACHINE)
RESDIR      := 
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

CFLAGS      ?= -std=gnu99 -O3 -fPIC
LIB         := 
INC         := -I$(INCDIR)
INCDEP      := -I$(INCDIR)

#SOURCES     := $(shell find $(SRCDIR) -type f -name "*.$(SRCEXT)")
SOURCES     := $(shell ls $(SRCDIR)/*.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))



all: lib
lib: resources $(PRODUCTS)
remake: cleaner all
pkg: lib install
	
#Make the Directories
install:
	@mkdir -p $(PACKAGEDIR)
	@cp ./cmdtab.h $(PACKAGEDIR)
	@cp $(PRODUCTDIR)/* $(PACKAGEDIR)
	@rm -rf $(PACKAGEDIR)/../cmdtab
	@ln -s cmdtab.$(VERSION) ./$(PACKAGEDIR)/../cmdtab
	cd ../_hbsys && $(MAKE) sys_install INS_MACHINE=$(THISMACHINE) INS_PKGNAME=cmdtab

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

#Build the static library: universal on all POSIX
libcmdtab.a: $(OBJECTS)
	ar -rcs $(PRODUCTDIR)/libcmdtab.a $(OBJECTS)
	ranlib $(PRODUCTDIR)/libcmdtab.a

#Build Dynamic Library
libcmdtab.Linux.so: $(OBJECTS)
	$(CC) -shared -fPIC -Wl,-soname,libcmdtab.so.1 -o $(PRODUCTDIR)/libcmdtab.so.$(VERSION) $(OBJECTS) -lc

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
.PHONY: all lib pkg remake clean cleaner resources


