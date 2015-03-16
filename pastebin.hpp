#ifndef __PASTEBIN_HPP
#define __PASTEBIN_HPP
#include "storage.hpp"

using namespace httpserver;

class pastebin_resource : public http_resource<pastebin_resource> {
        public:
                void render(const http_request& req, http_response** res);
		void set_storage(storage *st);
	private:
		void highlight(string *filename, string *useragent, string *src, string *hl_src, bool dohighlight = true);
		void get(string &key, string &usr, string &pwd, string &useragent, http_response** res, bool dohighlight = true);
		void remove(string &key, string &usr, string &pwd, http_response** res);
		void post(string &key, string &ysr, string &pwd, string &content, http_response** res);
		storage *st;
};


#endif
