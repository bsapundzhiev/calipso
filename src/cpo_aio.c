//testing : use -lrt -DFILE_AIO_TEST

#include <stdio.h>
#include <stdlib.h>
#include <aio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

//The signal number to use.
#define SIG_AIO SIGRTMIN + 5

int total;

//Signal handler called when an AIO operation finishes
void aio_handler(int signal, siginfo_t *info, void*uap)
{
/*
	int cbNumber = info->si_value.sival_int;
	printf("AIO operation %d completed returning %d\n", 
		cbNumber,
		0);
*/
	total =	aio_return( info->si_value.sival_ptr );
	printf("aio: total %d\n", total);
}


ssize_t fd_aio_read(int fd, void *buf, size_t len)
{
	ssize_t retval; 
	ssize_t nbytes; 
struct aiocb myaiocb;
	//Set up the signal handler
	struct sigaction action;
	action.sa_sigaction = aio_handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIG_AIO, &action, NULL);

	//struct aiocb myaiocb; 
	bzero( &myaiocb, sizeof (struct aiocb)); 
	myaiocb.aio_fildes = fd;
	myaiocb.aio_offset = 0; 
	myaiocb.aio_buf = buf; 
	myaiocb.aio_nbytes = len; //sizeof (buf); 
//	myaiocb.aio_lio_opcode = LIO_WRITE;
	//The signal to send, and the value of the signal
	myaiocb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	myaiocb.aio_sigevent.sigev_signo = SIG_AIO;
	//myaiocb.aio_sigevent.sigev_value.sival_int = 0;
	myaiocb.aio_sigevent.sigev_value.sival_ptr = &myaiocb;

	retval = aio_read( &myaiocb ); 
		if (retval == -1) { 
			perror("aio_read:"); 
			exit(1);
		}
	/* continue processing */  
	/* wait for completion */ 
	while ( (retval = aio_error( &myaiocb) ) == EINPROGRESS );
	
	int err = aio_error(&myaiocb);
    if (err != 0) {
                printf("aio_read err : %s\n", strerror(err));
           }
     
	/* free the aiocb */ 
	/*nbytes*/ total = aio_return( &myaiocb);
	printf("aio_read nbytes %d err %d retval %d\n", total/*nbytes*/, err, retval);
	return total; //nbytes;
}

ssize_t fd_aio_write(int fd, void *buf, size_t len)
{
	ssize_t retval; 
	ssize_t nbytes; 
	struct aiocb myaiocb; 

	bzero( &myaiocb, sizeof (struct aiocb)); 

	myaiocb.aio_fildes = fd;
	myaiocb.aio_offset = 0; 
	myaiocb.aio_buf = buf; 
	myaiocb.aio_nbytes = len; //sizeof (buf); 
	myaiocb.aio_sigevent.sigev_notify = SIGEV_NONE; 
	retval = aio_write( &myaiocb ); 
		if (retval == -1) { 
			perror("aio_write:"); 
			exit(1);
		} 
	/* continue processing */  
	/* wait for completion */ 
	while ( (retval = aio_error( &myaiocb) ) == EINPROGRESS) ; 
	int err = aio_error(&myaiocb);
	       if (err != 0)
                 printf("aio_write err : %s\n", strerror(err));
     
	/* free the aiocb */ 
	nbytes = aio_return( &myaiocb) ;
	printf("aio_read nbytes %d err %d retval %d\n", nbytes, err, retval);
	return nbytes;
}


long int calipso_aio_sendfile(int out_fd, int in_fd,  size_t size )
{
    /* block size */
#define BUFSZ (1024 * 4)

    char *buf;
    long int cc,ww;
    long int total;
    total = 0;

    while ( size > 0 ) {
        buf = malloc( BUFSZ );

        if (( cc = fd_aio_read( in_fd, buf, BUFSZ) ) < 0 ||
                ( ww = fd_aio_write( out_fd, buf, cc ) ) < 0 ) {
            return -1;
        } else if ( cc == 0 || out_fd < 0 ) {
			printf("AIO errno %d\n", errno);
            break;
        }

        total += cc;
        size -= cc;

        //printf("total byte's: %ld\n", total );

        free(buf);
    }

    return total;
}


#if FILE_AIO_TEST

#include <sys/fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
int main(int argc, char **argv)
{

    int in , out;
    long int n;
    struct stat status;

    static clock_t startclock, stopclock;

    float time_total;

    if (argc < 2) {
        printf("filename\n");
        return 1;
    }

    stat( argv[1] , &status );


    if ( (in = open ( argv[1] , O_RDONLY )) == 0 ) {
        perror("open()");
        return -1;
    }

    if ( (out = open ( "test" , O_CREAT | O_RDWR )) == 0 ) {
        perror("open()");
        return -1;
    }

//__off_t;
    printf("fd: %d, file sz: %ld\n", in, status.st_size);

    startclock = clock();

    n = calipso_aio_sendfile( out, in, status.st_size );

    stopclock = clock();

    time_total = ((float)( stopclock -startclock ) / (float)CLOCKS_PER_SEC) ;

    printf("stat %ld return %ld for %.4f sec - total: %.4f\n", status.st_size ,n ,time_total , ( time_total / (float)n) );

    close (in);

    close (out);

    return 0;
}

#endif

