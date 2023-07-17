SRC_FILES = src/cfg.c \
	src/net.c \
	src/utils.c \
	src/command.c \
	src/mdxext-lex.yy.c \
	src/mdxext-yacc.tab.c \
	src/mdx.c \
	src/mdd.c \
	src/vce.c \
	src/rb-tree.c \
	src/log.c \
	src/tools/elastic-byte-buffer.c \
	src/locks/global-locks.c \
	src/mdm-astmrfn-interpreter.c \
	src/mdm-astlogifn-interpreter.c \
	src/printer.c \
	src/mdm-ast-str-fn.c \
	src/mathematics.c \
	src/env.c \
	src/mdm-ast-set-func.c \
	src/mdm-ast-num-func.c \
	src/mdm-ast-lv-func.c
OBJ_FILES = src/cfg.o \
	src/net.o \
	src/utils.o \
	src/command.o \
	src/mdxext-lex.yy.o \
	src/mdxext-yacc.tab.o \
	src/mdx.o \
	src/mdd.o \
	src/vce.o \
	src/rb-tree.o \
	src/log.o \
	src/tools/elastic-byte-buffer.o \
	src/locks/global-locks.o \
	src/mdm-astmrfn-interpreter.o \
	src/mdm-astlogifn-interpreter.o \
	src/printer.o \
	src/mdm-ast-str-fn.o \
	src/mathematics.o \
	src/env.o \
	src/mdm-ast-set-func.o \
	src/mdm-ast-num-func.o \
	src/mdm-ast-lv-func.o

euclid: src/euclid-svr.c src/euclid-cli.c $(SRC_FILES) src/euclid-svr.o src/euclid-cli.o $(OBJ_FILES)
	cc      $(OBJ_FILES)  src/euclid-svr.o  -o src/euclid-svr      -lpthread -lm
	cc  -g  $(SRC_FILES)  src/euclid-svr.c  -o src/euclid-svr.out  -lpthread -lm
	cc      $(OBJ_FILES)  src/euclid-cli.o  -o src/euclid-cli      -lpthread -lm
	cc  -g  $(SRC_FILES)  src/euclid-cli.c  -o src/euclid-cli.out  -lpthread -lm

src/mdxext-lex.yy.c: src/mdxext-lex.l src/mdxext-yacc.tab.h
	flex -i -o src/mdxext-lex.yy.c src/mdxext-lex.l

src/mdxext-yacc.tab.h src/mdxext-yacc.tab.c: src/mdxext-yacc.y
	bison -d src/mdxext-yacc.y -o src/mdxext-yacc.tab.c

clean :
	rm -f src/*.o
	rm -f src/tools/*.o
	rm -f src/locks/*.o
	rm -f src/*.out
	rm -f src/euclid-svr
	rm -f src/euclid-cli
	rm -f src/*yacc.tab.h
	rm -f src/*yacc.tab.c
	rm -f src/*lex.yy.c