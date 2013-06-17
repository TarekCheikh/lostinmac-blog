/*
 * odbc-demo.c : ce programme demontre
 * un simple select dans une base utilisant UnixODBC
 */
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>

#define	BUFFER_LEN	1024
#define	OK		0
#define	KO		-1
#define	TIMEOUT		5	/* Timeout is set to 5 seconds */
#define	REC_NUMBER	1	/* Pour la fonction SQLGetDiagRec */
#define	SQLSTATE_LEN	7
#define	DB_NAME_LEN	128
#define	DB_VERSION_LEN	32
#define	STRING_REQUEST	"SELECT department_id, first_name, last_name, salary FROM HR.employees WHERE department_id is not null"

int odbc_connection(char *datasource, char *user, char *pwd)
{
	long status;
	SQLCHAR	 state[SQLSTATE_LEN];		/* Pour SQLSTATE code qui designe une erreur ou un warning */
	SQLINTEGER odbc_error;
	SQLSMALLINT error_text_len;
	SQLCHAR error_text[BUFFER_LEN];
	SQLCHAR db_name[DB_NAME_LEN];		/* Nom de la base de donnees */
	SQLCHAR db_version[DB_VERSION_LEN];	/* Version de la base de donnes */
	SQLHENV environment_handle;     	/* Pour mettre en place notre environnement */
	SQLHDBC connection_handle;      	/* Pour la connexion a la source de donnees */
	SQLHSTMT statment_handle;		/* Pour notre requete */
	int emp_department_id = 0;
	SQLCHAR	emp_first_name[21];
	SQLCHAR emp_last_name[26];
	float	emp_salary = 0.0;
	int number_fetched_lines = 0;

	/* Etape 1. allocation de la memoire pour mettre en place notre environnement ODBC */
	status = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &environment_handle);
	if ((status != SQL_SUCCESS) && (status != SQL_SUCCESS_WITH_INFO)) {
		fprintf(stderr, "SQLAllocHandle: SQL_HANDLE_ENV Failed\n");
		return KO;
	}
	/* Tous les recents driver ODBC supporte la version 3 d'ODBC */
	status = SQLSetEnvAttr(environment_handle, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, SQL_IS_INTEGER);
	if ((status != SQL_SUCCESS) && (status != SQL_SUCCESS_WITH_INFO)) {
		fprintf(stderr, "SQLSetEnvAttr Failed\n");
		SQLFreeHandle(SQL_HANDLE_ENV, environment_handle);
		return KO;
	}
	/* Etape 2. allocation de la memoire pour le handle de la connexion */
	status = SQLAllocHandle(SQL_HANDLE_DBC, environment_handle, &connection_handle);
	if ((status != SQL_SUCCESS) && (status != SQL_SUCCESS_WITH_INFO)) {
		fprintf(stderr, "SQLAllocHandle: SQL_HANDLE_DBC Failed\n");
		SQLFreeHandle(SQL_HANDLE_ENV, environment_handle);
		return KO ;
	}
	/* Mise en place d'un timeout : temps d'attente d'une connexion */
	SQLSetConnectAttr(connection_handle, SQL_LOGIN_TIMEOUT, (SQLPOINTER *) TIMEOUT, 0);
	/* Etape 3. connexion a la source de donnees "datasource" */
	/* On peut aussi utiliser la fonction SQLDriverConnect qui prend plus de parametres */
	status = SQLConnect(connection_handle, (SQLCHAR *) datasource, SQL_NTS, (SQLCHAR *) user, SQL_NTS, (SQLCHAR *) pwd, SQL_NTS);
	if ((status != SQL_SUCCESS) && (status != SQL_SUCCESS_WITH_INFO)) {
		fprintf(stderr, "SQLConnect failed\n");
		SQLGetDiagRec(SQL_HANDLE_DBC, connection_handle, REC_NUMBER, state, &odbc_error, error_text, BUFFER_LEN, &error_text_len);
		fprintf(stderr, "%s (error number : %d)\n", error_text, (int)odbc_error);
		SQLFreeHandle(SQL_HANDLE_DBC, connection_handle);
		SQLFreeHandle(SQL_HANDLE_ENV, environment_handle);
		return KO;
	}
	fprintf(stdout, "Connection to DataSource %s succeeded\n", datasource);
	/* Le nom et la version de la base de donnees */
	SQLGetInfo(connection_handle, SQL_DBMS_NAME, (SQLPOINTER)db_name, sizeof(db_name), NULL);
	SQLGetInfo(connection_handle, SQL_DBMS_VER, (SQLPOINTER)db_version, sizeof(db_version), NULL);
	fprintf(stdout, "Database name : %s\n", db_name);
	fprintf(stdout, "Database version : %s\n", db_version);
	/* Etape 3. allocation de la memoire pour la requete */
	status = SQLAllocHandle(SQL_HANDLE_STMT, connection_handle, &statment_handle);
	if ((status != SQL_SUCCESS) && (status != SQL_SUCCESS_WITH_INFO)) {
		fprintf(stderr, "SQLAllocHandle: SQL_HANDLE_STMT Failed\n");	
		/* On se deconnecte avant de quitter */
		status = SQLDisconnect(connection_handle);
		if ((status != SQL_SUCCESS) && (status != SQL_SUCCESS_WITH_INFO)) {
			fprintf(stderr, "SQLDisconnect failed\n");
			SQLGetDiagRec(SQL_HANDLE_DBC, connection_handle, REC_NUMBER, state, &odbc_error, error_text, BUFFER_LEN, &error_text_len);
			fprintf(stderr, "%s (error number : %d)\n", error_text, (int)odbc_error);
		}
		SQLFreeHandle(SQL_HANDLE_DBC, connection_handle);
		SQLFreeHandle(SQL_HANDLE_ENV, environment_handle);
		return KO;
	}
	/* Etape 4. executer la requete */
	status = SQLExecDirect(statment_handle, (SQLCHAR *)STRING_REQUEST, SQL_NTS);
	if ((status != SQL_SUCCESS) && (status != SQL_SUCCESS_WITH_INFO)) {
		fprintf(stderr, "SQLExecDirect Failed\n");
		/* On se deconnecte avant de quitter */
		status = SQLDisconnect(connection_handle);
		if ((status != SQL_SUCCESS) && (status != SQL_SUCCESS_WITH_INFO)) {
			fprintf(stderr, "SQLDisconnect failed\n");
			SQLGetDiagRec(SQL_HANDLE_DBC, connection_handle, REC_NUMBER, state, &odbc_error, error_text, BUFFER_LEN, &error_text_len);
			fprintf(stderr, "%s (error number : %d)\n", error_text, (int)odbc_error);
		}
		SQLFreeHandle(SQL_HANDLE_STMT, statment_handle);
		SQLFreeHandle(SQL_HANDLE_DBC, connection_handle);
		SQLFreeHandle(SQL_HANDLE_ENV, environment_handle);
		return KO;
	}
	/* Etape 5. recuperer les resultats de la requete */
	/* Correspondance entre les variables et leur position dans la requete :
	 * emp_department_id correspond a department_id, il est en position 1 et est de type int
	 * emp_first_name correspond a first_name, de type string et est en position 2
	 * emp_last_name correspond a last_name, de type string et est en position 3
	 * emp_salary correspond a salary, de type float et est en position 4
	 */
	SQLBindCol(statment_handle, 1, SQL_C_LONG, &emp_department_id, sizeof(emp_department_id), &odbc_error);
	SQLBindCol(statment_handle, 2, SQL_C_CHAR, &emp_first_name, sizeof(emp_first_name), &odbc_error);
	SQLBindCol(statment_handle, 3, SQL_C_CHAR, &emp_last_name, sizeof(emp_last_name), &odbc_error);
	SQLBindCol(statment_handle, 4, SQL_C_FLOAT, &emp_salary, sizeof(emp_salary), &odbc_error);
	while(SQL_NO_DATA != (status = SQLFetch(statment_handle))) {
		fprintf(stdout, "<%d> - <%s> - <%s> - <%f>\n", emp_department_id, emp_first_name, emp_last_name, emp_salary);	
		number_fetched_lines++;
	}
	/* Afficher le nombre de lignes retourné */
	fprintf(stdout, "%d rows fetched\n", number_fetched_lines);

	SQLFreeHandle(SQL_HANDLE_STMT, statment_handle);
	SQLFreeHandle(SQL_HANDLE_DBC, connection_handle);
	SQLFreeHandle(SQL_HANDLE_ENV, environment_handle);
	return OK;
}

int main(int argc, char **argv)
{
	if (argc != 4) {
		fprintf(stdout, "Usage : %s dataSource user password\n", argv[0]);
		return KO;
	}
	odbc_connection(argv[1], argv[2], argv[3]);
	return OK;
}
