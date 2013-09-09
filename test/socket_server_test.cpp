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
 * Test socket server by establishing a socket server with an echo server,
 * then connecting to it, sending some data through to the far side
 * and making sure it gets echoed back correctly.
 */

#include <boost/bind.hpp>
#include <cppunit>
#include "fdes_stream.hpp"
#include <iostream>
#include "socket_server.hpp"
#include <string>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

class socket_server_test : public test_class {

  CPPUNIT_TEST_SUITE(socket_server_test);
  CPPUNIT_TEST(echo_test);
  CPPUNIT_TEST_SUITE_END();

  class echo_service {
    fr::socket_server::server_interface *owner;
    int fdes;
    
  public:
    // We're just going to ignore remote address for this test
    echo_service(fr::socket_server::server_interface *owner, int fdes, sockaddr_in *remote_address) : owner(owner), fdes(fdes)
    {
    }

    echo_service(const echo_service &copy) : owner(copy.owner), fdes(copy.fdes)
    {
    }

    ~echo_service()
    {
    }

    void operator()()
    {
      // When we go out of scope this will close the file descriptor
      fr::socket_server::fdes_stream::pointer stream_converter = fr::socket_server::fdes_stream::get_pointer(fdes);
      std::istream &stream_in = *(stream_converter->get_stream_in().get());
      std::ostream &stream_out = *(stream_converter->get_stream_out().get());
      while(stream_in.good()) {
	std::string line;
	std::getline(stream_in, line);
	stream_out << line << std::endl;
      }
    }
  };
  
  bool server_is_listening;

  void server_listening_callback()
  {
    server_is_listening = true;
  }


public:

  void echo_test()
  {
    const int port=13000;
    server_is_listening = false;
    fr::socket_server::socket_server<echo_service> server(port);
    server.start_listening.connect(boost::bind(&socket_server_test::server_listening_callback, this));

    server.start();
    while(!server_is_listening) {
      // Probably don't technically NEED this, but it does a pretty
      // nice job of insuring I don't get random test failures due
      // to timing problems.
      std::this_thread::yield();
    }
    // Presumably server is now listening
    // On the server side you go socket, listen, accept. On the
    // client side you go socket, connect, write.
    int fdes;
    // Hmm, TODO: Write socket_client heh heh heh
    fdes = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > fdes) {
      CPPUNIT_FAIL("Error opening socket");
    }
    hostent *server_host;
    server_host = gethostbyname("localhost");
    if (nullptr == server_host) {
      CPPUNIT_FAIL("Gethostbyname failed to look up 127.0.0.1");
    }
    sockaddr_in server_address;
    memset((void *)&server_address, 0, sizeof(sockaddr_in));
    server_address.sin_family = AF_INET;
    memcpy((void *)(&server_host->h_addr), (const void *)&server_address.sin_addr.s_addr, server_host->h_length);
    server_address.sin_port = htons(port);
    if (0 > connect(fdes, (struct sockaddr *) &server_address, sizeof(sockaddr_in))) {
      CPPUNIT_FAIL("Error connecting to server");
    }

    fr::socket_server::fdes_stream::pointer stream_converter = fr::socket_server::fdes_stream::get_pointer(fdes);
    std::istream &stream_in = *(stream_converter->get_stream_in().get());
    std::ostream &stream_out = *(stream_converter->get_stream_out().get());
    std::string expected("Heyo!");
    stream_out << expected << std::endl;
    std::string actual;
    std::getline(stream_in, actual);
    CPPUNIT_ASSERT(expected == actual);
    server.shutdown();
    server.join();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(socket_server_test);
