#!/usr/bin/env python3
import requests

print("""
	<!DOCTYPE html>
	<html lang="en">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Dog Lovers' Blog</title>
		<style>
			body { font-family: Arial, sans-serif; margin: 0; padding: 0; }
			.header { background-color: #b4c796; padding: 20px; text-align: center; }
			.nav { background-color: #333; }
			.nav a { float: left; color: rgb(14, 14, 14); text-align: center; padding: 14px 16px; text-decoration: none; font-size: 17px; }
			.nav a:hover { background-color: #ddd; color: black; }
			.content { padding: 20px; margin-top: 20px;}
			.footer { background-color: #b4c796; padding: 10px; text-align: center; }
			.article { margin-bottom: 20px; }
			.article h2 { color: #333; }
			.article p { color: #666; }
		</style>
	</head>
	<body>

	<div class="header">
		<h1>GoodBoi Randomizer</h1>
		<p>Refresh the page and admire the goodboiness!</p>
	</div>

	<div class="nav">
		<a href="http://localhost:4243/">Home</a>
		<a href="#">Good Boi Randomizer</a>
		<a href="http://localhost:4243/form.html">Random form</a>
		<a href="http://localhost:4243/file_upload.html">Single File Upload</a>
		<a href="http://localhost:4243/multifile_upload.html">Multiple File Upload</a>
	</div>

	<div class="content">
	""")

response = requests.get("https://dog.ceo/api/breeds/image/random")

if response.status_code == 200:
	image_link = response.json()["message"]
	print(f"<p>Here is some random cuteness to enlighten your day:</p>")
	print(f"<img src='{image_link}'/>")
else:
	print(f"<p>Sorry, no dog photo available for now :/</p>")


print("""
	</div>

	<div class="footer">
		<p> 2024 Dog Lovers' Blog.</p>
	</div>

	</body>
	</html>
""")
