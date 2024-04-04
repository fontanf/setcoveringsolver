import argparse
import sys
import os

parser = argparse.ArgumentParser(description='')
parser.add_argument('directory')
parser.add_argument(
        "-t", "--tests",
        type=str,
        nargs='*',
        help='')

args = parser.parse_args()


main = os.path.join(
        "bazel-bin",
        "setcoveringsolver",
        "setcovering",
        "main")


if args.tests is None or "greedy" in args.tests:
    print("Greedy")
    print("------")
    print()

    greedy_data = [
            (os.path.join("beasley1987", "scpa1.txt"), "orlibrary"),
            (os.path.join("beasley1987", "scpb2.txt"), "orlibrary"),
            (os.path.join("beasley1987", "scpc3.txt"), "orlibrary"),
            (os.path.join("beasley1987", "scpd4.txt"), "orlibrary"),
            (os.path.join("beasley1987", "scpe5.txt"), "orlibrary") ]

    for instance, instance_format in greedy_data:
        instance_path = os.path.join(
                "data",
                instance)
        json_output_path = os.path.join(
                args.directory,
                "greedy",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + "  --format \"" + instance_format + "\""
                + "  --algorithm greedy"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()
