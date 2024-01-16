TARGET_ROOT = pager
TARGET_SHARED = lib${TARGET_ROOT}.so
TARGET_STATIC = lib${TARGET_ROOT}.a
TARGET_TEST = test

PREFIX ?= /usr/local
MAN_PATH = $(PREFIX)/share/man/man3

MAN_PAGE = $(TARGET_ROOT).3

# Change if source files not in base directory:
SRC = .

CFLAGS = -Wall -Werror -std=c99 -pedantic -ggdb -fvisibility=hidden
LDFLAGS =


# define to make 'dprintf' and especially 'vdprintf' available:
CFLAGS += -D_POSIX_C_SOURCE=200809
# to enable cfmakeraw
CFLAGS += -D_DEFAULT_SOURCE

# CFLAGS += -fsanitize=address
# LDFLAGS += -fsanitize=address

# Uncomment the following if target is a Shared library
CFLAGS_LIB = $(CFLAGS) -fPIC
LDFLAGS_LIB = $(LDFLAGS)
LDFLAGS_TEST = $(LDFLAGS) -lcontools -llinelist -ltinfo

# Build module list (info make -> "Functions" -> "File Name Functions")
MODULES = $(addsuffix .o,$(filter-out ./test_%,$(basename $(wildcard $(SRC)/*.c))))
TEST_TARGETS = $(subst test_,,$(filter ./test_%,$(basename $(wildcard $(SRC)/*.c))))
TEST_SOURCES = $(addsuffix .c,$(filter ./test_%,$(basename $(wildcard $(SRC)/*.c))))
TEST_MODULES = $(addsuffix .o,$(filter ./test_%,$(basename $(wildcard $(SRC)/*.c))))

# Libraries need header files.  Set the following accordingly:
HEADERS = $(TARGET_ROOT).h


# Declare non-filename targets
.PHONY: all preview install uninstall clean help

all: ${TARGET_SHARED} ${TARGET_STATIC}

preview:
	@echo Beginning build of ${TARGET_ROOT} libraries.
	@echo "shared library:" ${TARGET_SHARED}
	@echo "static library:" ${TARGET_STATIC}
	@echo "modules:       " ${MODULES}
	@echo "test sources:  " $(TEST_SOURCES)
	@echo "test targets:  " $(TEST_TARGETS)

${TARGET_SHARED}: ${MODULES} ${HEADERS}
	${CC} ${CFLAGS} --shared -o $@ ${MODULES} ${LDFLAGS}

${TARGET_STATIC}: ${MODULES} ${HEADERS}
	ar rcs $@ ${MODULES} $(LDFLAGS)

%o : %c ${HEADERS}
	$(CC) $(CFLAGS_LIB) -c -o $@ $<

test:
	rm -f $(TEST_TARGETS)
	$(MAKE) $(TEST_TARGETS)

$(TEST_TARGETS) : $(TEST_SOURCES)
	$(CC) $(CFLAGS) -o $@ test_$@.c $(TARGET_STATIC) $(LDFLAGS_TEST)

For shared library targets:
install:
	mkdir --mode=775 -p $(MAN_PATH)
	install -D --mode=644 $(HEADERS) $(PREFIX)/include
	install -D --mode=775 $(TARGET_STATIC) $(PREFIX)/lib
	install -D --mode=775 $(TARGET_SHARED) $(PREFIX)/lib
	soelim $(MAN_PAGE) | gzip -c - > $(MAN_PATH)/$(MAN_PAGE).gz
	ldconfig $(PREFIX)/lib

# Remove the ones you don't need:
uninstall:
	rm -f $(PREFIX)/lib/$(TARGET)
	rm -f $(PREFIX)/include/$(HEADERS)
	rm -f $(MAN_PATH)/$(MAN_PAGE).gz
	ldconfig $(PREFIX)/lib

clean:
	rm -f $(TARGET_SHARED)
	rm -f $(TARGET_STATIC)
	rm -f $(TARGET_TEST)
	rm -f $(MODULES)
	rm -f $(TEST_TARGETS)

help:
	@echo makeflage are $(MAKEFLAGS)

	@echo "Makefile options:"
	@echo
	@echo "  test       to build test program using library"
	@echo "  preview    to see relevent files"
	@echo "  install    to install project"
	@echo "  uninstall  to uninstall project"
	@echo "  clean      to remove generated files"
	@echo "  help       this display"
