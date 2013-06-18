#! /usr/bin/env python

import pyodbc

conn = pyodbc.connect('DSN=Oracle;UID=hr;PWD=human')
curs = conn.cursor()
curs.execute('SELECT * FROM product_component_version')
row = curs.fetchone()
print row
