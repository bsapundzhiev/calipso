
NAME    =       mod_proxy
SRCS    =       mod_proxy.c
OBJS    =       $(SRCS:.c=.o)

CFLAGS  +=      -fPIC -D_MODULE -DDEBUG -I ../../include
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
