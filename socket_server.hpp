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
 * A class that listens on a socket and executes a specific service
 * class when a connection is received. The service class can be
 * any class which implements void operator()() and which accepts
 * a server_interface * (The owner of that service,) an int file
 * descriptor (The fdes of the socket for it to communicate on) and
 * a sockaddr_in * incoming address structure (You can look at its
 * sin_family to determine if it's an AF_INET or an AF_INET6 if
 * you're concerned about it, or you can just ignore it altogether
 * if you don't need to know.)
 */

#ifndef _HPP_SOCKET_SERVER
#define _HPP_SOCKET_SERVER

#include "server_interface.hpp"
#include "server_body.hpp"
#include "signal_handler.hpp"

namespace fr {

  namespace socket_server {

    // I dunno, that name seems a bit... redundant...

    template <typename service_class,
	      typename handler_class = signal_handler,
	      typename body_class = server_body<service_class> >
    class socket_server : public server_interface {
      int port;
      std::thread *body_thread;
      std::thread *sig_handler_thread;
      bool done;
    public:
      socket_server(int port) : port(port), body_thread(nullptr), sig_handler_thread(nullptr), done(false)
      {
      }

      ~socket_server()
      {
      }

      std::thread *start() override
      {
	// Alert listeners the show is about to begin
	server_start();

	// Must block off all blockable signals prior to spawning
	// children
	sigset_t set;
	sigfillset(&set); // all signals
	
	int sig_ret = pthread_sigmask(SIG_SETMASK, &set, NULL);
	if (sig_ret != 0) {
	  // Honestly it's never going to be, though.
	  perror("Error blocking signals");
	}
	

	// Install signal catching thread
	handler_class handler(this, &set);
	sig_handler_thread = new std::thread(handler);
	
	body_class body(this, port);
	// Note that std::thread will copy it, so if you're doing something
	// esoteric and non-copyable in your body, you need to act accordingly
	body_thread = new std::thread(body);
	return body_thread;
      }

      void join() override
      {
	sig_handler_thread->join();
	body_thread->join();
      }

      void shutdown() override
      {
	done = true;
	// Send myself a usr1 so signal handler exits
	pid_t me = getpid();
	kill(me,SIGUSR1);
      }

      bool is_done() override
      {
	return done;
      }

    };

  }
}

#endif
