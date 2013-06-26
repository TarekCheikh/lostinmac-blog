#! /usr/bin/env python

import Sybase

# Connexion a la base "pubs2". 'TCOSERVER' est l'adresse du serveur telle que defini dans "/opt/sybase/interfaces"
conn = Sybase.connect('TCOSERVER', 'tarek1', '123456', 'pubs2') 

# Creer un curseur
cursor = conn.cursor()

# Executer une requete
cursor.execute("select * from titles where price > 15.00")

# Recuperer et afficher le resultat
rows = cursor.fetchall()
for row in rows:
	print row
