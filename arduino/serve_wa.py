#!/usr/bin/env python

from flask import Flask, request, send_file
import io
import random
import re
import subprocess

app = Flask(__name__)


class Config:
    def __init__(self):
        self.id = 19
        self.key = '112233445566'
        self.rec_size = 512
        self.rec_bits = 2
        self.rec_days = 1
        self.record = self.biased_random_record()

    def biased_random_record(self):
        n = 2 ** self.rec_bits
        weights = [0.9] + [0.1 / (n - 1) for _ in range(1, n)]
        return random.choices(range(n), weights=weights, k=self.rec_size)


c = Config()


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
        match = re.search(r'R"html\((.*?)\)html";', data, re.DOTALL)
        content = match.group(1)
        content_lines = content.splitlines()
        version_idx = [i for i, line in enumerate(content_lines)
                       if "PROJECT_VERSION" in line][0]
        version = content_lines[version_idx]
        version = re.sub(r'\)html" .* R"html\(',
                         f"{get_project_version()}-{get_git_revision()}",
                         version)
        content_lines[version_idx] = version
        return "\n".join(content_lines)


@app.route('/favicon.ico')
def favicon():
    with open('favicon.cpp', 'r') as f:
        content = f.read()
    match = re.search(r'unsigned char icon32_png\[\] = \{([^\}]+)\};', content)
    byte_values = match.group(1).strip().split(',')
    byte_values = [int(b.strip(), 16) for b in byte_values if b.strip()]
    img_io = io.BytesIO(bytes(byte_values))
    return send_file(img_io, mimetype='image/png')


@app.route('/settings', methods=['GET', 'POST'])
def settings():
    if request.method == 'GET':
        r = f"id={c.id}\nkey={c.key}\nrec-size={c.rec_size}"
        r += f"\nrec-bits={c.rec_bits}\nrec-days={c.rec_days}"
        return r
    if request.method == 'POST':
        c.id = int(request.form.get('id'))
        c.key = request.form.get('key')
        c.rec_size = int(request.form.get('rec-size'))
        c.rec_bits = int(request.form.get('rec-bits'))
        c.rec_days = int(request.form.get('rec-days'))
        return index()


@app.route('/record')
def record():
    return ' '.join((f'{i}:{c}' for i, c in enumerate(c.record) if c > 0))


@app.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return 'No file part'
    return 'success'


if __name__ == '__main__':
    app.run(debug=True)
