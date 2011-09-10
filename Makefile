PROJECT = nilwm

SOURCE = nilwm.c config.c event.c client.c layout.c bar.c
OBJECTS = ${SOURCE:.c=.o}
DEBUG_OBJECTS = ${SOURCE:.c=.do}

VERSION = 0.1
PREFIX ?= /usr/local
MANPREFIX ?= ${PREFIX}/share/man

XCB_FLAGS = $(shell pkg-config --cflags xcb-keysyms)
XCB_LIBS = $(shell pkg-config --libs xcb-keysyms)

DEBUG_FLAGS = -O0 -g -DDEBUG

CFLAGS += -Wall -Wextra ${XCB_FLAGS}
LDFLAGS += ${XCB_LIBS}

all: ${PROJECT}

${PROJECT}: ${OBJECTS}
	@echo CC -o $@
	@${CC} ${LDFLAGS} -o $@ ${OBJECTS}

${PROJECT}-debug: ${DEBUG_OBJECTS}
	@echo CC -o ${PROJECT}-debug
	@${CC} ${LDFLAGS} -o ${PROJECT}-debug ${DEBUG_OBJECTS}

${OBJECTS}: config.h

${DEBUG_OBJECTS}: config.h

config.h: config.def.h
	@if [ -f $@ ] ; then \
		echo "config.h exists, but config.def.h is newer."; \
	else \
		cp $< $@ ; \
	fi

%.o: %.c
	@echo CC $<
	@${CC} -c ${CFLAGS} -o $@ $<

%.do: %.c
	@echo CC $<
	@${CC} -c ${CFLAGS} ${DEBUG_FLAGS} -o $@ $<

debug: ${PROJECT}-debug

clean:
	@rm -rf ${PROJECT} ${OBJECTS} ${PROJECT}-debug ${DEBUG_OBJECTS}

distclean: clean
	@rm -rf config.h

install: all
	@echo installing executable file
	@mkdir -p "${DESTDIR}${PREFIX}/bin"
	@cp -f "${PROJECT}" "${DESTDIR}${PREFIX}/bin"
	@chmod 755 "${PROJECT}" "${DESTDIR}${PREFIX}/bin/${PROJECT}"
	@echo installing manual page
	@mkdir -p "${DESTDIR}${MANPREFIX}/man1"
	@sed "s/VERSION/${VERSION}/g" < ${PROJECT}.1 > "${DESTDIR}${MANPREFIX}/man1/${PROJECT}.1"
	@chmod 644 "${DESTDIR}${MANPREFIX}/man1/${PROJECT}.1"

uninstall:
	@echo removing executable file
	@rm -f "${DESTDIR}${MANPREFIX}/bin/${PROJECT}"
	@echo removing manual page
	@rm -f "${DESTDIR}${MANPREFIX}/man1/${PROJECT}.1"

