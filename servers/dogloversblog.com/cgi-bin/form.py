#!/usr/bin/env python3
import cgi

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields

if form.getvalue('name'):
    name = form.getvalue('name')
else:
    name = "World"

if form.getvalue('happy'):
    happy = form.getvalue('happy')
else:
    happy = "Yes"

print("<html><head><title>CGI Form</title></head><body>")
print(f"<h1>Hello, {name}!</h1>")
print(f"<p>Are you happy today? {happy}</p>")
print("</body></html>")
