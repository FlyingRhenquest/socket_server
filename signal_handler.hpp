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
 * Thread listens for signals and calls its owner's posix_signal_caught
 * signal if one is caught. Note that most of the signal handling
 * sorcery must be set up by its owner.
 */

#ifndef _HPP_SIGNAL_HANDLER
#define _HPP_SIGNAL_HANDLER

#include <errno.h>
#include "server_interface.hpp"
#include <signal.h>
#include <stdio.h>

namespace fr {

  namespace socket_server {

    class signal_handler {
      server_interface *owner;
      sigset_t *set;
      
    public:
      signal_handler(server_interface *owner, sigset_t *set) : owner(owner), set(set)
      {
      }

      signal_handler(const signal_handler &copy) : owner(copy.owner), set(copy.set)
      {
      }

      ~signal_handler()
      {
      }

      void operator()()
      {
	int retval;
	int sig;
	sigset_t set_copy;
	memcpy(&set_copy, set, sizeof(sigset_t));
	while(!owner->is_done()) {
	  retval = sigwait(set, &sig);
	  if (retval != 0) {
	    perror("sigwait");
	  } else {
	    owner->posix_signal_caught(sig);
	  }
	  if (!owner->is_done()) {
	    // reset set or you'll only catch one and not... them all...
	    // (But if you do it after owner->is_done, set may have
	    // gone away)
	    memcpy(set, &set_copy, sizeof(sigset_t));
	  }
	}
      }
    };
  }
}

#endif
