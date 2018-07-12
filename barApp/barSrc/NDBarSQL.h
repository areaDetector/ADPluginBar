/*
 * NDBarSQL.h
 *
 * Header file for SQL extention to EPICS Bar/QR Reader Plugin
 *
 * Author: Jakub Wlodek
 * Created: July 12, 2018
 */


#ifndef NDBarSQL_H
#define NDBarSQL_H


// standard includes
#include <stdio.h>
//#include <sstream>
//#include <iostream>
//#include <stdexcept>
#include <string.h>

// sql includes only include what is necessary here
#include "mysql_connection.h"
//#include <cppconn/driver.h>
//#include <cppconn/exception.h>
//#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

typedef struct{
	string dbName;
	string tableName;
	string server;
	string username;
	string password;
}BarSQLConnection;

//class that contains all NDBarSQL fucntions and variables
class NDBarSQL {

	public:
		//constructor definition
		NDBarSQL(string dbName, string tableName, string server, string username, string password);

	protected:
		//variables used by the SQL extention
		BarSQLConnection* currentConnection;

 	private:
		//functions used by the SQL extention
		//function that connects to the server
		sql::Connection* connect_to_sql();

		//function that initializes the sample table
		void init_sample_db(sql::Connection* con);

		//function that adds a barcode entry into the table
		void add_to_table(string BarcodeMessage, string BarcodeType, sql::Connection* con);

		//function that disconnects from the MySQL server
		void disconnect_from_sql(sql::Connection* con);
};

#endif
