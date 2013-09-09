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
 * Server interface. Defines all the things a server can do. Any
 * class that implements this can be an owner for a server body
 * and service classes.
 */

#ifndef _HPP_SERVER_INTERFACE
#define _HPP_SERVER_INTERFACE

#include <arpa/inet.h>
#include <boost/signals2/signal.hpp>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace fr {

  namespace socket_server {

    class server_interface {
      
    public:

      // Called when start is called. This is to alert listeners that the
      // server is about to start
      boost::signals2::signal<void ()> server_start;

      // Signals that the listener has started listening
      boost::signals2::signal<void ()> start_listening;
      
      // Signals that a connection request has taken place. This occurs
      // right after accept(), with the address and length of the address
      // structure from accept.
      boost::signals2::signal<void (struct sockaddr *)> connection_request;

      // Since I'm using threads, a signal arising in any thread
      // can be handled by any thread. This can result in things
      // like my main thread exiting when a pipe gets closed on
      // a service thread. That's inconvenient. So the server
      // should intercept signals. Rather than just do that quietly,
      // I'll add a signal for it. Any interested parties can
      // attach to the signal.
      boost::signals2::signal<void (int signal)> posix_signal_caught;

      // Start listening for connections. Returns a listener thread pointer
      // in case you want to join it later or something.
      virtual std::thread *start() = 0;

      // Wait for listener body to complete. You don't need to call this,
      // if you want to do additional processing in your main thread. if
      // you just want to wait and exit, you can use this.
      virtual void join() = 0;

      // Shut server down cleanly -- this signals the server body that a
      // shutdown has been requested. The body in turn can stop processing,
      // finish processing current connections and exit.
      virtual void shutdown() = 0;

      // Will return false until shutdown is called. Once shutdown
      // is called, will return true.

      virtual bool is_done() = 0;

    };

  }

}

#endif
