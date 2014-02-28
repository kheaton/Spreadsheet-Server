#include "ss_session.h"
#include <iostream>

using namespace std;
using boost::asio::ip::tcp;
using boost::property_tree::ptree;
using boost::property_tree::write_xml;
using boost::property_tree::xml_writer_settings;

namespace ss_server
{

  
  ss_xml::ss_xml(std::string name)
  {
    ptree tree;
  }

  void ss_xml::xmlCreate(string name, string password)
  {
    tree.add("spreadsheet", "");
    tree.add("spreadsheet.name", name);
    tree.add("spreadsheet.password", password);
    ptree& cel = tree.add("spreadsheet.cell", "");
    cel.add("content", "");

    xmlWrite(name);  
  }

  void ss_xml::xmlUpdate(boost::unordered_map<string, string> cellAndContent, string name)
  {
    // iterator to go over the map of cells
    boost::unordered_map<std::string, std::string>::const_iterator i;

    // add each cells name and content to the xml
    for(i = cellAndContent.begin(); i != cellAndContent.end(); ++i)
      {
		ptree& cell = tree.add("spreadsheet.cell", "");
		cell.add("name", i->first);
		cell.add("content", i->second);
      }

    xmlWrite(name);
  }

  void ss_xml::xmlWrite(string name)
  {
    // I write the xml and put it on the server
    write_xml(name, tree, std::locale(), xml_writer_settings<char>(' ', 4));
  }
  
  
  sub_session::sub_session(boost::asio::ip::tcp::socket &socket, 
							std::string name, ss_session *session_ptr, unsigned int session_num)
							:socket_(socket)
							{}
	void sub_session::update_ses(std::string cell, std::string contents)
	{}
  
  
  /////////////////////////////////////////////////////////////////////
  //                        Session Functions                        //
  /////////////////////////////////////////////////////////////////////
  ss_session::ss_session(boost::asio::ip::tcp::socket &socket, 
			std::string doc_name, server *server_ptr):xmlobj(doc_name), 
			soclist(), thestack(), cellAndContent()
  {
	server_session = server_ptr;
    version = 0;
    docname = doc_name;
    std::list<sub_session>::iterator it = soclist.begin();
    session_number = 0;
    //int xmllength;
    
    this->addSocket(socket);
  }
  
  //public function to have a sub_session leave 
  void ss_session::leave_session(unsigned int session_num)
  {	  
	  if(soclist.size() <= 1)
	  {
		 server_session->kill_session(docname); 
	  }
	  else
	  {
		//loop through each spot in the list
		for(std::list<sub_session>::iterator iterator = soclist.begin(), end = soclist.end();
			(iterator != end); iterator++)
		{
			////When we find the right one, erase
			if(session_num == iterator->ses_num)
			{
				soclist.erase(iterator);
				break;
			}
		}
	  }
  }
  void ss_session::addSocket(boost::asio::ip::tcp::socket& socket)
  {
	//create new sub_session object, push it onto the sub_session list
	sub_session temp(socket, this->docname, this, session_number++);
	soclist.push_front(temp);
  }
  bool ss_session::save()
  {
	  //Update the XML object
	  xmlobj.xmlUpdate(cellAndContent, docname);
	  
	  return true;
  }
  void ss_session::update(std::string cell, std::string content)
  {

	  //Loop over each subsession and call the update function
	  for(std::list<sub_session>::iterator iterator = soclist.begin(), end = soclist.end();
			iterator != end; iterator++)
		{
				//send sub_session an update
				iterator->update_ses(cell, content);
		}
  }
  //Function to increase the inst and string version
  void ss_session::increase_version()
  {
	  stringstream temp;
	  temp << ++version;
	  version_str = temp.str();
  }
}






