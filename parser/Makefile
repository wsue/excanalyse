
DUMPWRAP_OBJS	= ../../out/binutils/dumpwrap.o 
DUMPWRAP_LIBS	= ../../out/binutils/libopcodes.a ../../out/binutils/libbfd.a ../../out/binutils/libiberty.a

PARSER_OBJDIR	= ../../tmp/parser

PARSER_OBJS 	= $(patsubst %.c,$(PARSER_OBJDIR)/%.o,$(filter-out app.c,$(wildcard *.c)))

CFLAGS	+= -g -I../binutils -I../include
LDFLAGS	+= -lz # -L../../out/binutils -lbfd -liberty -lopcodes  -lm

../../out/binutils/libparser.a:$(PARSER_OBJS) $(DUMPWRAP_OBJS)
	ar ruv $@ $(PARSER_OBJS) $(DUMPWRAP_OBJS)
	ranlib $@

test:$(PARSER_OBJDIR) app

app:$(PARSER_OBJS) $(PARSER_OBJDIR)
	$(CC) -g app.c -o app $(CFLAGS) $(PARSER_OBJS) $(DUMPWRAP_OBJS)  $(DUMPWRAP_LIBS) $(LDFLAGS)


clean:
	rm -rf $(PARSER_OBJDIR)/*


$(PARSER_OBJDIR):
	mkdir -p $@

$(PARSER_OBJDIR)/%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<
