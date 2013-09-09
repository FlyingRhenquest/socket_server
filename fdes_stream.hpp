/**
 * Take an int fdes and provide a stream (G++-specific)
 *
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
 */

#ifndef _HPP_FDES_STREAM
#define _HPP_FDES_STREAM

#include <ext/stdio_filebuf.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <unistd.h>

namespace fr {

  namespace socket_server {

    /**
     * Gets streams from an fdes. You can retrieve both istreams and ostreams
     * for a file descriptor. Since streams can't be copied, you get
     * std::shared_ptrs. You can always convert them to a reference inside
     * your code if you want to. You can also get the fdes back out later
     * if you want to just pass this class around.
     *
     * This class will attempt to close the fdes in the destructor.
     * Generally it'd be a good idea to create a shared pointer to
     * it and pass that around. Then your fdes will just get closed
     * when the last pointer goes away.
     */

    class fdes_stream {
    public:
      // Some pointer defs for your convenience
      typedef std::shared_ptr<std::istream> istream_pointer;
      typedef std::shared_ptr<std::ostream> ostream_pointer;
      typedef std::shared_ptr<fdes_stream> pointer;

    private:
      typedef std::shared_ptr<__gnu_cxx::stdio_filebuf<char> > filebuf_pointer;

      int fdes;
      filebuf_pointer buf_in;
      filebuf_pointer buf_out;
      istream_pointer stream_in;
      ostream_pointer stream_out;
      
    public:

      static pointer get_pointer(int fdes)
      {
	return std::make_shared<fdes_stream>(fdes);
      }

      fdes_stream(int fdes) : fdes(fdes)
      {
	buf_in = std::make_shared<__gnu_cxx::stdio_filebuf<char > >(fdes, std::ios_base::in);
	buf_out = std::make_shared<__gnu_cxx::stdio_filebuf<char > >(fdes, std::ios_base::out);
	stream_in = std::make_shared<std::istream>(buf_in.get());
	stream_out = std::make_shared<std::ostream>(buf_out.get());
      }

      fdes_stream(const fdes_stream &copy) : fdes(copy.fdes), buf_in(copy.buf_in), buf_out(copy.buf_out), stream_in(copy.stream_in), stream_out(copy.stream_out)
      {
      }

      ~fdes_stream()
      {
	close(fdes);
      }

      int get_fdes() const
      {
	return fdes;
      }

      istream_pointer get_stream_in() const
      {
	return stream_in;
      }

      ostream_pointer get_stream_out() const
      {
	return stream_out;
      }

    };

  }

}

#endif
