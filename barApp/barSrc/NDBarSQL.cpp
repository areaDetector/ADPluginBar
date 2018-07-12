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
 * @return: con -> connection to MySQL server
 *
*/
sql::Connection* NDBarSQL::connect_to_sql(){
	sql::Driver *driver;
	sql::Connection * con;
	driver = get_driver_instance();
	con = driver->connect(currentConnection->server, currentConnection->username, currentConnection->password);
	return con;
}

/*
 * Function that creates the inintal barcode table in the database
 *
 * @return: void
*/
void NDBarSQL::init_sample_db(){
	sql::PreparedStatement* pstatement;
	sql::Statement* statement;
	statement = currentConnection->con->createStatement();
	statement->execute("USE "+currentConnection->dbName);
	delete statement;
	pstatement = currentConnection->con->prepareStatement("CREATE TABLE IF NOT EXISTS ?(BarcodeMessage VARCHAR(50), BarcodeType VARCHAR(20), Timestamp DATE DEFAULT NULL, Description VARCHAR(200) DEFAULT NULL, PRIMARY KEY(BarcodeMessage))");
	pstatement->setString(1, currentConnection->tableName);
	pstatement->executeUpdate();
	delete pstatement;
}

/*
 * Function that adds new barcode to sample table
 *
 * @params: BarcodeMessage -> message read from barcode
 * @params: BarcodeType -> type of barcode read
 * @return: void;
*/
void NDBarSQL::add_to_table(string BarcodeMessage, string BarcodeType){
	sql::Statement* statement;
	statement = currentConnection->con->createStatement();
	statement->execute("USE "+currentConnection->dbName);
	delete statement;
	sql::PreparedStatement* pstatement;
	pstatement = currentConnection->con->prepareStatement("INSERT INTO ? (BarcodeMessage, BarcodeType, Timestamp) VALUES (?, ?, NOW())");
	pstatement->setString(1, currentConnection->tableName);
	pstatement->setString(2, BarcodeMessage);
	pstatement->setString(3, BarcodeType);
	pstatement->executeUpdate();
	delete pstatement;
}

/*
 * Function that disconnects form database
 *
 * @return: void
 *
*/
void NDBarSQL::disconnect_from_sql(){
 	delete currentConnection->con;
}


/*
 * Constructor for the NDBarSQL class. It initializes the connection, and creates a sample Db
 * with the specified parameters if so desired.
 *
 * @params: dbName -> name of the database to connect to. Must be created in current version
 * @params: tableName -> name of table in which barcode information will be stored
 * @params: server -> server where database will be stored
 * @params: username -> username to connect to database
 * @params: password -> password to connect to the database
 *
*/
NDBarSQL::NDBarSQL(string dbName, string tableName, string server, string username, string password){

	//initialize the connection struct with the input values
	currentConnection = new BarSQLConnection;
	currentConnection->dbName = dbName;
	currentConnection->tableName = tableName;
	currentConnection->server = server;
	currentConnection->username = username;
	currentConnection->password = password;

	//connect to the database
	currentConnection->con  = connect_to_sql();
	//init table if necessary
	init_sample_db();
}

/*
 * Destructor for NDBarSQL class. First it closes the connection, then deallocates
 * memory used for previous variables
 *
*/
NDBarSQL::~NDBarSQL(){
	disconnect_from_sql();
	if(currentConnection) delete currentConnection;
}
