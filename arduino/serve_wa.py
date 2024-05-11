#!/usr/bin/env python

from flask import Flask, request
import re

app = Flask(__name__)
id = 19


@app.route('/')
def index():
    with open("IndexHtml.cpp", 'r') as file:
        data = file.read()
        match = re.search(r'R"html\((.*?)\)html"', data, re.DOTALL)
        return match.group(1)


@app.route('/settings')
def get_settings():
    return f"""id={id}"""


@app.route('/upload', methods=['POST'])
def upload_file():
    # Check if the POST request has the file part
    if 'file' not in request.files:
        return 'No file part'
    # file = request.files['file']
    return 'success'


if __name__ == '__main__':
    app.run(debug=True)
