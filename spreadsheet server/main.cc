#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "server.h"

int main(int argc, char* argv[])
{

  std::cout << "Inside of main\n";
  std::cout << argv[0] <<"\n";
  
  if(argc > 1)
    std::cout << argv[1] << std::endl;
	std::cout << "Just printed out the port.\n";
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

	std::cout << "Going to craete the io_service.\n";
    boost::asio::io_service io_service;
    std::cout << "Just created the io_service.\n";

    ss_server::server s(std::atoi(argv[1]), io_service);
    std::cout << "Starting server.\n";
    io_service.run();
    std::cout << "Stopping server.\n";
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
