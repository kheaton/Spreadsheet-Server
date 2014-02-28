#include "server.h"
#include <iostream>

//using boost::asio::ip::tcp;

namespace ss_server
{
	bool DEBUG = true;
	
	void print_debug_message(std::string msg)
	{
		if(DEBUG)
			std::cout << msg << std::endl;
	}
	
  ///////////////////////////////////////////////////////////////////////////////
  //-----------------------Session Functions-----------------------------------//
  ///////////////////////////////////////////////////////////////////////////////
  
  //session constructor
  //initialize the io_service and socket inline

  //session(boost::asio::io_service& io_serv): socket_(io_serv)

  session::session(boost::asio::io_service& io_serv, server *server_ptr):socket_(io_serv), io_service_(io_serv)
  {
	print_debug_message("Inside session constructor");
    //save reference to the host server
    server_ptr_ = server_ptr;
  }

  boost::asio::ip::tcp::socket& session::socket()
  {
    return this->socket_;
  }

  //Function called to start reading from socket in session
  void session::start_reading()
  {
	  print_debug_message("Start reading from socket");
	  
      socket_.async_read_some(boost::asio::buffer(buffer_, buffer_length),
         boost::bind(&session::parse_request, this,
           boost::asio::placeholders::error,
           boost::asio::placeholders::bytes_transferred));
  }
  
  //fuction will take the socket and it's reques and call the appropriate
  //fucntions to handle request
  void session::parse_request(const boost::system::error_code& error, size_t bytes_transferred)
  {
	print_debug_message("Inside parse request");
	print_debug_message("buffer_");
		
	//Set up a vector and split up the string based on : and '\n'
	std::vector<std::string> parsed_result;
	std::string initial = buffer_;
	boost::algorithm::split(parsed_result, initial, boost::algorithm::is_any_of("\n, :"));
	
	//Parse out the split up string, and call appropriate functions
	if((parsed_result[0] == "CREATE") && (parsed_result[1] == "Name") 
		&& (parsed_result[3] == "Password"))
		create_requested(parsed_result);
	else if((parsed_result[0] == "JOIN") && (parsed_result[1] == "Name") 
		&& (parsed_result[3] == "Password"))
		join_requested(parsed_result);
	else
		other_requested();	
  }
  
  //fucntion to do the parse request from a write function
  void session::begin_read(const boost::system::error_code& error)
  {
	  print_debug_message("Start reading from socket in begin_read");
	  
      socket_.async_read_some(boost::asio::buffer(buffer_, buffer_length),
         boost::bind(&session::parse_request, this,
           boost::asio::placeholders::error,
           boost::asio::placeholders::bytes_transferred));
  }
  
  void session::handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
		std::cout << "Read from socket: " << buffer_ << std::endl;
      boost::asio::async_write(socket_,
          boost::asio::buffer(buffer_, buffer_length),
          boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error));
    }
    else
    {
      delete this;
    }
  }
  
  void session::handle_write(const boost::system::error_code& error)
  {
	print_debug_message("inside handle write");
	
    if (!error)
    {
      socket_.async_read_some(boost::asio::buffer(buffer_, buffer_length),
          boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
  }

  //Function to see if spread sheet already exists
  bool session::spreadsheet_exists(std::string spreadsheet_name)const
  {
	print_debug_message("Checking if " + spreadsheet_name + " exists.");  
	  
    //create filestream and try and open the passed in file. 
    std::fstream file;
    file.open(spreadsheet_name.c_str(), std::ios_base::out | std::ios_base::in);
    //if is open, then it exists, close and return true;
    if(file.is_open())
    {
	  print_debug_message(spreadsheet_name + " does exist");
      file.close();
      return true;
    }
    //else return false
    print_debug_message(spreadsheet_name + " does not exist");
    return false;
  }
  
  //checks if password is valid for document
  bool session::password_is_valid(std::string doc_name, std::string doc_password)const
  {
	  print_debug_message("Verify Password:\nDoc Name: " + doc_name
							+ "\nDoc Password: " + doc_password);
	  
	  //open up the document
	  std::fstream file;
	  file.open(doc_password.c_str());
	  
	  //create tree from the xml
	  boost::property_tree::ptree doc_tree;
	  boost::property_tree::xml_parser::read_xml(file, doc_tree);
	  
	  //close the file
	  file.close();
	  
	  //boost::property_tree::ptree password_node;
	  //password_node = doc_tree.get_child("password");
	  
	  //pull the value of the password
	  std::string password(doc_tree.get<std::string>("password"));
	  
	  //return if the password is right. 
	  print_debug_message("Does (doc)" + password + "== (passed)" + doc_password + "?" 
						  + (password == doc_password? "Yes" : "No"));
						  
	  return (password == doc_password);	 
  }
  
  //fucntion to see if there is a session already running for
  //specific spreadsheet
  bool session::ss_session_in_progress(std::string spreadsheet_name)const
  {
	  print_debug_message("Check if " + spreadsheet_name + " exists.");
	  print_debug_message("Does " + spreadsheet_name + "exist? " +
						  ((server_ptr_->ss_sessions_.find(spreadsheet_name)
						  != server_ptr_->ss_sessions_.end())?"Yes":"No"));
						  
	  //see if an ss_session already exists in the map
		return (server_ptr_->ss_sessions_.find(spreadsheet_name) != 
				server_ptr_->ss_sessions_.end());
  }
  //funciton will handle create requests
  void session::create_requested(std::vector<std::string> parsed_result)
  {
	print_debug_message("Create has been requested.");
	print_debug_message(buffer_);
	
	//set up appropriate values
	std::string doc_name = parsed_result[2];
	std::string doc_password = parsed_result[4];
	
	//if the document doesn't already exist, create it. 
	if(!spreadsheet_exists(doc_name))
	{
		print_debug_message("Creating XML doc " + doc_name);
		
		std::string output_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
		output_xml += "<spreadsheet>";
		output_xml += "<name>\"" + doc_name + "\"</name>";
		output_xml += "<password>\"" + doc_password + "\"</password>";
		output_xml += "</spreadsheet>";
		
		print_debug_message(output_xml);
	
		std::ofstream fout;
		fout.open(doc_name.c_str());
		fout << output_xml;
		fout.close();
		
		print_debug_message("Sending create OK message");
		
		//Document already exists, let the client know and close socket
		std::string create_ok = "CREATE OK\nNAME:" + doc_name + "\n";
		create_ok += "Password:" + doc_password + "\n";
		
		print_debug_message("Reading socket");
		
		boost::asio::async_write(socket_,
		 boost::asio::buffer(create_ok.c_str(), create_ok.length()),
          boost::bind(&session::begin_read, this,
            boost::asio::placeholders::error));
		
	}
	else
	{
		print_debug_message("Document " + doc_name + " already exists.");
		
		//Document already exists, let the client know and close socket
		std::string create_error = "CREATE FAIL\nNAME:" + doc_name + "\n";
		create_error += doc_name + " already exists.\n";
		boost::asio::async_write(socket_,
		 boost::asio::buffer(create_error.c_str(), create_error.length()),
          boost::bind(&session::close_socket, this,
            boost::asio::placeholders::error));
	}
  }
  //function will handle join requests
  void session::join_requested(std::vector<std::string> parsed_result)
  {	  	  
	//set up appropriate values
	std::string doc_name = parsed_result[2];
	std::string doc_password = parsed_result[4];
	
	print_debug_message("Join has been requested for " + doc_name); 
	
	//check if the document exists
	if(!spreadsheet_exists(doc_name))
	{
		print_debug_message(doc_name + " does not exist. Send Join Fail."); 
		
		//if it doesn't, send join fail and close socket
		std::string join_error = "JOIN FAIL\nNAME:" + doc_name + "\n";
		join_error += "Spreadsheet doesn't exist.\n";
		boost::asio::async_write(socket_,
		 boost::asio::buffer(join_error.c_str(), join_error.length()),
          boost::bind(&session::close_socket, this,
            boost::asio::placeholders::error));		
		return;
	}
	
	//verify password
	if(!password_is_valid(doc_name, doc_password))
	{
		print_debug_message("Invalid Password. Send Join Fail"); 
		
		//if bad password, send join fail and close socket
		std::string join_error = "JOIN FAIL\nNAME:" + doc_name + "\n";
		join_error += "In correct password.\n";
		boost::asio::async_write(socket_,
		 boost::asio::buffer(join_error.c_str(), join_error.length()),
          boost::bind(&session::close_socket, this,
            boost::asio::placeholders::error));		
		return;
	}
		
	//check if sessions in progress
	if(ss_session_in_progress(doc_name))
	{
		print_debug_message("Session for " + doc_name + " is in progress. "
							"Add socket to ss_session."); 
							
		//if it is in progress, add socket to ss_session
		ss_session *temp = server_ptr_->ss_sessions_[doc_name];
		temp->addSocket(this->socket_);
		
	}
	else
	{
		print_debug_message("No session in progress for " + doc_name); 
		print_debug_message("Create new session."); 
		//else create new ss_session
		//ss_session(boost::asio::ip::tcp::socket &socket, std:: string name, server *server_ptr);
		ss_session temp_session = new ss_session(this->socket_, doc_name, this);
		
		//add session to dictionary
		this->sever_ptr_->ss_session_[doc_name] = temp_session;
		
		print_debug_message("Added ss_session to dictionary."); 
		
		//add ss_session and session to dictionary
		server_ptr_->sessions_[doc_name] = this;
		
		print_debug_message("Added session to dictionary"); 
	}	
  }
  //function willl handle all non valid requests from socket
  void session::other_requested()
  {
	if(DEBUG)
	{
		std::string str = buffer_; 
		print_debug_message(str + " is invalid. Closing session."); 
	}
	this->close_socket();
  }
  //function close the socket and delete this object. can be called whenever
  void session::close_socket()
  {
	this->socket_.close();
	delete this;
	print_debug_message("Session closed/deleted."); 
  }
  //funciton to close the socket. Allows for it to be called as a callback 
  //from asynchronous operation.
  void session::close_socket(const boost::system::error_code& error)
  {
	close_socket();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //------------------------Server Functions-----------------------------------//
  /////////////////////////////////////////////////////////////////////////////// 
  //constructor
  //have to initialize all of the boost variables inline
  server::server(int port_num, boost::asio::io_service& io_service): ss_sessions_(), sessions_(),
  acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_num))
  {
	print_debug_message("New server created on port " + port_num); 
    port_number_ = port_num;
        
    //start accepting sockets
     my_session = new session(io_service, this);

    print_debug_message("Accepting sockets");
    acceptor_.async_accept(my_session->socket(), boost::bind(&server::accept_handler, this, my_session, boost::asio::placeholders::error));
  }

  void server::accept_handler(session* my_session,const boost::system::error_code& error)
  {
	print_debug_message("Acceped socket"); 
    //start session listening on that socket
    //set up new session and start accepting sockets agian. 
    my_session->start_reading();
    my_session = new session(io_service_, this);
    //acceptor_.async_accept(my_session->socket_, accept_socket); 
    print_debug_message("Accepting sockets");
    acceptor_.async_accept(my_session->socket(), boost::bind(&server::accept_handler, this, my_session, boost::asio::placeholders::error));
  }
  
   //function that will allow a session to kill itself
  void server::kill_session(std::string session_name)
  {
	print_debug_message("Kill session requested for " + session_name); 
	 
	sessions_.erase(session_name);
	ss_sessions_.erase(session_name);
	
	print_debug_message("Session for " + session_name + " has been killed.");
  }
}
