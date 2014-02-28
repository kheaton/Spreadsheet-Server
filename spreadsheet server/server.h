#ifndef _SERVER_H_
#define _SERVER_H_

#include "ss_session.h"

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace ss_server
{
  class ss_session{};
  class server;

   class session
   {
   public:
   //private:
     //socket that this session is working on
     boost::asio::ip::tcp::socket socket_;
     
     //length of the buffers
     enum {buffer_length = 2048};
     
     //buffer
     char buffer_[buffer_length];
     
     //pointer to the host server	
     server *server_ptr_;
     
     //io_service so we can make more threads
     boost::asio::io_service& io_service_;

     friend class server;
  
   public:
     //private:
     session(boost::asio::io_service& io_serv, server *server_ptr);
     boost::asio::ip::tcp::socket& socket();
     
     //Function called to start reading from socket in session
     void start_reading();
     
      void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred);
      
      void handle_write(const boost::system::error_code& error);
      
     //Function to see if spread sheet already exists
     bool spreadsheet_exists(std::string spreadsheet_name)const;
     
     //checks if password is valid for document
     bool password_is_valid(std::string doc_name, std::string doc_password)const;
     
     //fucntion to see if there is a session already running for
     //specific spreadsheet
     bool ss_session_in_progress(std::string spreadsheet_name)const;
     
     //fuction will take the socket and it's reques and call the appropriate
     //fucntions to handle request
     void parse_request(const boost::system::error_code& error, size_t bytes_transferred);
     
     //fucntion to allow the request to be parsed from a write async function
     void begin_read(const boost::system::error_code& error);
     
     //funciton will handle create requests
     void create_requested(std::vector<std::string> parsed_result);
     
     //function will handle join requests
     void join_requested(std::vector<std::string> parsed_result);
     
     //function willl handle all non valid requests from socket
     void other_requested();
     
     //funciton to close the socket and deletes the current session. 
     //Allows for it to be called as a callback from asynchronous operation.
     //This happens on errors. Overloaded version allows for same functionality
	 //without being a callback.
	 void close_socket(const boost::system::error_code& error);
	 void close_socket();
	 
   };

   class server
  {
    /////////////////////////////////////////////////////////////////////
    ////////
    ////////
    //////               switch back to private
    /////////
    /////
    //////////////////////////////////////////////////////////////////////
  public:
    //private:
    /*------------------- Private Member Variables -------------------- */
    //The external io_service the server runs on, it is the thread
    //we will use to accept all incoming socket requests
    boost::asio::io_service io_service_;
    
    //port number the server is listening to
    int port_number_;
    
    //Hash Set for the ss_sessions
    boost::unordered_map<std::string, ss_session*>ss_sessions_;
    
    //hash set for the regular sessions
    boost::unordered_map<std::string, session*> sessions_;
    //std::map<std::string, session> sessions_;
    
    //acceptor to receive incoming connections
    boost::asio::ip::tcp::acceptor acceptor_;
    
    //pointer to hold the current session being worked on
    session *my_session;

    /*-------------------- Private Functions ------------------------------*/
    //Function called when socket is accepted
    void accept_handler(session* my_session, const boost::system::error_code& error);

	//function that will allow a session to kill itself
    void kill_session(std::string session_name);

    //so session class can call function to kill itself
    friend class ss_session;
    
    //friend class used for testing
    friend class testclass;

  public:
    //constructor
    server(int port_number, boost::asio::io_service& io_serv);
  };

}

#endif
