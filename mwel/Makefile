RM_RF = /bin/rm -Rf
RSYNC_A = /usr/bin/rsync -a

MW_APP_SUPPORT_DIR = /Library/Application\ Support/MWorks
MWEL_DIR = $(MW_APP_SUPPORT_DIR)/MWEL

.PHONY: all install uninstall test

all: install

install: test
	$(RSYNC_A) --exclude=__pycache__ mwel mwel2xml xml2mwel $(MWEL_DIR)

uninstall:
	$(RM_RF) $(MWEL_DIR)

test:
	/usr/bin/python -W default -m unittest discover
