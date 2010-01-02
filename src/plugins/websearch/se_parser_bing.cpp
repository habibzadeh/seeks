/**
 * The Seeks proxy and plugin framework are part of the SEEKS project.
 * Copyright (C) 2009 Emmanuel Benazera, juban@free.fr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 **/
 
#include "se_parser_bing.h"
#include "miscutil.h"

#include <strings.h>
#include <iostream>

using sp::miscutil;

namespace seeks_plugins
{
   std::string se_parser_bing::_sr_string_en = "ALL RESULTS";
   std::string se_parser_bing::_bing_stupid[2] =
     { "Document title", "Titre du document / Document title" };
   
   se_parser_bing::se_parser_bing()
     :se_parser(),_h1_flag(false),_h1_sr_flag(false),_results_flag(false),_h3_flag(false),
      _link_flag(false),_p_flag(false),_cite_flag(false),_cached_flag(false)
       {
       }
   
   se_parser_bing::~se_parser_bing()
     {
     }
   
   void se_parser_bing::start_element(parser_context *pc,
				      const xmlChar *name,
				      const xmlChar **attributes)
     {
	const char *tag = (const char*)name;
	
	if (strcasecmp(tag,"h1") == 0)
	  {
	     _h1_flag = true;
	  }
	else if (_h1_sr_flag && strcasecmp(tag,"div") == 0)
	  {
	     const char *a_class = se_parser::get_attribute((const char**)attributes,"class");
	     if (a_class && strcasecmp(a_class,"sb_tlst") == 0)
	       {
		  // create new snippet.
		  search_snippet *sp = new search_snippet(_count+1);
		  _count++;
		  sp->_engine |= std::bitset<NSEs>(SE_BING);
		  pc->_snippets->push_back(sp);
		  pc->_current_snippet = sp;
		  
		  _cached_flag = false; // in case previous snippet did not close the cached flag.

		  _results_flag = true;
	       }
	  }
	else if (_h1_sr_flag && strcasecmp(tag,"h3") == 0)
	  {
	     _h3_flag = true;
	  }
	else if (_h1_sr_flag && _h3_flag && strcasecmp(tag,"a") == 0)
	  {
	     _link_flag = true;
	     const char *a_link = se_parser::get_attribute((const char**)attributes,"href");
	     
	     if (a_link)
	       _link = std::string(a_link);
	  }
	else if (_h1_sr_flag && strcasecmp(tag,"p") == 0)
	  {
	     _p_flag = true;
	  }
	else if (_h1_sr_flag && strcasecmp(tag,"cite") == 0)
	  {
	     _cite_flag = true;
	  }
	else if (_h1_sr_flag && _cached_flag && strcasecmp(tag,"a") == 0) // may not be very robust...
	  {
	     _cached_flag = false;
	     const char *a_link = se_parser::get_attribute((const char**)attributes,"href");
	     if (a_link)
	       pc->_current_snippet->_cached = std::string(a_link);
	     pc->_current_snippet->set_archive_link();
	  }
     }
   
   void se_parser_bing::characters(parser_context *pc,
				   const xmlChar *chars,
				   int length)
     {
	handle_characters(pc, chars, length);
     }
   
   void se_parser_bing::cdata(parser_context *pc,
			      const xmlChar *chars,
			      int length)
     {
	handle_characters(pc, chars, length);
     }
   
   void se_parser_bing::handle_characters(parser_context *pc,
					  const xmlChar *chars,
					  int length)
     {
	if (_h1_flag && !_h1_sr_flag)
	  {
	     std::string a_chars = std::string((char*)chars);
	     miscutil::replace_in_string(a_chars,"\n"," ");  // in case...
	     miscutil::replace_in_string(a_chars,"\r"," ");
	     
	     if (strcasecmp(a_chars.c_str(),_sr_string_en.c_str()) == 0)
	       _h1_sr_flag = true;
	  }
	/* else if (!_results_flag)
	  return; */
	else if (_p_flag)
	  {
	     std::string a_chars = std::string((char*)chars);
	     miscutil::replace_in_string(a_chars,"\n"," ");
	     miscutil::replace_in_string(a_chars,"\r"," ");
	     miscutil::replace_in_string(a_chars,"-"," ");
	     miscutil::replace_in_string(a_chars,se_parser_bing::_bing_stupid[1],"");
	     miscutil::replace_in_string(a_chars,se_parser_bing::_bing_stupid[0],"");
	     _summary += a_chars;
	  }
	else if (_cite_flag)
	  {
	     std::string a_chars = std::string((char*)chars);
	     miscutil::replace_in_string(a_chars,"\n"," ");
	     miscutil::replace_in_string(a_chars,"\r"," ");
	     //miscutil::replace_in_string(a_chars,"-"," ");
	     _cite += a_chars;
	  }
	else if (_h3_flag)
	  {
	     std::string a_chars = std::string((char*)chars);
	     miscutil::replace_in_string(a_chars,"\n"," ");
	     miscutil::replace_in_string(a_chars,"\r"," ");
	     _h3 += a_chars;
	  }
	
	
     }

   void se_parser_bing::end_element(parser_context *pc,
				    const xmlChar *name)
     {
	const char *tag = (const char*) name;
	
	if (!_results_flag)
	  return;
	
	if (_h1_sr_flag && _h3_flag && strcasecmp(tag,"a") == 0)
	  {
	     _link_flag = false;
	     pc->_current_snippet->set_url(_link);
	     _link = "";
	  }
	else if (_p_flag && strcasecmp(tag,"p") == 0)
	  {
	     _p_flag = false;
	     pc->_current_snippet->set_summary(_summary);
	     _summary = "";
	  }
	else if (_cite_flag && strcasecmp(tag,"cite") == 0)
	  {
	     _cite_flag = false;
	     pc->_current_snippet->_cite = _cite;
	     _cite = "";
	     _cached_flag = true; // getting ready for the Cached link, if any.
	  }
	else if (_h3_flag && strcasecmp(tag,"h3") == 0)
	  {
	     _h3_flag = false;
	     pc->_current_snippet->_title = _h3;
	     _h3 = "";
	  }
	
     }
   
   
} /* end of namespace. */
