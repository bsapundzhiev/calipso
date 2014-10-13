
NAME    =       mod_proxy
SRCS    =       mod_proxy.c
OBJS    =       $(SRCS:.c=.o)

CFLAGS  +=      -fPIC -D_MODULE -DDEBUG -I ../../include
LDFLAGS +=

ifeq ($(OS), Darwin)
	CFLAGS += -I ../../../os/macosx
endif

all:shared

static  :       $(OBJS)
	${AR} -r lib$(NAME).a $(OBJS)
	ranlib lib$(NAME).a

shared  :       $(OBJS)
ifeq ($(OS), Darwin)
	${CC} -dynamiclib -undefined dynamic_lookup -o $(NAME).bundle $(OBJS)
else
	${CC} -shared -o $(NAME).so $(OBJS)
endif

clean   :
	rm -f $(OBJS)

distclean  :       clean
	rm -f lib$(NAME).a $(NAME).so *~
