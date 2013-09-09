/**
 * Copyright 2013 Bruce Ide
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * This is the server body. It accepts a server_interface owner and a
 * port to listen on. It listens for a connection and spawns off a
 * service thread when one comes in. It will call its owner's
 * start_listening signal when (or just before) it starts listening,
 * and its owner's connection_request signal whenever a connection
 * request comes in.
 *
 * The programmer using this library should never need to have any
 * direct contact with the server body, unless he wants to write one
 * for UDP or named pipes or something.
 */

#ifndef _HPP_SERVER_BODY
#define _HPP_SERVER_BODY

#include "server_interface.hpp"
#include <sys/select.h>
#include <thread>

namespace fr {

  namespace socket_server {

    template <typename service_class>
    class server_body {
      server_interface *owner;
      int port;
      
    public:
      server_body(server_interface *owner, int port) : owner(owner), port(port)
      {
      }

      server_body(const server_body &copy) : owner(copy.owner), port(copy.port)
      {
      }

      ~server_body()
      {
      }

      void operator()()
      {
	int errbuflen = 255;
	int BACKLOG = 20;
	int retval = 0; // Used for checking status of various I/O calls
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	char errbuf[errbuflen];
	memset(errbuf, 0, errbuflen);

	if (0 > sock) {	  
	  strerror_r(errno, errbuf, errbuflen);
	  throw std::string(errbuf);
	}
	sockaddr_in addr;

	// Accept TCP/IP connections on port port on any
	// interface. If we wanted to limit where we were
	// listening, sin_addr.s_addr is where we'd do it
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	retval = bind(sock, (sockaddr *) &addr, sizeof(sockaddr_in));
	if (0 > retval) {
	  strerror_r(errno, errbuf, errbuflen);
	  throw std::string(errbuf);
	}
 
	// We can have BACKLOG connections waiting to be accepted
	// before (I assume) the system just stops accepting
	// them. The man page says the client "may" receive
	// an ECONNREFUSED error after that.
	retval = listen(sock, BACKLOG);

	if (0 > retval) {
	  strerror_r(errno, errbuf, errbuflen);
	  throw std::string(errbuf);
	}
	// Ok! We're listening! ..ish. Of course, we still have to
	// actually ACCEPT in-bound requests, but we're listening
	// for them!
	owner->start_listening();
	
	while(!owner->is_done()) {
	  // We poll our newly-created socket with a timeout of
	  // one second. This allows us to check every second to
	  // see if our owner was shut down and stop accepting
	  // connections if it was. We could also do additional
	  // processing here if we had any additional processing
	  // we wanted to do.
	  fd_set set;
	  timeval tv;
	  tv.tv_sec = 1;
	  tv.tv_usec = 0;
	  FD_SET(sock, &set); // Add socket to set
	  sockaddr_in incoming_address;
	  select(sock + 1, &set, NULL, NULL, &tv);
	  // Here we either got a connection or timed out.
	  // Check to see if we got a connection...
	  if (FD_ISSET(sock, &set)) {	    
	    // Yaar
	    socklen_t size;
	    int fdes = accept(sock, (sockaddr *) &incoming_address, &size);
	    if (0 > fdes) {
	      // I'm going to treat this as non-fatal
	      perror("Error accepting connection");
	    } else {
	      owner->connection_request((sockaddr *)&incoming_address);
	      service_class serve_it(owner, fdes, (sockaddr_in *) &incoming_address);
	      std::thread *thrd = new std::thread(serve_it);
	      thrd->detach();
	    }
	  }
	}
	
      }
    };

  };

}

#endif
