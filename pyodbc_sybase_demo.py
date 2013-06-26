#! /usr/bin/env python

import pyodbc

# On se connecte a notre base (comme defini dans odbc.ini)
db_connection = pyodbc.connect('DSN=SybasePubs2;UID=tarek1;PWD=123456')
# On cree un curseur pour faire des requetes
cursor = db_connection.cursor()
# On execute une requete
cursor.execute("SELECT * FROM sysobjects WHERE type = 'U'")
print "Dumping all tables Data from Sybase pubs2 database :"
# On parcours et on affiche le resultat de la requete
while 1:
	row = cursor.fetchone()
	# La table 'au_pix' de la base 'pubs2' contient des donnees binaires
	# qu'on souhaite pas afficher
	if not row or (row[0] == 'au_pix'):
		break
	print "Dumping table:", row[0]
	print "\n"
	print "###########################################"
	print "\n"
	db_connection_table = pyodbc.connect('DSN=SybasePubs2;UID=tarek1;PWD=123456')
	table_cursor = db_connection_table.cursor()
	table_request = "SELECT * from " + row[0]
	table_cursor.execute(table_request)
	while 1:
		table_row = table_cursor.fetchone()
		if not table_row:
			break
		print table_row
	print "\n"
	print "###########################################"
	print "\n"
	table_cursor.close()
	db_connection_table.close()
cursor.close()
db_connection.close()
