/*
 * ocilib-simple-demo.c : montre un exemple d'utilisation
 * de la librairie OCILIB 
 */
#include <ocilib.h>
#include <stdio.h>
#include <stdlib.h>

/* User, Pass et Nom de la base(ou service) */
#define	DB_USER	"hr"
#define	DB_PASS	"human"
#define	DB_NAME	"TOCORACL"

/* Notre requete */
#define	DB_REQUEST	"SELECT department_id, first_name, last_name, salary FROM employees WHERE PHONE_NUMBER is not null"

/* Pour tracer les erreurs */
void error_handler(OCI_Error *err)
{
	printf("Error ORA-%05d - msg : %s\n", OCI_ErrorGetOCICode(err), OCI_ErrorGetString(err));	
}

int main(int argc, char **argv)
{
	OCI_Connection *connection_handle;		/* Pour se connecter a Oracle */
	OCI_Statement *statment_handle;			/* Pour notre requete */
	OCI_Resultset *result_handle;			/* Pour recuperer le resultat de la requete */
	int status;

	/* Initialiser la librarie */
	if (!OCI_Initialize(error_handler, NULL, OCI_ENV_DEFAULT))
		return EXIT_FAILURE;

	/* Connexion a la base Oracle */
	connection_handle = OCI_ConnectionCreate(DB_NAME, DB_USER, DB_PASS, OCI_SESSION_DEFAULT);

	/* Probleme de connexion */
	if (connection_handle == NULL)
		return EXIT_FAILURE;

	/* La connexion est OK */
	/* On affiche quelques infos a propos du serveur */
	printf("<%s>\n", OCI_GetVersionServer(connection_handle));
	printf("Server major version : %d\n", OCI_GetServerMajorVersion(connection_handle));
	printf("Server minor version : %d\n", OCI_GetServerMinorVersion(connection_handle));
	printf("Server revision version : %d\n", OCI_GetServerRevisionVersion(connection_handle));
	printf("Connection version : %d\n", OCI_GetVersionConnection(connection_handle));

	/* On va constuire notre requete */
	statment_handle = OCI_StatementCreate(connection_handle);

	/* "Houston houston ona un probleme" ==> Le plus probable c'est que ca soit un probleme d'allocation mémoire */
	if (statment_handle == NULL)
		return EXIT_FAILURE;

	/* On execute notre requete */
	status = OCI_ExecuteStmt(statment_handle, DB_REQUEST);
	if (status == FALSE)
		return EXIT_FAILURE;

	/* On recupere le resultat de la requete */
	result_handle = OCI_GetResultset(statment_handle);

	/* On parcourt le resultat */
	while (OCI_FetchNext(result_handle)) {
		/* Notre requete attend :
		 * Un int en premiere position ==> department_id
		 * Une chaine en seconde position ==> first_name
		 * Une chaine en troisieme position ==> last_name
		 * Un float en quatrieme position ==> salary
		 */
		printf("<%d>, <%s>, <%s>, <%.2f>\n", OCI_GetInt(result_handle,  1), \
				OCI_GetString(result_handle, 2), \
				OCI_GetString(result_handle, 3), \
				OCI_GetFloat(result_handle, 4));
	}
	
	/* On affiche le nombre de colonnes recupereés */
	printf("\n%d row(s) fetched\n", OCI_GetRowCount(result_handle));

	/* Avant de quitter on ferme la connexion et on libere la mémoire comme des grands :- */
	OCI_ConnectionFree(connection_handle);
	OCI_Cleanup();
	return EXIT_SUCCESS;
}
