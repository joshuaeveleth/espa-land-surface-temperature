#-----------------------------------------------------------------------------
# Makefile
#
# Simple makefile for building and installing land-surface-temperature
# applications.
#-----------------------------------------------------------------------------
.PHONY: check-environment all install clean all-script install-script clean-script all-rit install-rit clean-rit all-rit-aux install-rit-aux clean-rit-aux rpms rit-rpm rit-aux-rpm

include make.config

DIR_RIT = not-validated-prototype_lst
DIR_AUX = lst_auxiliary_data

all: all-script all-rit

install: check-environment install-script install-rit

clean: clean-script clean-rit clean-aux

#-----------------------------------------------------------------------------
all-script:
	echo "make all in scripts"; \
        (cd scripts; $(MAKE) all);

install-script: check-environment
	echo "make install in scripts"; \
        (cd scripts; $(MAKE) install);

clean-script:
	echo "make clean in scripts"; \
        (cd scripts; $(MAKE) clean);

#-----------------------------------------------------------------------------
all-rit:
	echo "make all in not-validated-prototype_lst"; \
        (cd $(DIR_RIT); $(MAKE) all);

install-rit: check-environment
	echo "make install in not-validated-prototype_lst"; \
        (cd $(DIR_RIT); $(MAKE) install);

clean-rit:
	echo "make clean in not-validated-prototype_lst"; \
        (cd $(DIR_RIT); $(MAKE) clean);

#-----------------------------------------------------------------------------
all-rit-aux:
	echo "make install in lst_auxiliary_data"; \
        (cd $(DIR_AUX); $(MAKE));

install-rit-aux:
	echo "make install in lst_auxiliary_data"; \
        (cd $(DIR_AUX); $(MAKE) install);

clean-rit-aux:
	echo "make install in lst_auxiliary_data"; \
        (cd $(DIR_AUX); $(MAKE) clean);

#-----------------------------------------------------------------------------
rpms: rit-rpm
	rpmbuild -bb --clean RPM_spec_files/RPM.spec

rit-rpm:
	rpmbuild -bb --clean RPM_spec_files/RPM-RIT.spec

rit-aux-rpm:
	rpmbuild -bb --clean RPM_spec_files/RPM-RIT-Aux.spec

#-----------------------------------------------------------------------------
check-environment:
ifndef PREFIX
    $(error Environment variable PREFIX is not defined)
endif

