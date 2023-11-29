#include <windows.h>
#include <odbcinst.h>
#include <sql.h>
#include <sqlext.h>
#include <ctype.h>
#include <string>
#include <iostream>
using namespace std;
SQLWCHAR connectionStringDSN[256] = L"DSN=autoserviceConn;UID=ru; PWD=ru;";

SQLWCHAR connectionStringDriver[256] = L"DRIVER={ODBC Driver 17 for SQL Server};SERVER={localhost\\SQLExpress};DATABASE={autoservice};UID=ru;PWD=ru;";




void errorMessage(const char* s)
{
	cout << "*** Error *** " << s << endl;
}
void Message(const char* s)
{
	cout << s << endl;
}

void DoInsert(SQLWCHAR connectionString[256])
{
	HENV hEnv;
	HDBC hDbc;
	HSTMT hStmt = SQL_NULL_HSTMT;
	RETCODE rc;
	SQLSMALLINT cbOutConStr = 0;
	SQLWCHAR outConnectionString[256];

	SQLAllocEnv(&hEnv);
	SQLAllocConnect(hEnv, &hDbc);

	rc = SQLDriverConnect(hDbc, NULL,
		connectionString, SQL_NTS,
		outConnectionString,
		sizeof(outConnectionString),
		&cbOutConStr, SQL_DRIVER_COMPLETE);
	//SQL_DRIVER_NOPROMPT SQL_DRIVER_COMPLETE
	if (!(rc == SQL_SUCCESS) && !(rc ==
		SQL_SUCCESS_WITH_INFO))
	{
		cout << rc << endl;
		errorMessage("Error SQLDriverConnect");
		return;
	}

	rc = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

	SQLWCHAR QUERYWITHDATA[256] = L"insert into autoservice.dbo.services (name, city, address, rating, is_main, date_created) values ('test name','test city','test address',4.45,0,GETDATE())";
	SQLWCHAR QUERY[256] = L"insert into autoservice.dbo.services (name, city, address, rating, is_main, date_created) values (?,?,?,?,?,GETDATE())";
	rc = SQLPrepare(hStmt, QUERY, SQL_NTS);

	string name_val = "", city_val = "", address_val = "";
	double rating_val = 0.0; bool is_main_val = false;

	cin.get();
	cout << "name:" << endl;
	getline(cin, name_val);
	cout << "city:" << endl;
	getline(cin, city_val);
	cout << "address:" << endl;
	getline(cin, address_val);
	cout << "rating:" << endl;
	cin >> rating_val;
	cout << "is main (0 or 1):" << endl;
	cin >> is_main_val;

	SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, 50, 0, (SQLPOINTER)name_val.c_str(), name_val.length(), NULL);
	SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, 50, 0, (SQLPOINTER)city_val.c_str(), city_val.length(), NULL);
	SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, 100, 0, (SQLPOINTER)address_val.c_str(), address_val.length(), NULL);
	SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &rating_val, 0, NULL);
	
	SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_BIT, SQL_BIT, 1, 0, &is_main_val, 0, NULL);

	rc = SQLExecute(hStmt);

	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
	{
		Message("execute successfully");
	}
	else
	{
		errorMessage("error of execute SQL");
	}

	SQLDisconnect(hDbc);
	SQLFreeConnect(hDbc);
	SQLFreeEnv(hEnv);
}

void DoSelect(SQLWCHAR connectionString[256])
{
	HENV hEnv;
	HDBC hDbc;
	HSTMT hStmt = SQL_NULL_HSTMT;
	RETCODE rc;
	SQLSMALLINT cbOutConStr = 0;

	SQLWCHAR outConnectionString[256];
	SQLAllocEnv(&hEnv);
	SQLAllocConnect(hEnv, &hDbc);
	rc = SQLDriverConnect(hDbc, NULL,
		connectionString, SQL_NTS,
		outConnectionString,
		sizeof(outConnectionString),
		&cbOutConStr, SQL_DRIVER_COMPLETE);
	//SQL_DRIVER_NOPROMPT SQL_DRIVER_COMPLETE
	if (!(rc == SQL_SUCCESS) && !(rc ==
		SQL_SUCCESS_WITH_INFO))
	{
		cout << rc << endl;
		errorMessage("Error SQLDriverConnect");
		return;
	}
	SQLHSTMT hstmt;
	rc = SQLAllocStmt(hDbc, &hstmt);
	SQLWCHAR command[256] = L"SELECT id, name, rating, date_created FROM services";
	rc = SQLExecDirect(hstmt, command, SQL_NTS);
	SQLINTEGER ServiceId;
	SQLCHAR ServiceName[50];
	SQLFLOAT ServiceRating;
	TIMESTAMP_STRUCT ServiceDateCreated;

	SQLLEN CbSerId, CbSerName, CbSerRating, CbSerDateCreated;
	
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		/* Bind columns 1, 2 */
		SQLBindCol(hstmt, 1, SQL_C_ULONG, &ServiceId, 0, &CbSerId);
		SQLBindCol(hstmt, 2, SQL_C_CHAR, ServiceName, 50, &CbSerName);
		SQLBindCol(hstmt, 3, SQL_C_DOUBLE, &ServiceRating, 6, &CbSerRating);
		SQLBindCol(hstmt, 4, SQL_C_TIMESTAMP,&ServiceDateCreated ,6, &CbSerDateCreated);
		
		/* Fetch and print each row of data. On */
		/* an error, display a message and exit. */
		while (TRUE) {
			rc = SQLFetch(hstmt);
			if (rc == SQL_ERROR)
			{
				errorMessage("Error in the fetch");
			}
			if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
			{
				cout << "row: id " << ServiceId << ", name:  " << ServiceName << ", rating " << ServiceRating << ", created date " << ServiceDateCreated.year << "/" << ServiceDateCreated.month << "/" << ServiceDateCreated.day << ", " << ServiceDateCreated.hour << ":" << ServiceDateCreated.minute << std::endl;

			}
			else {
				break;
			}
		}
		// SQLFreeStmt(hstmt, SQL_DROP);
		SQLDisconnect(hDbc);
		SQLFreeConnect(hDbc);
		SQLFreeEnv(hEnv);
	}
	else
		errorMessage("Exec SQL error");
}
void ListDSN()
{
	const short SQL_MAX_DSN_LENGTH_ =
		SQL_MAX_DSN_LENGTH;
	SQLWCHAR szDSN[SQL_MAX_DSN_LENGTH + 1];
	SQLWCHAR szDescription[256];
	short wDSNLen;
	SQLSMALLINT wDesLen;
	int retCode;
	SQLHENV hEnv = NULL;
	string DSNName;
	string resultString;
	string Descr;
	SQLAllocEnv(&hEnv);
	retCode = SQLDataSources(hEnv,
		SQL_FETCH_FIRST, szDSN, SQL_MAX_DSN_LENGTH_ + 1,
		&wDSNLen, szDescription, 256, &wDesLen);
	while (retCode == SQL_SUCCESS || retCode ==	SQL_SUCCESS_WITH_INFO)
	{
		DSNName = (string)((char*)szDSN);
		Descr = (string)((char*)szDescription);
		resultString += DSNName;
		resultString += "\n";
		retCode = SQLDataSources(hEnv,
			SQL_FETCH_NEXT, szDSN, SQL_MAX_DSN_LENGTH_ + 1,
			&wDSNLen, szDescription, 256,
			&wDesLen);
	}
	SQLFreeEnv(hEnv);
	Message(resultString.c_str());
}
void ShowMenu()
{
	int choice = 0;
	bool done = false;
	do
	{
		cout << "\n*** MENU ***" << endl;
		cout << "0 - output" << endl;
		cout << "1 - list all DNS" << endl;
		cout << "2 - select (using DNS)" << endl;
		cout << "3 - select (using driver directly)" << endl;
		cout << "4 - insert record (using driver directly)" << endl;
		cout << "Choice : ";
		cin >> choice;
		switch (choice)
		{
		
			case 1: ListDSN(); break;
			case 2: DoSelect(connectionStringDSN);
				break;
			case 3: DoSelect(connectionStringDriver);
				break;
			case 4: DoInsert(connectionStringDriver);
				break;
			default: done = true; break;
		}
	} while (!done);
}

int main(int count, char* cparams[])
{
	ShowMenu();
	return 0;
}