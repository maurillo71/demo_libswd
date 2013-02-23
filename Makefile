VERSION_STRING = 0.1

sources = demo_libswd.c

TARGETS = $(sources:.c=)

LIBS 	= -lswd
#EXTRA_LIBS ?= -ldl	# for get_cpu
DESTDIR	?=
prefix  ?= /usr
bindir  ?= $(prefix)/bin
mandir	?= $(prefix)/share/man
srcdir	?= $(prefix)/src

override CFLAGS += -D_GNU_SOURCE -Wall -Wno-nonnull -Isrc/include


ifndef DEBUG
	CFLAGS	+= -O2
else
	CFLAGS	+= -O0 -g
endif

# ifeq ($(NUMA),1)
# 	CFLAGS += -DNUMA
# 	NUMA_LIBS = -lnuma
# endif

%.o: %.c
	$(CC) -D VERSION_STRING=$(VERSION_STRING) -c $< $(CFLAGS)

# Pattern rule to generate dependency files from .c files
%.d: %.c
	@$(CC) -MM $(CFLAGS) $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@ || rm -f $@

.PHONY: all
all: $(TARGETS)

# Include dependency files, automatically generate them if needed.
-include $(sources:.c=.d)

demo_libswd: demo_libswd.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(NUMA_LIBS)

CLEANUP  = $(TARGETS) *.o .depend *.*~ *.orig *.rej rt-tests.spec *.d
CLEANUP += $(if $(wildcard .git), ChangeLog)

.PHONY: clean
clean:
	for F in $(CLEANUP); do find -type f -name $$F | xargs rm -f; done

.PHONY: distclean
distclean: clean
	rm -rf BUILD RPMS SRPMS releases *.tar.gz 

.PHONY: changelog
changelog:
	git log >ChangeLog

.PHONY: install
install: all
	mkdir -p "$(DESTDIR)$(bindir)"
	cp $(TARGETS) "$(DESTDIR)$(bindir)"

.PHONY: release
release: clean changelog
	mkdir -p releases
	rm -rf tmp && mkdir -p tmp/rt-tests
	cp -r Makefile COPYING ChangeLog src tmp/rt-tests
	tar -C tmp -czf rt-tests-$(VERSION_STRING).tar.gz rt-tests
	rm -f ChangeLog
	cp rt-tests-$(VERSION_STRING).tar.gz releases

.PHONY: push
push:	release
	scripts/do-git-push $(VERSION_STRING)

.PHONY: pushtest
pushtest: release
	scripts/do-git-push --test $(VERSION_STRING)

rt-tests.spec: Makefile rt-tests.spec-in
	sed s/__VERSION__/$(VERSION_STRING)/ <$@-in >$@

HERE	:=	$(shell pwd)
RPMARGS	:=	--define "_topdir $(HERE)" 	\
		--define "_sourcedir $(HERE)/releases" 	\
		--define "_builddir $(HERE)/BUILD" 	\

.PHONY: rpm
rpm:	rpmdirs release rt-tests.spec
	rpmbuild -ba $(RPMARGS) rt-tests.spec

.PHONY: rpmdirs
rpmdirs:
	@[ -d BUILD ]  || mkdir BUILD
	@[ -d RPMS ]   || mkdir RPMS
	@[ -d SRPMS ]  || mkdir SRPMS

.PHONY: help
help:
	@echo ""
	@echo " rt-tests useful Makefile targets:"
	@echo ""
	@echo "    all       :  build all tests (default"
	@echo "    install   :  install tests to local filesystem"
	@echo "    release   :  build source tarfile"
	@echo "    rpm       :  build RPM package"
	@echo "    clean     :  remove object files"
	@echo "    distclean :  remove all generated files"
	@echo "    help      :  print this message"

.PHONY: tags
tags:
	ctags -R --extra=+f --c-kinds=+p *
