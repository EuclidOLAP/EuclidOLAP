SRC_FILES = cfg.c net.c utils.c command.c mdxext-lex.yy.c mdxext-yacc.tab.c mdx.c mdd.c vce.c rb-tree.c log.c \
tools/elastic-byte-buffer.c locks/global-locks.c mdm-astmrfn-interpreter.c \
mdm-astlogifn-interpreter.c printer.c mdm-ast-str-fn.c math.c
OBJ_FILES = cfg.o net.o utils.o command.o mdxext-lex.yy.o mdxext-yacc.tab.o mdx.o mdd.o vce.o rb-tree.o log.o \
tools/elastic-byte-buffer.o locks/global-locks.o mdm-astmrfn-interpreter.o \
mdm-astlogifn-interpreter.o printer.o mdm-ast-str-fn.o math.o

euclid : euclid-svr.c euclid-cli.c $(SRC_FILES) euclid-svr.o euclid-cli.o $(OBJ_FILES)
	cc      $(OBJ_FILES)  euclid-svr.o  -o euclid-svr      -lpthread
	cc  -g  $(SRC_FILES)  euclid-svr.c  -o euclid-svr.out  -lpthread
	cc      $(OBJ_FILES)  euclid-cli.o  -o euclid-cli      -lpthread
	cc  -g  $(SRC_FILES)  euclid-cli.c  -o euclid-cli.out  -lpthread

mdxext-lex.yy.c: mdxext-lex.l
	flex -i -o mdxext-lex.yy.c mdxext-lex.l

mdxext-yacc.tab.h mdxext-yacc.tab.c: mdxext-yacc.y
	bison -d mdxext-yacc.y

clean :
	rm -f *.o
	rm -f tools/*.o
	rm -f locks/*.o
	rm -f *.out
	rm -f euclid-svr
	rm -f euclid-cli
	rm -f *yacc.tab.h *yacc.tab.c *lex.yy.c
