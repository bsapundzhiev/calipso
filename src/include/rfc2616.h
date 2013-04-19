/*
 * from >> http://www.w3.org/Protocols/
 */
#ifndef RFC_H
#define RFC_H

#define HTTP_CONTINUE            		100   	//"Continue"
#define	HTTP_SWITCHING_PROTOCOLS		101   	//"Switching Protocols"
#define HTTP_OK           			200   	//"OK"
#define HTTP_CREATED           			201  	//"Created"
#define HTTP_ACEPTED           			202  	//"Accepted"
#define	HTTP_NON_AUTHORITIVE_INFORMATION	203   	//"Non-Authoritative Information"
#define	HTTP_NO_CONTENT           		204   	//"No Content"
#define HTTP_RESET_CONTENT          		205   	//"Reset Content"
#define	HTTP_PARTIAL_CONTENT           		206   	//"Partial Content"
#define	HTTP_MULTIPLE_CHOICES           	300   	//"Multiple Choices"
#define HTTP_MOVED_PERMANENTLY          	301   	//"Moved Permanently"
#define HTTP_FOUND          			302   	//"Found"
#define HTTP_SEE_OTHER          		303  	//"See Other"
#define HTTP_NOT_MODIFIED          		304   	//"Not Modified"
#define HTTP_USE_PROXY          		305   	//"Use Proxy"
#define HTTP_TEMPORARY_REDIRECT          	307  	//"Temporary Redirect"
#define HTTP_BAD_REQUEST          		400   	//"Bad Request"
#define HTTP_UNAUTHORIZED          		401   	//"Unauthorized"
#define HTTP_PAYMENT_REQUIRED          		402   	//"Payment Required"
#define HTTP_FORBIDDEN          		403   	//"Forbidden"
#define HTTP_NOT_FOUND          		404   	//"Not Found"
#define HTTP_METHOD_NOT_ALLOWED           	405   	//"Method Not Allowed"
#define HTTP_NOT_ACCEPTABLE			406   	//"Not Acceptable"
#define HTTP_PROXY_AUTHENTICATION_REQUIRED     	407   	//"Proxy Authentication Required"
#define HTTP_REQUEST_TIMEOUT          		408   	//"Request Time-out"
#define HTTP_CONFLICT				409   	//"Conflict"
#define HTTP_GONE		          	410   	//"Gone"
#define HTTP_LENGHT_REQUIRED	          	411   	//"Length Required"
#define HTTP_PRECONDITION_FAILED          	412   	//"Precondition Failed"
#define HTTP_REQUEST_ENTITY_TOO_LARGE          	413   	//"Request Entity Too Large"
#define HTTP_REQUEST_URI_TOO_LARGE		414   	//"Request-URI Too Large"
#define HTTP_UNSUPPORTED_MEDIA_TYPE		415   	//"Unsupported Media Type"
#define HTTP_REQUEST_RANGE_NOT_SATISFIABLE     	416   	//"Requested range not satisfiable"
#define HTTP_EXEPTION_FAILED          		417   	//"Expectation Failed"
#define HTTP_INTERNAL_SERVER_ERROR          	500   	//"Internal Server Error"
#define HTTP_NOT_IMPLEMENTED          		501   	//"Not Implemented"
#define HTTP_BAD_GATEWAY	          	502   	//"Bad Gateway"
#define HTTP_SERVICE_UNAVAILABLE		503   	//"Service Unavailable"
#define HTTP_GATEWAI_TIMEOUT          		504  	//"Gateway Time-out"
#define HTTP_VERSION_NOT_SUPPORTED          	505   	//"HTTP Version not supported"

/*
        Request-Line   = Method SP Request-URI SP HTTP-Version CRLF


       Method         = "OPTIONS"                ; Section 9.2
                      | "GET"                    ; Section 9.3
                      | "HEAD"                   ; Section 9.4
                      | "POST"                   ; Section 9.5
                      | "PUT"                    ; Section 9.6
                      | "DELETE"                 ; Section 9.7
                      | "TRACE"                  ; Section 9.8
                      | "CONNECT"                ; Section 9.9
                      | extension-method
       extension-method = token
*/

#endif /*!RFC_H*/
