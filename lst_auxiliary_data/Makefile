#-----------------------------------------------------------------------------
# Makefile
#
# Simple makefile for installing the scripts.
#-----------------------------------------------------------------------------
.PHONY: all install clean

# Inherit from upper-level make.config
TOP = ..
include $(TOP)/make.config

SCRIPTS = build_narr_aux_archive_from_CISL_RDA.py \
          update_narr_aux_data.py
MODULES = lst_auxiliary_utilities.py
CONFIG = example-lst_auxiliary.config

#-----------------------------------------------------------------------------
all:

#-----------------------------------------------------------------------------
install:
	install -d $(link_path)
	install -d $(lst_install_path)
	@for script in $(SCRIPTS); do \
            cmd="install -m 755 $$script $(lst_install_path)"; \
            echo "$$cmd"; $$cmd; \
            cmd="ln -sf $(lst_link_source_path)/$$script $(link_path)/$$script"; \
            echo "$$cmd"; $$cmd; \
        done
	@for module in $(MODULES); do \
            cmd="install -m 644 $$module $(lst_install_path)"; \
            echo "$$cmd"; $$cmd; \
            cmd="ln -sf $(lst_link_source_path)/$$module $(link_path)/$$module"; \
            echo "$$cmd"; $$cmd; \
        done
	@for config in $(CONFIG); do \
            cmd="install -m 644 $$config $(lst_install_path)"; \
            echo "$$cmd"; $$cmd; \
            cmd="ln -sf $(lst_link_source_path)/$$config $(link_path)/$$config"; \
            echo "$$cmd"; $$cmd; \
        done

#-----------------------------------------------------------------------------
clean:

