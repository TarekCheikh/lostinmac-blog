#! /usr/bin/env python

import cx_Oracle
import time

# Les informations necessaires pour se connecter a la base
db_user = 'hr'
db_password = 'human'
db_server = '192.168.1.95'
db_port = '1521'
db_name = 'TOCORACL'

# On veut juste savoir ca prend combien de temps
start_time = time.clock()
# On se connecte
connection = cx_Oracle.connect("%s/%s@%s:%s/%s" %
    (db_user, db_password, db_server, db_port, db_name))

# On cree un curseur pour notre requete
curs = connection.cursor()

# on execuste notre requete
curs.execute("""SELECT department_id, first_name, last_name, salary
    FROM HR.employees
    WHERE department_id is not null 
    """)

print "+------------------+------------------+------------------+------------------+"
print "|",
# On recupere les noms des colonnes
columns_names = curs.description
for column in columns_names:
    print "%16s |" %column[0],
print "\n+------------------+------------------+------------------+------------------+"

# On affiche le resultat
for row in curs:
    for column in row:
        print "%16s | " %column,
    print ""

# Affichage du temps mis  par le triatement
elapsed_time = time.clock() - start_time
print "Elapsed time = %f s" % elapsed_time

# On ferme la connexion
connection.close()
