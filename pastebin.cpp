#include <httpserver.hpp>
#include <iostream>
#include <sstream>
#include "srchilite/sourcehighlight.h"
#include "srchilite/langmap.h"

#include "pastebin.hpp"
#include "storage.hpp"

#include "config.h"

using namespace std;
using namespace httpserver;

//#define DEBUG

#define USAGE	"PUT: curl http://server:8080/foo.c --data-binary @foo.c\n" \
		"PUT with rudimentary auth: curl http://server:8080/foo.c --data-binary @foo.c --user britneyspears\n" \
		"GET: curl http://server:8080/foo.c\n" \
		"GET without highlighting: curl -http://server:8080/foo.c -A \"\"\n"
void
pastebin_resource::set_storage(storage *st)
{
	this->st = st;
}

void
pastebin_resource::highlight(string *filename, string *useragent, string *src, string *hl_src, bool dohighlight)
{
	istringstream input;
	ostringstream output;
	string highlight = "none";

	if (useragent->compare(0, 4, "curl") == 0) {
		highlight = "esc.outlang";
		dohighlight = false;
	} else if (useragent->compare(0, 7, "Mozilla") == 0)
		highlight = "html.outlang";

	if (highlight != "none") {
		srchilite::SourceHighlight sourceHighlight(highlight);
		srchilite::LangMap langMap(DATADIR, "lang.map");

		sourceHighlight.setDataDir(DATADIR);
		if (dohighlight)
			sourceHighlight.setGenerateLineNumbers(true);

		string inputLang = langMap.getMappedFileNameFromFileName(*filename);
#ifdef DEBUG
		cout << "inputLang: " << inputLang << endl;
#endif
		if (inputLang == "")
			inputLang = "nohilite.lang";

		input.str(*src);
		sourceHighlight.highlight(input, output, inputLang);

		*hl_src = output.str();
		if (highlight == "html.outlang") {
			hl_src->insert(0, "<html><body>");
			hl_src->append("</body></html>");
		}
	} else {
		*hl_src = *src;
	}
}

void
pastebin_resource::remove(string &key, string &usr, string &pwd, http_response** res)
{
	int ret;

	ret = st->remove(key, usr, pwd);

	if (ret == 0) {
		*res = new http_string_response("removed", 200);
	} else {
		*res = new http_string_response("not authorized", 200);
	}
}

void
pastebin_resource::get(string &key, string &usr, string &pwd, string &useragent, http_response** res, bool dohighlight)
{
	string *src;

	if (key == "") {
		*res = new http_string_response(USAGE, 200);
		return;
	}

	src = st->load(key);
	if (src != NULL) {
		string hl_src;

		this->highlight(&key, &useragent, &(*src), &hl_src, dohighlight);
		*res = new http_string_response(hl_src, 200);
		if (useragent.compare(0, 7, "Mozilla") == 0) {
			(**res).set_content_type("text/html");
		}
		delete src;
	} else
		*res = new http_string_response("not found :(", 200);
}

void
pastebin_resource::post(string &key, string &usr, string &pwd, string &content, http_response** res)
{
	bool auto_pwd = false;
	string *src;

	if (key[key.length() - 4] != '.' &&
		key[key.length() - 3] != '.' &&
		key[key.length() - 2] != '.') {
		*res = new http_string_response("filename not like *.??? :(", 200);
		return;
	}

	if (usr == "" && pwd == "") {
		src = st->load(key);
		if (src == NULL) {
			usr = "Ralph Himberger";
			pwd = "correct horse battery staple";
			auto_pwd = true;
		} else {
			delete src;
		}
	}

	if (st->save(key, content, usr, pwd) == 0) {
#ifdef DEBUG
		cout << "saved" << endl;
#endif
		if (auto_pwd) {
			string resp = "Automagically chosen (usr, pwd): "
					"(" + usr + ", " + pwd + ")\n";
			*res = new http_string_response(resp, 200);
		} else {
			*res = new http_string_response("saved", 200);
		}
	} else {
#ifdef DEBUG
		cout << "auth problem" << endl;
#endif
		*res = new http_string_response("not authorized", 200);
	}
}

void
pastebin_resource::render(const http_request& req, http_response** res)
{
	string key = &(req.get_path()[1]);
	string content = req.get_content();
	string method = req.get_method();
	string useragent = req.get_header("User-Agent");
	string usr = req.get_user();
	string pwd = req.get_pass();
	string ln = req.get_arg("ln");
	*res = NULL;

#ifdef DEBUG
	cout << "path: " << req.get_path() << endl;
	cout << "key: " << key << endl;
	cout << "method: " << method << endl;
	cout << "useragent: " << useragent << endl;
	cout << "ln: " << ln << endl;
	cout << "content: " << endl << content << endl;
#endif

	if (method == "GET") {
		get(key, usr, pwd, useragent, res, ln == "1");
	} else if (method == "POST") {
		post(key, usr, pwd, content, res);
	} else if (method == "DELETE") {
		remove(key, usr, pwd, res);
	} else {
		*res = new http_string_response("not found :(", 200);
		return;
	}
}

int
main()
{
	storage st;
	webserver ws = create_webserver(PORT).max_threads(5);

	pastebin_resource wr;
	wr.set_storage(&st);
	ws.register_resource("/*", &wr, true);

	ws.start(true);
	return 0;
}
