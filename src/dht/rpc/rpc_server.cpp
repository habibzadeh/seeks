/**
 * This is the p2p messaging component of the Seeks project,
 * a collaborative websearch overlay network.
 *
 * Copyright (C) 2009  Emmanuel Benazera, juban@free.fr
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rpc_server.h"
#include "spsockets.h"
#include "errlog.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <errno.h>

using sp::errlog;
using sp::spsockets;

namespace dht
{
   rpc_server::rpc_server(const std::string &hostname, const short &port)
     :_na(hostname,port)
     {     
     }
   
   rpc_server::~rpc_server()
     {
     }
   
   dht_err rpc_server::run()
     {
	// create socket.
	int udp_sock = socket(AF_INET,SOCK_DGRAM,0);
	
	struct sockaddr_in server;
	size_t length = sizeof(server);
	bzero(&server,length);
	server.sin_family = AF_INET; // beware, should do AF_INET6 also.
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(_na.getPort());
	
	// bind socket.
	int bind_error = bind(udp_sock,(struct sockaddr*)&server,length);
	if (bind_error < 0)
	  {
#ifdef _WIN32
	     errno = WSAGetLastError();
	     if (errno == WSAEADDRINUSE)
#else
	       if (errno == EADDRINUSE)
#endif
		 {
		    spsockets::close_socket(udp_sock);
		    errlog::log_error(LOG_LEVEL_FATAL, "rpc_server: can't bind to %s:%d: "
				      "There may be some other server running on port %d",
				      _na.getNetAddress().c_str(),
				      _na.getPort(), _na.getPort());
		    // TODO: throw exception instead.
		    return(-3);
		 }
	     else
	       {
		  spsockets::close_socket(udp_sock);
		  errlog::log_error(LOG_LEVEL_FATAL, "can't bind to %s:%d: %E",
				    _na.getNetAddress().c_str(), _na.getPort());
		  // TODO: throw exception instead.
		  return(-1);
	       }
	  }
	
	// get messages, one by one.
	size_t buflen = 1024;
	char buf[buflen];
	struct sockaddr_in from;
	size_t fromlen = sizeof(struct sockaddr_in);
	while(true)
	  {
	     int n = recvfrom(udp_sock,buf,buflen,0,(struct sockaddr*)&from,&fromlen);
	     if (n < 0)
	       {
		  errlog::log_error(LOG_LEVEL_ERROR, "recvfrom: error receiving DGRAM message, %E");
	       }
	     
	     // message.
	     errlog::log_error(LOG_LEVEL_LOG, "rpc_server: received a datagram");
	     std::string dtg_str = std::string(buf);
	     
	     // TODO: produce and send a response.
	     // sendto(sock,...).
	     
	     try
	       {
		  std::string resp_msg;
		  serve_response(dtg_str,resp_msg);
	       }
	     catch (dht_exception ex)
	       {
		  errlog::log_error(LOG_LEVEL_LOG, "rpc_server exception: %ex", ex.what().c_str());
	       
		  // TODO: error decision.
	     	  // TODO: rethrow something.
	       }
	     
	  } // end while server listening loop.
	
	return DHT_ERR_OK;
     }

   dht_err rpc_server::serve_response(const std::string &msg,
				      std::string &resp_msg)
     {
	return DHT_ERR_OK;
     }
      
} /* end of namespace. */

  