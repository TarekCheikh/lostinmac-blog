/*
 * hello-sybase-odbc.c : ce programme demontre
 * une simple utilisation d'UnixODBC avec Sybase
 */
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>

#define	BUFFER_LEN		1024
#define	OK			0
#define	KO			-1
#define	TIMEOUT			5	/* Le timeout est fixe a 5 seconds */
#define	REC_NUMBER		1	/* Pour la fonction SQLGetDiagRec */
#define	SQLSTATE_LEN		7
#define	DB_NAME_LEN		128
#define	DB_VERSION_LEN		32

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
	char result_buffer[1024];		/* Pour chaque ligne du resultat */
	SQLRETURN ret;
	SQLSMALLINT columns;			/* Nombre de colonnes dans le resultat */
	int i;

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
	/* Etape 4. Envoyer une requete via la procedure sp_clearstats, qui affiche les stats.
	 * CECI NE FONCTIONNERA PAS SI VOTRE DRIVER ODBC EST FREETDS!
	 */
	status = SQLExecDirect(statment_handle, (SQLCHAR *)"sp_clearstats", SQL_NTS);
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

	SQLNumResultCols(statment_handle, &columns);
	/* Etape 5. recuperer les resultats de la requete */
	while(SQL_NO_DATA != (status = SQLFetch(statment_handle))) {
		for (i = 1; i <= columns; i++) {
			/* On recupere le buffer de chaque ligne du resultat */
			ret = SQLGetData(statment_handle, i, SQL_C_CHAR, result_buffer, sizeof(result_buffer), NULL);
			if (ret == SQL_SUCCESS) {
				fprintf(stdout, "%s | ",  result_buffer);	
			}
		}
		fprintf(stdout, "\n");
	}

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
