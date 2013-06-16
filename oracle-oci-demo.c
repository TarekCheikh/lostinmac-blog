/*
 *	oracle-oci-demo.c
 *	Un simple program qui demontre l'uilisation
 *	de la librairie Oracle oci (Oracle Call Interface API)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oci.h>

#define CONNECTION_STRING       "TOCORACL"	/* Cette valeur se trouve dans votre tnsnames.ora */

#define	OK	0
#define	KO	1

#define	BUFFER_SIZE	512

/* Nombre de colonnes dans le resultat de notre requete */
#define NUM_FETCHED_COLUMNS 	4

#define SELECT_STRING	"SELECT department_id, first_name, last_name, salary FROM HR.employees WHERE department_id is not null"

/* Les identifiants pour se connecter a la base */
text *username = (text *) "hr";
text *password = (text *) "human";

int main(int argc, char **argv)
{
	OCIEnv		*env_handle = NULL;		/* Pour mettre en place notre environnement */
	OCIServer	*server_handle = NULL;		/* Pour le serveur */
	OCIError	*error_handle = NULL;		/* Pour les erreurs */
	OCISession	*user_session_handle = NULL;	/* Pour la session */
	OCISvcCtx	*service_handle = NULL;		/* Pour le contexte de la connexion (login, pass et serveur) */
	sword errcode = 0;
	text errbuf[BUFFER_SIZE];
	OCIStmt   *statement_handle = (OCIStmt *)0;
	OCIDefine *defhp[NUM_FETCHED_COLUMNS];
	text       select_query[256];
	ub4        emp_department_id = 0;
	text       emp_first_name[21];
	text       emp_last_name[26];
	float      emp_salary = 0.0;

	/* Etape 1 : Creation de l'environnement */
	errcode = OCIEnvCreate(&env_handle, OCI_DEFAULT, NULL, NULL, NULL, NULL, 0, NULL);
	if (errcode != 0) {
		fprintf(stderr, "OCIEnvCreate failed with errcode = %d.\n", errcode);
		return KO;
	}

	/* 
	 * Etape 2 : Allocations memoire pour les differents "handles" : 
	 * error_handle, service_handle, server_handle et user_session_handle 
	 */

	errcode = OCIHandleAlloc((void *)env_handle, (void **)&error_handle, OCI_HTYPE_ERROR, 0, NULL);
	if (errcode == OCI_INVALID_HANDLE) {
		fprintf(stderr, "OCIHandleAlloc : error_handle failed with errcode = %d.\n", errcode);
		return KO;
	}
	errcode = OCIHandleAlloc((void *)env_handle, (void **)&service_handle, OCI_HTYPE_SVCCTX, 0, NULL);
	if (errcode == OCI_INVALID_HANDLE) {
		fprintf(stderr, "OCIHandleAlloc : service_handle failed with errcode = %d.\n", errcode);
		return KO;
	}
	errcode = OCIHandleAlloc((void *)env_handle, (void **)&server_handle, OCI_HTYPE_SERVER, 0, NULL);
	if (errcode == OCI_INVALID_HANDLE) {
		fprintf(stderr, "OCIHandleAlloc : server_handle failed with errcode = %d.\n", errcode);
		return KO;
	}
	errcode = OCIHandleAlloc((void *)env_handle, (void **)&user_session_handle, OCI_HTYPE_SESSION, 0, NULL);
	if (errcode == OCI_INVALID_HANDLE) {
		fprintf(stderr, "OCIHandleAlloc : user_session_handle failed with errcode = %d.\n", errcode);
		return KO;
	}
	/* fin des allocations */

	/* 2.e. creation du contexte serveur */
	OCIServerAttach(server_handle, error_handle, (text *)CONNECTION_STRING, strlen(CONNECTION_STRING), OCI_DEFAULT);
	/* 2.f. Definition du serveur */
	OCIAttrSet((void *)service_handle, OCI_HTYPE_SVCCTX, (void *)server_handle, (ub4) 0, OCI_ATTR_SERVER, error_handle);
	/* 2.g. Definition du user */
	OCIAttrSet((void *)user_session_handle, OCI_HTYPE_SESSION, (void *)username, (ub4)strlen((char *)username), OCI_ATTR_USERNAME, error_handle);
	/* 2.h. Definition du mot de pass */
	OCIAttrSet((void *)user_session_handle, OCI_HTYPE_SESSION, (void *)password, (ub4)strlen((char *)password), OCI_ATTR_PASSWORD, error_handle);
	/* 2.i. Definition du service */
	OCIAttrSet((void *)service_handle, OCI_HTYPE_SVCCTX, (void *)user_session_handle, (ub4) 0, OCI_ATTR_SESSION, error_handle);
	/* 2.j. Initialisation de la session */
	errcode = OCISessionBegin(service_handle, error_handle, user_session_handle, OCI_CRED_RDBMS, (ub4) OCI_DEFAULT);
	if (errcode == OCI_ERROR)
	{
		OCIErrorGet((dvoid *)error_handle, (ub4) 1, (text *) NULL, &errcode,
				errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
		fprintf(stdout, "Error - %.*s\n", 512, errbuf);
		OCIHandleFree((dvoid *) env_handle, OCI_HTYPE_ENV);
		return KO;

	}
	/* A ce stade on est bien connecté */
	fprintf(stdout, "Connection succeed\n");
	/* Fin de l'etape 2 */

	strcpy(select_query, SELECT_STRING);
	/* Allocation mémoire pour la requete */
	errcode = OCIHandleAlloc(env_handle, (dvoid **)&statement_handle, OCI_HTYPE_STMT, (size_t)0, (dvoid **)0);
	if (errcode == OCI_INVALID_HANDLE) {
		fprintf(stderr, "OCIHandleAlloc : statement_handle failed with errcode = %d.\n", errcode);
		OCIHandleFree((dvoid *) env_handle, OCI_HTYPE_ENV);
		return KO;
	}
	/*  Preaparation de la requete */
	errcode = OCIStmtPrepare(statement_handle, error_handle, (text *)select_query, (ub4)strlen((const signed char *)select_query), OCI_NTV_SYNTAX, OCI_DEFAULT);
	if (errcode != OCI_SUCCESS) {
		fprintf(stderr, "OCIStmtPrepare failed with errcode = %d.\n", errcode);
		OCIHandleFree((dvoid *) env_handle, OCI_HTYPE_ENV);
		return KO;
	}

	/* Correspondance entre les variables et leur position dans la requete :
	 * emp_department_id correspond a department_id, il est en position 1 et est de type int
	 * emp_first_name correspond a first_name, de type string et est en position 2
	 * emp_last_name correspond a last_name, de type string et est en position 3
	 * emp_salary correspond a salary, de type float et est en position 4
	 */
	errcode = OCIDefineByPos(statement_handle, &defhp[0], error_handle, (ub4)1, (dvoid *)&emp_department_id, \
			(sb4)sizeof(ub4), (ub2)SQLT_INT, (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT);
	if (errcode != OCI_SUCCESS) {
		fprintf(stderr, "OCIDefineByPos : emp_department_id failed with errcode = %d.\n", errcode);
		OCIHandleFree((dvoid *) env_handle, OCI_HTYPE_ENV);
		return KO;
	}
	errcode = OCIDefineByPos(statement_handle, &defhp[1], error_handle, (ub4)2, (dvoid *)&emp_first_name, \
			(sb4)sizeof(emp_first_name), (ub2)SQLT_STR, (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT);
	if (errcode != OCI_SUCCESS) {
		fprintf(stderr, "OCIDefineByPos : emp_first_name failed with errcode = %d.\n", errcode);
		OCIHandleFree((dvoid *) env_handle, OCI_HTYPE_ENV);
		return KO;
	}
	errcode = OCIDefineByPos(statement_handle, &defhp[2], error_handle, (ub4)3, (dvoid *)&emp_last_name, \
			(sb4)sizeof(emp_last_name), (ub2)SQLT_STR, (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT);
	if (errcode != OCI_SUCCESS) {
		fprintf(stderr, "OCIDefineByPos : emp_last_name failed with errcode = %d.\n", errcode);
		OCIHandleFree((dvoid *) env_handle, OCI_HTYPE_ENV);
		return KO;
	}
	errcode = OCIDefineByPos(statement_handle, &defhp[3], error_handle, (ub4)4, (dvoid *)&emp_salary, \
			(sb4)sizeof(float), (ub2)SQLT_FLT, (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT); 
	if (errcode != OCI_SUCCESS) {
		fprintf(stderr, "OCIDefineByPos : emp_salary failed with errcode = %d.\n", errcode);
		OCIHandleFree((dvoid *) env_handle, OCI_HTYPE_ENV);
		return KO;
	}

	/* Execution de la requete */
	errcode = OCIStmtExecute(service_handle, statement_handle, error_handle, (ub4)0, (ub4)0, (OCISnapshot *)0, (OCISnapshot *)0, OCI_DEFAULT );
	if (errcode != OCI_SUCCESS)
	{
		OCIErrorGet((dvoid *)error_handle, (ub4) 1, (text *) NULL, &errcode, errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
		fprintf(stdout, "Error - %.*s\n", 512, errbuf);
		OCIHandleFree((dvoid *) env_handle, OCI_HTYPE_ENV);
		return KO;
	}

	/* Recuperation du resultat de la requete */
	errcode = OCIStmtFetch2(statement_handle, error_handle, 1, OCI_FETCH_NEXT, (sb4) 0, OCI_DEFAULT);
	if (errcode == OCI_NO_DATA) {
		fprintf(stdout, "No data found for this request!\n");
	}
	else if (errcode != OCI_SUCCESS) {
		OCIErrorGet((dvoid *)error_handle, (ub4) 1, (text *) NULL, &errcode, errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
		fprintf(stdout, "Error - %.*s\n", 512, errbuf);
		OCIHandleFree((dvoid *) env_handle, OCI_HTYPE_ENV);
		return KO;
	}

	while (errcode == OCI_SUCCESS)
	{
		fprintf(stdout, "Emp_First_Name : %s -- Emp_Last_name : %s -- Emp_Deptno : %u -- Sal : %.2f\n", emp_first_name, emp_last_name, emp_department_id, emp_salary);
		errcode = OCIStmtFetch2(statement_handle, error_handle, 1, OCI_FETCH_NEXT, (sb4) 0, OCI_DEFAULT);
	}

	/* On libere la memoire de notre traitement */
	OCISessionEnd(service_handle, error_handle, user_session_handle, OCI_DEFAULT);
	OCIServerDetach(server_handle, error_handle, OCI_DEFAULT);
	OCIHandleFree((dvoid *) env_handle, OCI_HTYPE_ENV);
	OCITerminate(OCI_DEFAULT);

	return 0;
}
