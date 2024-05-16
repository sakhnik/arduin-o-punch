#!/usr/bin/env python

from flask import Flask, request
import re
import subprocess

app = Flask(__name__)


class C:
    id = 19
    key = '112233445566'
    rec_size = 512
    rec_bits = 2
    rec_days = 1


def get_project_version():
    version_pattern = re.compile(r'^project\(.*VERSION\s+([0-9.-]+)',
                                 re.IGNORECASE)
    with open('CMakeLists.txt', 'r') as file:
        for line in file:
            match = version_pattern.search(line)
            if match:
                return match.group(1)
    return None


def get_git_revision():
    try:
        result = subprocess.run(['git', 'rev-parse', '--short', 'HEAD'],
                                check=True, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, text=True)
        git_revision = result.stdout.strip()
        return git_revision
    except subprocess.CalledProcessError as e:
        print(f"Error occurred while getting Git revision: {e.stderr}")
        return None


@app.route('/')
def index():
    with open("IndexHtml.cpp", 'r') as file:
        data = file.read()
        match = re.search(r'R"html\((.*?)\)html"', data, re.DOTALL)
        content = match.group(1)
        content_lines = content.splitlines()
        version_idx = [i for i, line in enumerate(content_lines)
                       if "PROJECT_VERSION" in line][0]
        version = content_lines[version_idx]
        version = re.sub(r'"\)html .* R"html\(',
                         f"{get_project_version()}-{get_git_revision()}",
                         version)
        content_lines[version_idx] = version
        return "\n".join(content_lines)


@app.route('/settings', methods=['GET', 'POST'])
def settings():
    if request.method == 'GET':
        r = f"id={C.id}\nkey={C.key}\nrec-size={C.rec_size}"
        r += f"\nrec-bits={C.rec_bits}\nrec-days={C.rec_days}"
        return r
    if request.method == 'POST':
        C.id = int(request.form.get('id'))
        C.key = request.form.get('key')
        C.rec_size = int(request.form.get('rec-size'))
        C.rec_bits = int(request.form.get('rec-bits'))
        C.rec_days = int(request.form.get('rec-days'))
        return index()


@app.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return 'No file part'
    return 'success'


if __name__ == '__main__':
    app.run(debug=True)
