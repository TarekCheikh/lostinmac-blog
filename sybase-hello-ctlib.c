/*
 * sybase-hello-ctlib.c : Un programme simple qui montre
 * l'utilisation du Sybase Client Library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Header pour la CT-Lib */
#include <ctpublic.h>

#define	CTLIB_VERSION	CS_CURRENT_VERSION	/* Version de la CT-Lib */

#define OK	0
#define KO	1

/* Informations de connexion */
#define DB_SERVER	"TCOSERVER"	/* defini dans le fichier /opt/sybase/interfaces */
/* #define DB_SERVER     NULL */	/* Dans ce cas il faut faire : export DSQUERY="TCOSERVER" */
#define DB_USER		"tarek"
#define DB_PASSWORD	"123456"


#define NUM_CLOMUNS   3
#define MAXSTRING    40
#define	STATELEN     3 

/* Macro utilisee en cas d'erreur */
#define EXIT_ON_FAIL(context, ret, str) \
	if (ret != CS_SUCCEED) \
	{ \
		fprintf(stdout, "Fatal error: %s\n", str); \
		if (context != NULL) \
		{ \
			(CS_VOID) ct_exit(context, CS_FORCE_EXIT); \
			(CS_VOID) cs_ctx_drop(context); \
		} \
		exit(EXIT_FAILURE); \
	}

/* Prototypes des fonctions de gestion des erreurs */
CS_RETCODE CS_PUBLIC cslibmsg_callback(CS_CONTEXT *context, CS_CLIENTMSG *clientmsg);
CS_RETCODE CS_PUBLIC clientmsg_callback(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *clientmsg);
CS_RETCODE CS_PUBLIC servermsg_callback(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *servermsg);

int main(int argc, char **argv)
{
	/* User et mot de passe */
	CS_CHAR *username = DB_USER;
	CS_CHAR *password = DB_PASSWORD;

	CS_CONTEXT         *context;	/* Structure de Contexte */
	CS_CONNECTION      *connection;	/* Structure de Connexion */
	CS_COMMAND         *cmd;	/* Command structure */

	/* Data format structures for column descriptions: */
	CS_DATAFMT          columns[NUM_CLOMUNS];

	CS_INT              datalength[NUM_CLOMUNS];
	CS_SMALLINT         indicator[NUM_CLOMUNS];
	CS_INT              count;
	CS_RETCODE          ret;
	CS_RETCODE          results_ret;
	CS_INT              result_type;
	CS_CHAR             name[MAXSTRING];
	CS_CHAR             city[MAXSTRING];
	CS_CHAR		    state[STATELEN];


	/* Etape 1: Initialisation */
	/* Allocation de la structure de contexte */
	context = NULL;
	ret = cs_ctx_alloc(CTLIB_VERSION, &context);
	EXIT_ON_FAIL(context, ret, "cs_ctx_alloc failed");

	/* Initialisation de la Client-Library */
	ret = ct_init(context, CTLIB_VERSION);
	EXIT_ON_FAIL(context, ret, "ct_init failed");

	/* Etape 2: Mise en place des fonctions de gestion des erreurs et des warnings */

	/* Gestion des erreurs au niveau de la librairie */
	ret = cs_config(context, CS_SET, CS_MESSAGE_CB, (CS_VOID *)cslibmsg_callback, CS_UNUSED, NULL);
	EXIT_ON_FAIL(context, ret, "cs_config(CS_MESSAGE_CB) failed");

	/* Gestion des erreurs au niveau du client */
	ret = ct_callback(context, NULL, CS_SET, CS_CLIENTMSG_CB, (CS_VOID *)clientmsg_callback);
	EXIT_ON_FAIL(context, ret, "ct_callback for client messages failed");

	/* Gestion des erreurs au niveau du serveur */
	ret = ct_callback(context, NULL, CS_SET, CS_SERVERMSG_CB, (CS_VOID *)servermsg_callback);
	EXIT_ON_FAIL(context, ret, "ct_callback for server messages failed");

	/* Etape 3: Connexion au serveur */

	/* Allocation de la structure de connexion */
	ret = ct_con_alloc(context, &connection);
	EXIT_ON_FAIL(context, ret, "ct_con_alloc() failed");

	/* Mise en place du user et du password */
	ret = ct_con_props(connection, CS_SET, CS_USERNAME, username, CS_NULLTERM, NULL);
	EXIT_ON_FAIL(context, ret, "Could not set user name");
	ret = ct_con_props(connection, CS_SET, CS_PASSWORD, password, CS_NULLTERM, NULL);
	EXIT_ON_FAIL(context, ret, "Could not set password");

	/* On se connecte */
	if (DB_SERVER == NULL)
	{
		/* Si aucun serveur n'est fourni, la variable
		 * d'environnement DSQUERY doit contenir le nom
		 * du serveur tel que defini dans le fichier
		 * "/opt/sybase/interfaces"
		 */
		ret = ct_connect(connection, NULL, 0);
	}
	else
	{
		ret = ct_connect(connection, DB_SERVER, CS_NULLTERM);
	}
	EXIT_ON_FAIL(context, ret, "Could not connect!");

	/* Etape 4: envoi d'une requete au serveur */

	/* Allocation de la structure pour envoyer la requete */
	ret = ct_cmd_alloc(connection, &cmd);
	EXIT_ON_FAIL(context, ret, "ct_cmd_alloc() failed");

	/* Mise en place de notre requete */
	ret = ct_command(cmd, CS_LANG_CMD, "SELECT au_lname, city, state from pubs2..authors", CS_NULLTERM, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_command() failed");

	/* On envoi la requete au serveur */
	ret = ct_send(cmd);
	EXIT_ON_FAIL(context, ret, "ct_send() failed");

	/* Etape 5: recuperation du resultat de la requete */
	while ((results_ret = ct_results(cmd, &result_type)) == CS_SUCCEED)
	{
		switch ((int)result_type)
		{
			/* La requete a renvoyee un resultat */
			case CS_ROW_RESULT:

				/* 1ere colonne "au_lname" */
				columns[0].datatype = CS_CHAR_TYPE;
				columns[0].format = CS_FMT_NULLTERM;
				columns[0].maxlength = MAXSTRING;
				columns[0].count = 1;
				columns[0].locale = NULL;
				ret = ct_bind(cmd, 1, &columns[0], name, &datalength[0], &indicator[0]);
				EXIT_ON_FAIL(context, ret, "ct_bind() for au_lname failed");

				/* 2eme colonne "city" */
				columns[1].datatype = CS_CHAR_TYPE;
				columns[1].format = CS_FMT_NULLTERM;
				columns[1].maxlength = MAXSTRING;
				columns[1].count = 1;
				columns[1].locale = NULL;

				ret = ct_bind(cmd, 2, &columns[1], city, &datalength[1], &indicator[1]);
				EXIT_ON_FAIL(context, ret, "ct_bind() for city failed");

				/* 3eme colonne "state" */
				columns[2].datatype = CS_CHAR_TYPE;
				columns[2].format = CS_FMT_NULLTERM;
				columns[2].maxlength = STATELEN;
				columns[2].count = 1;
				columns[2].locale = NULL;

				ret = ct_bind(cmd, 3, &columns[2], state, &datalength[2], &indicator[2]);
				EXIT_ON_FAIL(context, ret, "ct_bind() for state failed");

				/* Fetch des lignes renvoyees */
				while (((ret = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, &count)) == CS_SUCCEED) || (ret == CS_ROW_FAIL))
				{

					/* En cas d'erreur sur une ligne, on le signale */
					if (ret == CS_ROW_FAIL)
					{
						fprintf(stdout, "Error on row %ld.\n", (long)(count + 1));
					}

					/* Affichage de la ligne */
					fprintf(stdout, "%15s | %15s | %3s\n", name, city, state);
				}

				/*
				 * Les lignes renvoyees par la requete ont etaient affichees,
				 * si tout s'est bien passe alors on doit avoir : CS_END_DATA
				 */
				if (ret == CS_END_DATA)
				{
					fprintf(stdout, "\nAll done processing rows.\n");
				}
				/* Une erreur s'est produite */
				else
				{
					EXIT_ON_FAIL(context, CS_FAIL, "ct_fetch failed");
				}

				break;

			case CS_CMD_SUCCEED:
				/* La requete s'est bien passee, mais n'a renvoyee aucune ligne */
				fprintf(stdout, "No rows returned.\n");
				break;

			case CS_CMD_FAIL:
				/* Une erreur s'est produite lors de l'execution de la requete */
				break;

			case CS_CMD_DONE:
				break;

			default:
				/* Une erreur inatendue s'est produite */
				EXIT_ON_FAIL(context, CS_FAIL, "ct_results returned unexpected result type");
				break;
		}
	}

	/* Analyse du resultat de l'etape precedente et sortie en erreur au cas ou */
	switch ((int)results_ret)
	{
		case CS_END_RESULTS:

			/* Tout est OK */
			break;

		case CS_FAIL:
			/* Une erreur lors du traitement */
			EXIT_ON_FAIL(context, CS_FAIL, "ct_results() returned CS_FAIL.");
			break;

		default:
			/* Une erreur inatendue s'est produite */
			EXIT_ON_FAIL(context, CS_FAIL, "ct_results returned unexpected return code");
			break;
	}

	/* Etape 6: liberation de la memoire, deconnexion */

	/* Liberation de la requete */
	ret = ct_cmd_drop(cmd);
	EXIT_ON_FAIL(context, ret, "ct_cmd_drop failed");

	/* Fermeture de la connexion et liberation de la memoire associee a sa structure */
	ret = ct_close(connection, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_close failed");
	ret = ct_con_drop(connection);
	EXIT_ON_FAIL(context, ret, "ct_con_drop failed");

	/* Quitter la CT-Lib et liberer la memoire */
	ret = ct_exit(context, CS_UNUSED);
	EXIT_ON_FAIL(context, ret, "ct_exit failed");

	ret = cs_ctx_drop(context);
	EXIT_ON_FAIL(context, ret, "cs_ctx_drop failed");

	return OK;
}

/* Fonction de gestion des erreurs au niveau du serveur */
CS_RETCODE CS_PUBLIC servermsg_callback(CS_CONTEXT *cp, CS_CONNECTION *chp, CS_SERVERMSG *msgp)
{
	fprintf(stdout, "Server message:\n\t");
	fprintf(stdout, "number(%ld) severity(%ld) state(%ld) line(%ld)\n",
			(long)msgp->msgnumber, (long)msgp->severity,
			(long)msgp->state, (long)msgp->line);

	if (msgp->svrnlen > 0)
		fprintf(stdout, "\tServer name: %s\n", msgp->svrname);

	if (msgp->proclen > 0)
		fprintf(stdout, "\tProcedure name: %s\n", msgp->proc);

	fprintf(stdout, "\t%s\n", msgp->text);

	return CS_SUCCEED;
}

/* Fonction de gestion des erreurs au niveau du client */
CS_RETCODE CS_PUBLIC clientmsg_callback(CS_CONTEXT *context, CS_CONNECTION *conn, CS_CLIENTMSG *emsgp)
{
	fprintf(stdout, "Client Library error:\n\t");
	fprintf(stdout, "severity(%ld) number(%ld) origin(%ld) layer(%ld)\n",
			(long)CS_SEVERITY(emsgp->severity),
			(long)CS_NUMBER(emsgp->msgnumber),
			(long)CS_ORIGIN(emsgp->msgnumber),
			(long)CS_LAYER(emsgp->msgnumber));

	fprintf(stdout, "\t%s\n", emsgp->msgstring);

	if (emsgp->osstringlen > 0)
	{
		fprintf(stdout, "Operating system error number(%ld):\n", (long)emsgp->osnumber);
		fprintf(stdout, "\t%s\n", emsgp->osstring);
	}

	return CS_SUCCEED;
}

/* Fonction de gestion des erreurs au niveau de la librairie */
CS_RETCODE CS_PUBLIC cslibmsg_callback(CS_CONTEXT *context, CS_CLIENTMSG *emsgp)
{
	fprintf(stdout, "CS-Library error:\n");
	fprintf(stdout, "\tseverity(%ld) layer(%ld) origin(%ld) number(%ld)",
			(long)CS_SEVERITY(emsgp->msgnumber),
			(long)CS_LAYER(emsgp->msgnumber),
			(long)CS_ORIGIN(emsgp->msgnumber),
			(long)CS_NUMBER(emsgp->msgnumber));

	fprintf(stdout, "\t%s\n", emsgp->msgstring);

	if (emsgp->osstringlen > 0)
	{
		fprintf(stdout, "Operating System Error: %s\n", emsgp->osstring);
	}

	return CS_SUCCEED;
}
