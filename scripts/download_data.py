import argparse
import gdown
import pathlib
import py7zr


parser = argparse.ArgumentParser(description='')
parser.add_argument(
        "-d", "--data",
        type=str,
        nargs='*',
        help='')
args = parser.parse_args()

if args.data is None:
    gdown.download(id="1TbuXAu54egtB2M9XAsmOZzhJpJ5JoN3I", output="data.7z")
    with py7zr.SevenZipFile("data.7z", mode="r") as z:
        z.extractall(path="data")
    pathlib.Path("data.7z").unlink()

if args.data is not None and "gecco2020" in args.data:
    gdown.download(id="12SnK3M4va1RxxcUIZ8Lh11yQNpHB14JC", output="data.7z")
    with py7zr.SevenZipFile("data.7z", mode="r") as z:
        z.extractall(path="data")
    pathlib.Path("data.7z").unlink()

if args.data is not None and "pace2025" in args.data:
    gdown.download(id="1XB_9uHEfBcPQ9lOWiVHiHBixHbMX9gxW", output="data.7z")
    with py7zr.SevenZipFile("data.7z", mode="r") as z:
        z.extractall(path="data")
    pathlib.Path("data.7z").unlink()

if args.data is not None and "pace2025_ds" in args.data:
    gdown.download(id="1V_m_PoFdgwMJRLkj2VwKsVraS30vYk0p", output="data.7z")
    with py7zr.SevenZipFile("data.7z", mode="r") as z:
        z.extractall(path="data")
    pathlib.Path("data.7z").unlink()
