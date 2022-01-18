BIN    = MD5
COMPILER  ?= gcc

NO_COLOR=\x1b[0m
OK_COLOR=\x1b[32;01m
ERROR_COLOR=\x1b[31;01m
WARN_COLOR=\x1b[33;01m
OK_STRING=$(OK_COLOR)[OK]$(NO_COLOR)

.PHONY: all clean
all: ${BIN}

${BIN}: *.c
	${COMPILER} -g -Wall -o $@ $@.c -pthread -lcrypto
	@echo -e '$(OK_COLOR)[*] Created executable $@ $(NO_COLOR)'

clean:
	rm -f ${BIN}
