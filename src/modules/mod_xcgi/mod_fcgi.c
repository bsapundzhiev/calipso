#include <stdio.h>
#include <calipso.h>
#include "mod_fcgi.h"
#include "fastcgi.h"
#include "chunks.h"

#define FCGI_SERVER_ADDR "127.0.0.1"
#define CFGI_SERVER_PORT 9000

typedef struct _FCGI_PARAM
{
    char *	Name;
    int16_t	NameLen;
    char *	Value;
    int16_t	ValueLen;
} FCGI_PARAM;

FCGI_PARAM g_pParamsBuffer[] =
{
    { "HTTP_ACCEPT", sizeof( "HTTP_ACCEPT" )-1, "*/*", sizeof( "*/*" )-1 },
    { "HTTP_HOST", sizeof( "HTTP_HOST" )-1, "localhost", sizeof( "localhost" )-1 },
    { "REQUEST_METHOD", sizeof( "REQUEST_METHOD" )-1, "GET", sizeof( "GET" )-1 },
    { "CONTENT_LENGTH", sizeof( "CONTENT_LENGTH" )-1, "0", sizeof( "0" )-1 },
    { "HTTP_HOST", sizeof( "HTTP_HOST" )-1, "localhost", sizeof( "localhost" )-1 },
    { "HTTPS", sizeof( "HTTPS" )-1, "off", sizeof( "off" )-1 },
    { "INSTANCE_ID", sizeof( "INSTANCE_ID" )-1, "1", sizeof( "1" )-1 },
    { "INSTANCE_META_PATH", sizeof( "INSTANCE_META_PATH" )-1, "/LM/W3SVC/1", sizeof( "/LM/W3SVC/1" )-1 },
    { "REMOTE_ADDR", sizeof( "REMOTE_ADDR" )-1, "127.0.0.1", sizeof( "127.0.0.1" )-1 },
    { "REMOTE_HOST", sizeof( "REMOTE_HOST" )-1, "127.0.0.1", sizeof( "127.0.0.1" )-1 },
    { "SERVER_NAME", sizeof( "SERVER_NAME" )-1, "localhost", sizeof( "localhost" )-1 },
    { "SERVER_PORT", sizeof( "SERVER_PORT" )-1, "80", sizeof( "80" )-1 },
    { "SERVER_PORT_SECURE", sizeof( "SERVER_PORT_SECURE" )-1, "0", sizeof( "0" )-1 },
    { "SERVER_PROTOCOL", sizeof( "SERVER_PROTOCOL" )-1, "HTTP/1.1", sizeof( "HTTP/1.1" )-1 },
    { "SERVER_SOFTWARE", sizeof( "SERVER_SOFTWARE" )-1, "Microsoft-IIS/6.0", sizeof( "Microsoft-IIS/6.0" )-1 }
};

FCGI_PARAM g_pSubstituteParams[] =
{
    { "SCRIPT_NAME", sizeof( "SCRIPT_NAME" )-1, "", 0 },
    { "PATH_TRANSLATED", sizeof( "PATH_TRANSLATED" )-1, "", 0 },
	{ "QUERY_STRING", sizeof("QUERY_STRING"-1),"", 0 },
    { "URL", sizeof( "URL" )-1, "", 0 }
};


static int SendParams(int socket , calipso_request_t *request, const char * filename);
static int ReceiveLoop(int socket, calipso_request_t *request);

static int parse_hedaer_buf(calipso_request_t *request , char *buf)
{
  char * tch;
  char * str = buf;
  tch = strtok (str, "\n\r");
  while (tch != NULL)
  { 
    printf ("'%s'\n",tch);
	//mod_xcgi_parse_cgi_header(request, tch);
    tch = strtok (NULL, "\n\r");
  }
	return 1;
}

int mod_xcgi_fcgi_exec(calipso_request_t *request, const char * filename)
{
	int sockfd, n;
    struct sockaddr_in serv_addr;

	calipso_reply_t *reply;
	calipso_resource_t *resource;
	
    /* Protocol Messages */
    
    FCGI_BeginRequestRecord Message_BeginRequest = 
    { 
        { FCGI_VERSION_1, FCGI_BEGIN_REQUEST, 0xAA, 0xAA, 0, 8 }, // reqid=AA AA, body=00 08 bytes
        { 0, FCGI_RESPONDER, FCGI_KEEP_CONN } 
    };

    FCGI_Header Message_StdIn   = { FCGI_VERSION_1, FCGI_STDIN,  0xAA, 0xAA };

	reply = calipso_request_get_reply(request);
	resource = calipso_reply_get_resource(reply);
	/* dont use resource */
	if(resource->resource_fd != -1) {
		close(resource->resource_fd);
		calipso_resource_set_file_descriptor(resource, -1);
	}
	
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
        perror("ERROR opening socket");
		calipso_reply_set_status(reply, HTTP_BAD_GATEWAY);
		return(0);
	}
 
	set_keep_alive(sockfd, 1);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(FCGI_SERVER_ADDR); 
    serv_addr.sin_port = htons(CFGI_SERVER_PORT);

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ 
        perror("ERROR connecting");
		calipso_reply_set_status(reply, HTTP_BAD_GATEWAY);
		return 0;
	}

	n = send(sockfd, (unsigned char*)&Message_BeginRequest, sizeof( Message_BeginRequest ) ,0);
   
	printf("n %d\n", n);

	/*if( 0 != n )
    {
        printf( "SendParams failed\n" );
        goto Finished;
    }
	if( sizeof( Message_BeginRequest ) != n )
    {
        printf( "incorrect Message_BeginRequest WriteFile size!\n" );
    }*/

    n = SendParams(sockfd,  request,filename);

    if( 0 != n )
    {
        printf( "SendParams failed\n" );
        goto Finished;
    }
    printf( "FCGI_PARAMS sent\n" );

    if( n = send( sockfd,(unsigned char*) &Message_StdIn, sizeof( Message_StdIn ), 0 ) )
    {
        //dwError = GetLastError( );
        //printf( "WriteFile Message_StdIn failed\n" );
        //goto Finished;
    }

    if( sizeof( Message_StdIn ) != n )
    {
        printf( "incorrect Message_StdIn WriteFile size!\n" );
    }
    printf( "FCGI_STDIN sent\n" );

    printf( "Launching receive loop\n" );
    n = ReceiveLoop( sockfd , request);
    if( 0 != n )
    {
        printf( "ReceiveLoop failed\n" );
        goto Finished;
    }

Finished:

    closesocket(sockfd);

	return CPO_OK;
}

static int SendParams(int socket , calipso_request_t *request,const char * filename)
{

	uint16_t        dwRet           = 0;
   
    uint16_t        cbTotal         = 0;
    FCGI_Header     Message_Params  = { FCGI_VERSION_1, FCGI_PARAMS, 0xAA, 0xAA };
    char*           pBuffer         = NULL;
    FCGI_Header*    pHeader         = NULL;
    char*           pWriter         = NULL;
    uint16_t        dwBytesWritten  = 0;
	int i;

	pBuffer = malloc( strlen(filename) );
    if( NULL == pBuffer )
    {
       dwRet = ERROR_OUTOFMEMORY;
       goto Finished;
    }
    *pBuffer = '/';
    memcpy( pBuffer + 1, filename, strlen( filename ) );

    g_pSubstituteParams[0].Value    = pBuffer;
    g_pSubstituteParams[0].ValueLen = strlen( pBuffer );
    g_pSubstituteParams[1].Value    = (char*)filename;
    g_pSubstituteParams[1].ValueLen = strlen( filename );
	if(request->querystring) {
		printf("Query string %s\n",request->querystring );
	g_pSubstituteParams[2].Value    = request->querystring;
    g_pSubstituteParams[2].ValueLen = strlen( request->querystring );
	}
	g_pSubstituteParams[3].Value    = request->uri;
	g_pSubstituteParams[3].ValueLen = strlen( request->uri );

	for(i = 0; i < sizeof( g_pParamsBuffer ) / sizeof( g_pParamsBuffer[0] ); i++ )
    {
        if(
            g_pParamsBuffer[i].NameLen > 255 ||
            g_pParamsBuffer[i].ValueLen > 255
        )
        {
            dwRet = ERROR_INVALID_DATA;
            printf( "don't support params > 255\n" );
            goto Finished;
        }
        cbTotal += g_pParamsBuffer[i].NameLen + g_pParamsBuffer[i].ValueLen + 2;
    }
    for( i = 0; i < sizeof( g_pSubstituteParams ) / sizeof( g_pSubstituteParams[0] ); i++ )
    {
        if(
            g_pSubstituteParams[i].NameLen > 255 ||
            g_pSubstituteParams[i].ValueLen > 255
        )
        {
            dwRet = ERROR_INVALID_DATA;
            printf( "don't support params > 255\n" );
            goto Finished;
        }
        cbTotal += g_pSubstituteParams[i].NameLen + g_pSubstituteParams[i].ValueLen + 2;
    }

    pBuffer = malloc( sizeof( FCGI_Header ) + cbTotal );
    if( NULL == pBuffer )
    {
        dwRet = ERROR_OUTOFMEMORY;
        goto Finished;
    }

    /* Fill in header*/
   
    pHeader = (FCGI_Header*)pBuffer;
    pHeader->version            = FCGI_VERSION_1;
    pHeader->type               = FCGI_PARAMS;
    pHeader->requestIdB0        = 0xAA;
    pHeader->requestIdB1        = 0xAA;
    pHeader->contentLengthB0    = cbTotal % 256;
    pHeader->contentLengthB1    = cbTotal / 256;
    pHeader->paddingLength      = 0;
    pHeader->reserved           = 0;

	pWriter = pBuffer + sizeof( FCGI_Header );

    for( i = 0; i < sizeof( g_pParamsBuffer ) / sizeof( g_pParamsBuffer[0] ); i++ )
    {
        *pWriter = g_pParamsBuffer[i].NameLen;
        pWriter++;

        *pWriter = g_pParamsBuffer[i].ValueLen;
        pWriter++;

        CopyMemory( pWriter, g_pParamsBuffer[i].Name, g_pParamsBuffer[i].NameLen );
        pWriter += g_pParamsBuffer[i].NameLen;

        CopyMemory( pWriter, g_pParamsBuffer[i].Value, g_pParamsBuffer[i].ValueLen );
        pWriter += g_pParamsBuffer[i].ValueLen;
    }
    for( i = 0; i < sizeof( g_pSubstituteParams ) / sizeof( g_pSubstituteParams[0] ); i++ )
    {
        *pWriter = g_pSubstituteParams[i].NameLen;
        pWriter++;

        *pWriter = g_pSubstituteParams[i].ValueLen;
        pWriter++;

        CopyMemory( pWriter, g_pSubstituteParams[i].Name, g_pSubstituteParams[i].NameLen );
        pWriter += g_pSubstituteParams[i].NameLen;

        CopyMemory( pWriter, g_pSubstituteParams[i].Value, g_pSubstituteParams[i].ValueLen );
        pWriter += g_pSubstituteParams[i].ValueLen;
    }

    //
    // Finally, send it off
    //
    if( dwBytesWritten = send( socket, pBuffer, sizeof( FCGI_Header ) + cbTotal, 0 ) )
    {
        dwRet = GetLastError( );
        printf( "WriteFile SendParams failed\n" );
        goto Finished;
    }
    if( sizeof( FCGI_Header ) + cbTotal != dwBytesWritten )
    {
        printf( "incorrect SendParams WriteFile size!\n" );
    }

Finished:

	if(pBuffer) {
		free(pBuffer);
	}

    return dwRet;
}

int ReceiveLoop(int socket, calipso_request_t *request)
{
	DWORD   dwError         = 0;
    BOOL    fEnd            = FALSE;
    DWORD   dwBytesRead     = 0;
    DWORD   dwResponseSize  = 0;
    PBYTE   pBuffer         = NULL;

    FCGI_Header Message_Response;

	int header_parse = 0;
	int total =0;

	struct mpool * pool = request->reply->pool;
	struct chunk_ctx * cb  = chunk_ctx_alloc(pool);

    while( !fEnd )
    {
        memset( &Message_Response, 0 , sizeof( Message_Response ) );

		dwBytesRead = recv(socket,(unsigned char*)&Message_Response, sizeof(FCGI_Header), 0);

        if( 8 != dwBytesRead )
        {
            dwError = ERROR_INVALID_DATA;
            printf( "partial header received\n" );
            break;
        }

        dwResponseSize = 
            ( Message_Response.contentLengthB1 << 8 )
            + Message_Response.contentLengthB0 
            + Message_Response.paddingLength;

		printf("dwResponseSize %d\n", dwResponseSize);
        printf(" clen %d\n", ( Message_Response.contentLengthB1 << 8 ) + Message_Response.contentLengthB0);
        if( dwResponseSize > 0 )
        {
			pBuffer = malloc( dwResponseSize+1 );
            if( NULL == pBuffer )
            {
                dwError = ERROR_OUTOFMEMORY;
                break;
            }
		total = dwResponseSize;
		//while(total > 0) 
		{

			dwBytesRead = recv( socket, pBuffer, total, 0  );
			printf("dwBytesRead %d\n" , dwBytesRead);
            if( ! dwBytesRead )
            {
                //dwError = GetLastError( );
                printf( "ReadFile failed\n" );
                break;
            }

            if( dwResponseSize != dwBytesRead )
            {
                dwError = ERROR_INVALID_DATA;
				
                printf( "dwResponseSize %d != dwBytesRead %d total %d\n", dwResponseSize, dwBytesRead, total );
                break;
				//pBuffer += dwBytesRead;
				//total -=dwBytesRead;

            }
			
		}
            pBuffer[( Message_Response.contentLengthB1 << 8 ) + Message_Response.contentLengthB0] = '\0';
			
        }

        switch( Message_Response.type )
        {

		case FCGI_STDOUT:{
           // printf( "FCGI_STDOUT: %s\n", pBuffer );
				if(!header_parse)
					header_parse = parse_hedaer_buf(request, pBuffer);
				else {
					int len = ( Message_Response.contentLengthB1 << 8 ) + Message_Response.contentLengthB0;
					printf("len %d\n", len);
					chunk_ctx_append(pool, cb, pBuffer , len);
				}
			} break;

        case FCGI_END_REQUEST:
            printf( "FCGI_END_REQUEST received\n" );
            fEnd = TRUE;
            break;

        default:
            dwError = ERROR_INVALID_DATA;
            printf( "unexpected Message_Response.type=%d\n", Message_Response.type );
            
            break;

        }

        if( NULL != pBuffer )
        {
            free( pBuffer );
            pBuffer = NULL;
        }
    }

	CNUNKS_ADD_TAIL_CTX(request->reply->out_filter, cb);
	//chunks_dump(request->reply->out_filter, 0);
	return CPO_OK;
}
