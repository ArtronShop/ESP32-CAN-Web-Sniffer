# -*- coding: utf-8 -*-
import os
import re
import subprocess
from datetime import datetime
Import("env")

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

def gen_web():
    # print("PROJECT_DIR: {}".format(PROJECT_DIR))
    web_out_dir = os.path.join(PROJECT_DIR, "src/web")
    if os.path.exists(web_out_dir):
        os.remove(web_out_dir)
    os.mkdir(web_out_dir)
    
    for dirpath, dnames, fnames in os.walk(os.path.join(PROJECT_DIR, "asset/web")):
        for f in fnames:
            file_path = os.path.join(dirpath, f)
            content = open(file_path, mode="r", encoding="utf-8").read()
            file_out = os.path.join(web_out_dir, f + ".h")
            open(file_out, mode="w", encoding="utf-8", newline='').write(content)

def remove_conflicting_file(path):
    file_path = os.path.join(PROJECT_DIR, path)
    if os.path.exists(file_path):
        os.remove(file_path)
        print("Remove file {}".format(path))

genheader()