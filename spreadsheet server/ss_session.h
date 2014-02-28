#ifndef _SS_SESSION_H_
#define _SS_SESSION_H_ 

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <string>
#include <sstream>
#include <list>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <stack>
#include <boost/algorithm/string.hpp>
#include <vector>

namespace ss_server
{
  class server
  {
  public:
    void kill_session(std::string docname);
  };

  class ss_session;
  class sub_sesssion;

  class ss_xml
  {
  public:
    ss_xml(std::string name);
    void xmlCreate(std::string name, std::string password);
    void xmlUpdate(boost::unordered_map<std::string, std::string> cellAndContent, std::string name);
    void xmlWrite(std::string name);

  private:
    boost::property_tree::ptree tree;
  }; 

  class sub_session
  {
  public:
    sub_session(boost::asio::ip::tcp::socket &socket, std::string name, 
				ss_session *session_ptr, unsigned int session_num);
    boost::asio::ip::tcp::socket& socket();
    void start();
    void handle_read(const boost::system::error_code& error,
		     size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);

  private:
    ss_session* ss_sess;
    boost::asio::ip::tcp::socket &socket_;
    enum { max_length = 1024 };
    char data_[max_length]; 
    std::string docname;
    int xmllength;
    unsigned int ses_num;
    
    friend class ss_session;

    std::string int_to_string(int i);
    std::string xmlfile();
    void change(std::vector<std::string> message);
    void undo(std::vector<std::string> message);
    void leave(boost::asio::ip::tcp::socket &socket);
    void update_ses(std::string cell, std::string contents);
    
    
  };

  class ss_session
  {
  public:
    ss_session(boost::asio::ip::tcp::socket &socket, std:: string name, server *server_ptr);
    void leave_session(unsigned int session_num);

  private:
    server* server_session; 
    int version;
    unsigned int session_number;
    std::string version_str;
    std::list<sub_session> soclist; 
    std::list<sub_session>::iterator it;
    std::stack<std::pair<std::string, std::string> > thestack;
    std::string docname;
    int xmllength;
    boost::unordered_map<std::string, std::string> cellAndContent;
    ss_xml xmlobj; 
    
    friend class sub_session;

    void addSocket(boost::asio::ip::tcp::socket& socket);
    bool save();
    void update(std::string cell, std::string content);
    void increase_version();
  }; 
}

#endif 



