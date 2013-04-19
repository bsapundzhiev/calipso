
NAME    =       mod_xcgi
SRCS    =       mod_xcgi.c
OBJS    =       $(SRCS:.c=.o)

CFLAGS  +=      -D_MODULE -DDEBUG -I ../../include -D_REENTRANT
LDFLAGS +=

all:shared

static  :       $(OBJS)
	${AR} -r lib$(NAME).a $(OBJS)
	ranlib lib$(NAME).a

shared  :       $(OBJS)
	${CC} -shared -o $(NAME).so $(OBJS)

clean   :
	rm -f $(OBJS)

distclean  :       clean
	rm -f lib$(NAME).a $(NAME).so *~
