/*
 * This file will contain an extention for the Area Detector Barcode Plugin
 * that allows for integration with MySQL databases. It will contain support
 * for automatically pushing detected barcodes and information to an SQL database.
 *
 * The MySQL C++ connector will be used to interact with the database
 *
 * Author: Jakub Wlodek
 * Created On: July 12, 2018
 *
*/

// standard includes
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <string.h>

// sql includes
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

// custom include
#include "NDBarSQL.h"


/*
 * Function used to connect with a MySQL database
 *
 * @params: server -> MySQL server to connect to i.e. 'localhost:3306'
 * @params: username -> username that will connect with mysql database
 * @params: password -> password for given username on given server
 * @return: con -> connection to MySQL server
 *
*/
sql::Connection* connect_to_sql(string server, string username, string password){
	sql::Driver *driver;
	sql::Connection * con;
	driver = get_driver_instance();
	con = driver->connect(server, username, password);
	return con;
}

/*
 * Function that creates the inintal barcode table in the database
 *
 * @params: dbName -> name of database to contain sample table
 * @params: tableName -> chosen name for sample table
 * @params: con -> connection to database
 * @return: void
*/
void init_sample_db(string dbName,string tableName, sql::Connection* con){
	sql::PreparedStatement* pstatement;
	sql::Statement* statement;
	statement = con->createStatement();
	statement->execute("USE "+dbName);
	delete statement;
	pstatement = con->prepareStatement("CREATE TABLE IF NOT EXISTS ?" + 
		"(BarcodeMessage VARCHAR(50), BarcodeType VARCHAR(20), Timestamp" +
	        " DATE DEFAULT NULL, Description VARCHAR(200) DEFAULT NULL, PRIMARY KEY(BarcodeMessage))");
	pstatement->setString(1, tableName);
	pstatement->executeUpdate();
	delete pStatement
}

/*
 * Function that adds new barcode to sample table
 *
 * @params: dbName -> name of database
 * @params: tableName -> name of sample table
 * @params: BarcodeMessage -> message read from barcode
 * @params: BarcodeType -> type of barcode read
 * @params: con -> connection to the database
 * @return: void;
*/
void add_to_table(string dbName, string tableName, string BarcodeMessage, string BarcodeType, sql::Connection* con){
	sql::Statement* statement;
	statement = con->createStatement();
	statement->execute("USE "+dbName);
	delete statement;
	sql::PreparedStatement* pstatement;
	pstatement = con->prepareStatement("INSERT INTO ? (BarcodeMessage, BarcodeType, Timestamp) VALUES (?, ?, NOW())");
	pstatement->setString(1, tableName);
	pstatement->setString(2, BarcodeMessage);
	pstatement->setString(3, BarcodeType);
	pstatement->executeUpdate();
	delete pstatement;
}

/*
 * Function that disconnects form database
 *
 * @params: con -> connection to database
 * @return: void
 *
*/
void disconnect(sql::Connection* con){
 	delete con;
}
  
  
