#ifndef __WEBDAV_H
#define __WEBDAV_H

#define dav_contnet_type "application/xml"


typedef enum {
	PROPFIND,
	MKCOL,
	DELETE,
	PUT,
	COPY,
	MOVE,
	LOCK,
	UNLOCK,
	OPTIONS
} edav_methos;

typedef enum {
	INVALID		= -2,
	ZERO		=  0,
	ONE			=  1,
	INFINITY	= -1,
} edav_depth;

/**
	 * WebDAV HTTP Headers
	 */
#define  HEADER_COLL_MEMBER	("Collection-Member")

#define HEADER_DAV				("DAV")
#define HEADER_DAV_1			("1")
#define HEADER_DAV_2			("2")

#define HEADER_DEPTH			("Depth")
#define HEADER_DEPTH_0			("0")
#define HEADER_DEPTH_1			("1")
#define HEADER_DEPTH_INFINTY	("infinity")

#define HEADER_DESTINATION		("Destination")

#define HEADER_DESTROY			("Destroy")
#define HEADER_DESTROY_VER		("VersionDestroy")
#define HEADER_DESTROY_NO_UNDO	("NoUndelete")
#define HEADER_DESTROY_UNDO		("Undelete")

#define HEADER_ENFORCE_LIVE		("Enforce-Live-Properties_ _")
#define HEADER_ENFORCE_LIVE_ALL	("_*_")
#define HEADER_ENFORCE_LIVE_OMIT ("Omit")

#define HEADER_IF_NONE_STATE_MATCH ("If-None-State-Match")
	
#define HEADER_IF_STATE_MATCH	("If-State-Match")
#define HEADER_IF_STATE_MATCH_AND ("AND")
#define String HEADER_IF_STATE_MATCH_OR ("OR")

#define HEADER_LOCK_INFO			("Lock-Info")
#define HEADER_LOCK_TYPE			("LockType")
#define HEADER_LOCK_TYPE_WRITE	("Write")
#define HEADER_LOCK_SCOPE		("LockScope")
#define HEADER_LOCK_SCOPE_EXCL	("Exclusive")
#define HEADER_LOCK_SCOPE_SHR	("Shared")
#define HEADER_ADDITIONAL_LCOKS	("AddLocks")
#define HEADER_LOCK_TREE		("Lock-Tree")
#define HEADER_LOCK_TREE_TRUE	("T")
#define HEADER_LOCK_TREE_FALSE	("F")

#define HEADER_LOCK_TOKEN		("Lock-Token")

#define HEADER_OVERWRITE		("Overwrite")
#define HEADER_OVERWRITE_TRUE	("T")
#define HEADER_OVERWRITE_FALSE	("F")

#define HEADER_PROPFIND			("Propfind")
#define HEADER_PROPFIND_ALL		("allprop")
#define HEADER_PROPFIND_NAME	("propname")

#define HEADER_STATUS_URI		("Status-URI")
	
#define HEADER_TIMEOUT			("Timeout")
#define HEADER_TIMEOUT_INFINITE	("Infinite")

/* elem */
#define ELEM_PROPERTYUPDATE ("propertyupdate")
#define ELEM_SET 			("set")
#define ELEM_REMOVE 		("remove")

#define ELEM_MULTISTATUS 	("multistatus")
#define ELEM_RESPONSE		("response")
#define ELEM_STATUS 		("status")
#define ELEM_RESPONSEDESCRIPTION 	("responsedescription")
#define ELEM_HREF 			("href")
#define ELEM_LINK 			("link")
#define ELEM_SRC 			("src")
#define ELEM_DST 			("dst")
#define ELEM_PROP 			("prop")
#define ELEM_RESOURCE_TYPE 	("resourcetype")
#define ELEM_COLLECTION_RESOURCE_TYPE ("collection")

int webdav_invoke(calipso_request_t *request);

#endif 
