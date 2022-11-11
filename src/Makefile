SRC_FILES = cfg.c net.c utils.c command.c mdxext-lex.yy.c mdxext-yacc.tab.c mdx.c mdd.c vce.c rb-tree.c log.c tools/elastic-byte-buffer.c
OBJ_FILES = cfg.o net.o utils.o command.o mdxext-lex.yy.o mdxext-yacc.tab.o mdx.o mdd.o vce.o rb-tree.o log.o tools/elastic-byte-buffer.o

euclid : Euclid.c Euclid-cli.c $(SRC_FILES) Euclid.o Euclid-cli.o $(OBJ_FILES)
	cc  -lpthread      $(OBJ_FILES)  Euclid.o      -o server
	cc  -lpthread  -g  $(SRC_FILES)  Euclid.c      -o server.out
	cc  -lpthread  -lreadline      $(OBJ_FILES)  Euclid-cli.o  -o euclid
	cc  -lpthread  -lreadline  -g  $(SRC_FILES)  Euclid-cli.c  -o euclid.out

mdxext-lex.yy.c: mdxext-lex.l
	flex -i -o mdxext-lex.yy.c mdxext-lex.l

mdxext-yacc.tab.h mdxext-yacc.tab.c: mdxext-yacc.y
	bison -d mdxext-yacc.y

clean :
	rm -f *.o
	rm -f tools/*.o
	rm -f *.out
	rm -f server
	rm -f euclid
	rm -f euclid-cli
	rm -f *yacc.tab.h *yacc.tab.c *lex.yy.c
