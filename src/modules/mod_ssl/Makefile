
NAME    =       mod_http_ssl
SRCS    =       mod_http_ssl.c
OBJS    =       $(SRCS:.c=.o)

ifndef USE_SSL
	CFLAGS  +=-DUSE_SSL=1
endif

CFLAGS  +=  -fPIC -g -O2 -Wall -D_MODULE -DDEBUG -I ../../include -D_FILE_OFFSET_BITS=64 -D_REENTRANT
LDFLAGS +=  -lssl

ifeq ($(OS), Darwin)
	CFLAGS += -I ../../../os/macosx
endif

all:shared

static  :       $(OBJS)
	${CC} -r lib$(NAME).a $(OBJS)
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
