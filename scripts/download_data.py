import argparse
import gdown
import os
import shutil
import pathlib


parser = argparse.ArgumentParser(description='')
parser.add_argument(
        "-d", "--data",
        type=str,
        nargs='*',
        help='')
args = parser.parse_args()

if args.data is None:
    gdown.download(id="12DHHyCLSM1iYSsWoU_mmytUwDADvMcve", output="data.7z")
    os.system("7z x data.7z")
    pathlib.Path("data.7z").unlink()
    shutil.copytree("set_covering", "data", dirs_exist_ok=True)

if args.data is not None and "gecco2020" in args.data:
    gdown.download(id="1PvdFc16nv0_MEs6kEDnFsy0mHRwOJ03X", output="data.7z")
    os.system("7z x data.7z")
    pathlib.Path("data.7z").unlink()
    shutil.copytree("set_covering_gecco2020", "data", dirs_exist_ok=True)

if args.data is not None and "pace2025" in args.data:
    gdown.download(id="1TY9uWcP1kveGTZhR7X8SMulaEwOBI0fY", output="data.7z")
    os.system("7z x data.7z")
    pathlib.Path("data.7z").unlink()
    shutil.copytree("set_covering_pace2025", "data", dirs_exist_ok=True)

if args.data is not None and "pace2025_ds" in args.data:
    gdown.download(id="1cM0d4yRzJTjtokj4BdLmbhPqeBtckMLA", output="data.7z")
    os.system("7z x data.7z")
    pathlib.Path("data.7z").unlink()
    shutil.copytree("set_covering_pace2025_ds", "data", dirs_exist_ok=True)
