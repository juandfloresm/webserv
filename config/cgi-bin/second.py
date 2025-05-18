#! /usr/bin/env python38

import html
import time
import cgi, cgitb 

cgitb.enable()

hit_count = 0

header = "Content-type: text/html\r\n\r\n"

date_string = time.strftime('%A, %B %d, %Y at %I:%M:%S %p %Z')

html = """
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>Current date</title>
</head>
<body>
  <p>
  Date: {0}
  </p>
  <p>
  Hit count: {1}
  </p>
</body>
</html>
""".format(html.escape(date_string), html.escape(str(hit_count)))

print(header + html)