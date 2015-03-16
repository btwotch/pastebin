#include <iostream>
#include <cstdlib>
#include <sqlite3.h>
#include "storage.hpp"

using namespace std;

#define CREATE_TABLE_PASTIES "CREATE TABLE IF NOT EXISTS pasties (key TEXT PRIMARY KEY, source TEXT);"
#define CREATE_TABLE_OWNER "CREATE TABLE IF NOT EXISTS owner (owner TEXT, ownee REFERENCES pasties(key) PRIMARY KEY);"
#define INSERT_PASTIES "INSERT OR REPLACE INTO pasties (key, source) VALUES (?, ?);"
#define SELECT_PASTIES "SELECT source FROM pasties WHERE key = ?;"
#define DELETE_PASTIES "DELETE FROM pasties WHERE key = ?;"
#define INSERT_OWNER "INSERT OR REPLACE INTO owner (owner, ownee) VALUES (?, ?);"
#define SELECT_OWNER "SELECT owner FROM owner WHERE ownee = ?;"
#define DELETE_OWNER "DELETE FROM owner WHERE ownee = ?;"

storage::storage()
{
	char *errmsg = NULL;

	sqlite3_open("pastebin.db", &db);

	if (sqlite3_exec(db, CREATE_TABLE_PASTIES, NULL, 0, &errmsg) != SQLITE_OK) {
		cerr << "could not create table (" << CREATE_TABLE_PASTIES << "): " << errmsg << endl;
		exit(-1);
	}

	if (sqlite3_exec(db, CREATE_TABLE_OWNER, NULL, 0, &errmsg) != SQLITE_OK) {
		cerr << "could not create table (" << CREATE_TABLE_OWNER << "): " << errmsg << endl;
		exit(-1);
	}

	pthread_mutex_init(&monitor, NULL);
}

storage::~storage()
{
	sqlite3_close(db);
	pthread_mutex_destroy(&monitor);
}

void storage::lock()
{
	pthread_mutex_lock(&monitor);
}

void storage::unlock()
{
	pthread_mutex_unlock(&monitor);
}

/* monitor must already be locked */
bool
storage::is_auth(string &key, string &usr, string &pwd)
{
	int ret;
	string up;
	string db_up;
	sqlite3_stmt *stmt;

	up = usr + pwd;

	sqlite3_prepare_v2(db, SELECT_OWNER, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW) {
		db_up = (char*) sqlite3_column_text(stmt, 0);
	
		if (db_up != up) {
			sqlite3_finalize(stmt);
			return false;
		}
	} else if (ret != SQLITE_DONE) {
		cerr << "SELECT_OWNER: " << sqlite3_errmsg(db) << endl;
	}

	sqlite3_finalize(stmt);

	return true;
}

int
storage::remove(string &key, string &usr, string &pwd)
{
	int ret;

	lock();
	sqlite3_stmt *stmt;

	if (!is_auth(key, usr, pwd)) {
		unlock();
		return -1;
	}

	sqlite3_prepare_v2(db, DELETE_PASTIES, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
	ret = sqlite3_step(stmt);
	if (ret != SQLITE_ROW && ret != SQLITE_DONE) {
		cerr << "DELETE_PASTIES: " << sqlite3_errmsg(db) << endl;
	}
	sqlite3_finalize(stmt);

	sqlite3_prepare_v2(db, DELETE_OWNER, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	unlock();

	return 0;
}

int
storage::save(string &key, string &s, string &usr, string &pwd)
{
	string up;
	string db_up;
	sqlite3_stmt *stmt;

	lock();
	if (!is_auth(key, usr, pwd)) {
		unlock();
		return -1;
	}

	up = usr + pwd;

	sqlite3_prepare_v2(db, INSERT_OWNER, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, up.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_STATIC);
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		cerr << "INSERT_OWNER: " << sqlite3_errmsg(db) << endl;
	}
	sqlite3_finalize(stmt);

	sqlite3_prepare_v2(db, INSERT_PASTIES, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, s.c_str(), -1, SQLITE_STATIC);
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		cerr << "INSERT_PASTIES: " << sqlite3_errmsg(db) << endl;
	}
	sqlite3_finalize(stmt);
	unlock();

	return 0;
}

string*
storage::load(string &key)
{
	string *ret = NULL;
	sqlite3_stmt *stmt;

	lock();
	sqlite3_prepare_v2(db, SELECT_PASTIES, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		ret = new string((char*)sqlite3_column_text(stmt, 0));
	}
	
	sqlite3_finalize(stmt);

	unlock();
	return ret;
}
