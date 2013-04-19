
NAME    =       mod_rewrite
SRCS    =       mod_rewrite.c
		
OBJS    =       $(SRCS:.c=.o)

CFLAGS  +=  -fPIC -g -O2 -Wall -D_MODULE -DDEBUG -I ../../include -I../../cplib/pcre -D_FILE_OFFSET_BITS=64 -D_REENTRANT
#LDFLAGS +=  -L ../../cplib/pcre -lpcre

all:shared

static  :       $(OBJS)
	${CC} -r lib$(NAME).a $(OBJS)
	ranlib lib$(NAME).a

shared  :       $(OBJS)
	${CC} -shared -o $(NAME).so $(OBJS)

clean   :
	rm -f $(OBJS)

distclean  :       clean
	rm -f lib$(NAME).a $(NAME).so *~
