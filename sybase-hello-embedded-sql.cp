/*
 *	sybase-hello-embedded-sql.cp : est simple programme
 *	qui montre l'utilisation du "Embedded SQL" pour Sybase
 */

#include <stdio.h>
#include <stdlib.h>
/* Include des headers pour "Embedded SQL" */
#include <sybhesql.h>
#include <sybtesql.h>
EXEC SQL INCLUDE SQLCA;

/* Les identifiants pour se connecter a la base */
#define SYBASE_USER	"tarek"
#define SYBASE_PASSWORD	"123456"
#define	SYBASE_SERVER	"TCOSERVER"	/* Defini dans le fichier "/opt/sybase/interfaces" */

#define OK	0	
#define KO	1

/* Define des longueurs des clonnes */
#define	AU_ID_LEN	11
#define	AU_LNAME_LEN	40	
#define	AU_FNAME_LEN	20
#define	PHONE_LEN	12
#define	ADDRESS_LEN	40
#define	CITY_LEN	20
#define	STATE_LEN	2
#define	COUNTRY_LEN	12
#define	POSTALCODE_LEN	10

void	error_handler();
void	warning_handler();

int main(int argc, char **argv)
{
	EXEC SQL BEGIN DECLARE SECTION;
	/* Pour le login, pass et le serveur */
	char	username[30];
	char	password[30];
	char    server[30];
	/* Pour les colonnes */
	char	au_id_var[AU_ID_LEN + 1];
	char	au_lname_var[AU_LNAME_LEN + 1];
	char	au_fname_var[AU_FNAME_LEN + 1];
	char	phone_var[PHONE_LEN + 1];
	char	address_var[ADDRESS_LEN + 1];
	char	city_var[CITY_LEN + 1];
	char	state_var[STATE_LEN + 1];
	char	country_var[COUNTRY_LEN + 1];
	char	postalcode_var[POSTALCODE_LEN + 1];
	EXEC SQL END DECLARE SECTION;

	/* Mise en place de la gestion des erreurs */
	EXEC SQL WHENEVER SQLERROR CALL error_handler();
	EXEC SQL WHENEVER SQLWARNING CALL warning_handler();
	EXEC SQL WHENEVER NOT FOUND CONTINUE;

	/* Connexion au serveur */ 
	strcpy(username, SYBASE_USER);
	strcpy(password, SYBASE_PASSWORD);
	strcpy(server, "TCOSERVER");

	EXEC SQL CONNECT :username IDENTIFIED BY :password using :server;

	/* On veut utiliser la base "PUBS2" */
	EXEC SQL USE pubs2;

	/* Declarer un cursor pour notre requete sur la table 'authors' */
	EXEC SQL DECLARE authors_list CURSOR FOR
		SELECT * FROM authors;		/* Attention les noms sont 'case sensitive' */
	/* Ouverture du curseur */
	EXEC SQL OPEN authors_list;

	/* Parcourir et afficher le resultat de la requete */
	for (;;) {
		EXEC SQL FETCH authors_list INTO 
			:au_id_var, :au_lname_var, :au_fname_var, :phone_var,
			:address_var, :city_var, :state_var, :country_var, :postalcode_var;
		/* Si l'erreur sqlca.sqlcode est 100 : qui veut dire qu'il n' y a
		 * plus de row dans le curseur, on arrete
		 */
		if (sqlca.sqlcode == 100)
			break;
		/* Affichage ligne par ligne */
		fprintf(stdout, "%s|%s|%s|%s|%s|%s|%s|%s|%s\n", au_id_var, au_lname_var, au_fname_var, \
				phone_var, address_var, city_var, state_var, country_var, postalcode_var);
	}

	/* On ferme le curseur */
	EXEC SQL CLOSE authors_list;
	/* On se deconnecte "CURRENT" ici veut dire la connexion courante */
	EXEC SQL DISCONNECT CURRENT;

	return OK;
}

/* Pour les erreurs */
void error_handler(void)
{
	fprintf(stderr, "\n** SQLCODE=(%ld)", sqlca.sqlcode);

	if (sqlca.sqlerrm.sqlerrml)
	{
		fprintf(stderr, "\n** ASE Error ");
		fprintf(stderr, "\n** %s", sqlca.sqlerrm.sqlerrmc);
	}

	fprintf(stderr, "\n\n");

	exit(EXIT_FAILURE);
}

/* Pour les warnings */
void warning_handler(void)
{
	if (sqlca.sqlwarn[1] == 'W')
	{
		fprintf(stderr, "\n** Data truncated.\n");
	}

	if (sqlca.sqlwarn[3] == 'W')
	{
		fprintf(stderr, "\n** Insufficient host variables to store results.\n");
	}	
	return;
}

