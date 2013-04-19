/* http.c - http defs
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

/*
 * from >> http://www.w3.org/Protocols/rfc2616/rfc2616.html
 * RFC 2616 Fielding, et al.
 * 10 Status Code Definitions
 */

#include "calipso.h"

struct http_status {
    int code;
    char *message;
};

static struct http_status status_codes[]= {
    {	100 ,	"Continue"				},
    {	101 ,	"Switching Protocols"	},
    {	200 ,	"OK"					},
    {	201 ,	"Created"				},
    {	202 ,	"Accepted"				},
    {   203 ,	"Non-Authoritative Information"	},
    {   204 ,	"No Content"			},
    {   205 ,	"Reset Content"			},
    {   206 ,	"Partial Content"		},
    {   300 ,	"Multiple Choices"		},
    {   301 ,	"Moved Permanently"		},
    {   302 ,	"Found"					},
    {   303 ,	"See Other"				},
    {   304 ,	"Not Modified"			},
    {   305 ,	"Use Proxy"				},
    {	307 ,	"Temporary Redirect"	},
    {   400	,	"Bad Request"			},
    {   401	,	"Unauthorized"			},
    {   402 ,	"Payment Required"		},
    {   403 ,	"Forbidden"				},
    {   404 ,	"Not Found"				},
    {   405 ,	"Method Not Allowed"	},
    {   406 ,	"Not Acceptable"		},
    {	407 ,	"Proxy Authentication Required"	},
    {   408 ,	"Request Time-out"		},
    {   409 ,	"Conflict"				},
    {   410 ,	"Gone"					},
    {   411 ,	"Length Required"		},
    {   412 ,	"Precondition Failed"		},
    {   413 ,	"Request Entity Too Large"	},
    {   414 ,	"Request-URI Too Large"		},
    {   415 ,	"Unsupported Media Type"	},
    {   416 ,	"Requested range not satisfiable"},
    {   417 ,	"Expectation Failed"	},
    {   500 ,	"Internal Server Error"	},
    {	501	,   "Not Implemented"		},
    {   502	,   "Bad Gateway"			},
    {   503 ,	"Service Unavailable"	},
    {	504 ,	"Gateway Time-out"		},
    {	505	,	"HTTP Version not supported"},
    { 	0 	,	NULL					},
};


char * calipso_http_status_get_message( int code )
{
    int i = 0;
    while (	status_codes[i].code != 0 || status_codes[i].message != NULL ) {
        if ( status_codes[i].code == code )
            return (status_codes[i].message);
        i++;
    }

    return( "UNKNOWN HTTP STATE" );
}

int calipso_http_status_is_info(int status)
{
    if (status / 100 == 1)
        return (1);

    return (0);
}

int calipso_http_status_is_successful(int status)
{
    if (status / 100 == 2)
        return (1);

    return (0);
}

int calipso_http_status_is_redirection(int status)
{
    if (status / 100 == 3)
        return (1);

    return (0);
}

int calipso_http_status_is_client_error(int status)
{
    if (status / 100 == 4)
        return (1);

    return (0);
}

int calipso_http_status_is_server_error(int status)
{
    if (status / 100 == 5)
        return (1);

    return (0);
}

int calipso_http_status_is_error(int status)
{
    return (calipso_http_status_is_client_error(status) ||
            calipso_http_status_is_server_error(status));
}

