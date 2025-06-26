# -*- coding: utf-8 -*-
Import("env")
import os
import re
import subprocess
import json
import shutil
# env.Execute("$PYTHONEXE -m pip install -vvv htmlmin")
import htmlmin
from datetime import datetime


PROJECT_DIR = env.subst("$PROJECT_DIR")

def genheader():
    revision = (
        subprocess.check_output(["git", "describe", "--tags", "--dirty", "--always"])
        .strip()
        .decode("utf-8")
    )
    content = ""
    content = content + "#define FW_VERSION \"%s\"\n" % revision
    content = content + "#define FW_BUILD_DATETIME \"%s\"\n" % datetime.today().strftime('%Y-%m-%d %H:%M:%S')
    open(os.path.join(PROJECT_DIR, "include/genheader.h"), mode="w", encoding="utf-8", newline='').write(content)

def remove_all(folder):
    for filename in os.listdir(folder):
        file_path = os.path.join(folder, filename)
        try:
            if os.path.isfile(file_path) or os.path.islink(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)
        except Exception as e:
            print('Failed to delete %s. Reason: %s' % (file_path, e))

def gen_web():
    # print("PROJECT_DIR: {}".format(PROJECT_DIR))
    web_out_dir = os.path.join(PROJECT_DIR, "src/web")
    if os.path.exists(web_out_dir):
        remove_all(web_out_dir)
    else:
        os.mkdir(web_out_dir)
    
    for dirpath, dnames, fnames in os.walk(os.path.join(PROJECT_DIR, "asset/web")):
        for f in fnames:
            file_path = os.path.join(dirpath, f)
            content = open(file_path, mode="r", encoding="utf-8").read()
            content = htmlmin.minify(content)
            file_out = os.path.join(web_out_dir, f.replace(".", "_") + ".h")
            open(file_out, mode="w", encoding="utf-8", newline='').write("const char * " + f.replace(".", "_") + " = " + json.dumps(content) + ";")

def remove_conflicting_file(path):
    file_path = os.path.join(PROJECT_DIR, path)
    if os.path.exists(file_path):
        os.remove(file_path)
        print("Remove file {}".format(path))

genheader()
gen_web()
