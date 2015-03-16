#ifndef __STORAGE_HPP
#define __STORAGE_HPP

#include <iostream>
#include <sqlite3.h>
#include <pthread.h>

using namespace std;

class storage {
	public:
		storage();
		~storage();
		int save(string &key, string &s, string &usr, string &pwd);
		int remove(string &key, string &usr, string &pwd);
		string* load(string &key);
	private:
		bool is_auth(string &key, string &usr, string &pwd);
		sqlite3* db;
		void lock();
		void unlock();
		pthread_mutex_t monitor;


};

#endif
