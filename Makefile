CC=gcc
AR=ar
CFLAGS=-Wall -O3 -g
SRCS=filter.c lexer.c parser.c str.t.c table.t.c utilities.c json.c munit.c str.c table.c
LIB_OBJS=json.o lexer.o parser.o str.o table.o utilities.o

all: test

test: filter str.t table.t
	@./str.t
	@./table.t
	@./run_tests.pl

json.a: $(LIB_OBJS)
	$(AR) rv $@ $(LIB_OBJS)
	ranlib $@

filter: filter.o json.a
	$(CC) $(LDFLAGS) -o $@ $@.o json.a

str.t: str.t.o munit.o json.a
	$(CC) $(LDFLAGS) -o $@ $@.o munit.o json.a

table.t: table.t.o munit.o json.a
	$(CC) $(LDFLAGS) -o $@ $@.o munit.o json.a

clean:
	-rm json.a filter str.t table.t *.o

depend: $(SRCS)
	./utilities/makedepend.pl '$(CC) $(CFLAGS) -MM $(SRCS)'

# DO NOT DELETE THIS LINE -- make depend depends on it.

filter.o: filter.c json.h parser.h tokens.h str.h
lexer.o: lexer.c lexer.h tokens.h str.h utilities.h
parser.o: parser.c parser.h json.h tokens.h lexer.h utilities.h
str.t.o: str.t.c str.h munit.h
table.t.o: table.t.c table.h munit.h utilities.h
utilities.o: utilities.c utilities.h
json.o: json.c json.h str.h table.h utilities.h
munit.o: munit.c munit.h
str.o: str.c str.h utilities.h
table.o: table.c table.h utilities.h
