#!/usr/bin/env python

from flask import Flask, request
import re

app = Flask(__name__)


class C:
    id = 19
    key = '112233445566'
    rec_size = 512
    rec_bits = 2


@app.route('/')
def index():
    with open("IndexHtml.cpp", 'r') as file:
        data = file.read()
        match = re.search(r'R"html\((.*?)\)html"', data, re.DOTALL)
        return match.group(1)


@app.route('/settings', methods=['GET', 'POST'])
def settings():
    if request.method == 'GET':
        return f"id={C.id}\nkey={C.key}\nrec-size={C.rec_size}\nrec-bits={C.rec_bits}"
    if request.method == 'POST':
        C.id = int(request.form.get('id'))
        C.key = request.form.get('key')
        C.rec_size = int(request.form.get('rec-size'))
        C.rec_bits = int(request.form.get('rec-bits'))
        return index()


@app.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return 'No file part'
    return 'success'


if __name__ == '__main__':
    app.run(debug=True)
