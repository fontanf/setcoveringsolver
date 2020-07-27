import os
import json
import sys

data       = sys.argv[1]
algorithm  = sys.argv[2]
timelimit  = sys.argv[3] if len(sys.argv) > 3 else None

datas = {}
datas["gecco2020_AC"] = [("gecco2020/AC_" + "{:02d}".format(i) + "_cover.txt", "-f gecco2020") for i in range(1, 33)]
datas["gecco2020_RW"] = [("gecco2020/RW_" + "{:02d}".format(i) + "_cover.txt", "-f gecco2020") for i in range(1, 38)]
datas["fulkerson1974"] = [("fulkerson1974/data." + str(i), "-f sts") for i in [9, 15, 27, 45, 81, 135, 243, 405, 729, 1215, 2187]]
datas["balas1980"] = [("balas1980/scp" + str(i) + ".txt", "-f orlibrary") for i in [41, 42, 43, 44, 45, 46, 47, 48, 49, 410, 51, 52, 53, 54, 55, 56, 57, 58, 59, 510, 61, 62, 63, 64, 65]]
datas["balas1996"] = [("balas1996/" + s + ".inp", "-f balas1996") for s in ["bus1", "bus2", "aa03", "aa04", "aa05", "aa06", "aa11", "aa12", "aa13", "aa14", "aa15", "aa16", "aa17", "aa18", "aa19", "aa20"]]
datas["beasley1987"] = [("beasley1987/scp" + a + str(i) + ".txt", "-f orlibrary") for a in ["a", "b", "c", "d", "e"] for i in range(1, 6)]
datas["beasley1990"] = [("beasley1990/scpnr" + a + str(i) + ".txt", "-f orlibrary") for a in ["e", "f", "g", "h"] for i in range(1, 6)]
datas["faster1994"] = [("faster1994/rail" + str(i) + ".txt", "-f faster1994") for i in [507, 516, 582, 2536, 2586, 4284, 4872]]
datas["wedelin1995"] = [("wedelin1995/" + s + ".dat", "-f wedelin1995") for s in ["a320", "a320_coc", "alitalia", "b727", "sasd9imp2", "sasjump"]]
datas["grossman1997"] = [("grossman1997/scp" + s + ".txt", "-f orlibrary") for s in ["cyc06", "cyc07", "cyc08", "cyc09", "cyc10", "cyc11", "clr10", "clr11", "clr12", "clr13"]]
datas["balas1980_unicost"] = [("balas1980/scp" + str(i) + ".txt", "-f orlibrary --unicost") for i in [41, 42, 43, 44, 45, 46, 47, 48, 49, 410, 51, 52, 53, 54, 55, 56, 57, 58, 59, 510, 61, 62, 63, 64, 65]]
datas["balas1996_unicost"] = [("balas1996/" + s + ".inp", "-f balas1996 --unicost") for s in ["bus1", "bus2", "aa03", "aa04", "aa05", "aa06", "aa11", "aa12", "aa13", "aa14", "aa15", "aa16", "aa17", "aa18", "aa19", "aa20"]]
datas["beasley1987_unicost"] = [("beasley1987/scp" + a + str(i) + ".txt", "-f orlibrary --unicost") for a in ["a", "b", "c", "d", "e"] for i in range(1, 6)]
datas["beasley1990_unicost"] = [("beasley1990/scpnr" + a + str(i) + ".txt", "-f orlibrary --unicost") for a in ["e", "f", "g", "h"] for i in range(1, 6)]
datas["faster1994_unicost"] = [("faster1994/rail" + str(i) + ".txt", "-f faster1994 --unicost") for i in [507, 516, 582, 2536, 2586, 4284, 4872]]
datas["wedelin1995_unicost"] = [("wedelin1995/" + s + ".dat", "-f wedelin1995 --unicost") for s in ["a320", "a320_coc", "alitalia", "b727", "sasd9imp2", "sasjump"]]

if __name__ == "__main__":

    directory_in = "data"
    directory_out = os.path.join("output", algorithm + (" | " + str(timelimit) if timelimit != None else ""), data)
    if not os.path.exists(directory_out):
        os.makedirs(directory_out)

    results_file = open(os.path.join(directory_out, "results.csv"), "w")
    results_file.write("Instance,Value,,Time to best,Time to end\n")

    main_exec = os.path.join(".", "bazel-bin", "setcoveringsolver", "main")
    for instance_name, args in datas[data]:
        s = "_unicost" if "--unicost" in args else ""
        instance_path = os.path.join(directory_in, instance_name)
        output_path   = os.path.join(directory_out, instance_name + s + ".json")
        cert_path     = os.path.join(directory_out, instance_name + s + "_solution.txt")
        if not os.path.exists(os.path.dirname(output_path)):
            os.makedirs(os.path.dirname(output_path))

        command = main_exec \
                + " -v" \
                + " -i \"" + instance_path + "\"" \
                + " " + args \
                + (" -t " + str(timelimit) if timelimit != None else "") \
                + " -a \"" + algorithm + "\"" \
                + " -c \"" + cert_path + "\"" \
                + " -o \"" + output_path + "\""
        print(command)
        os.system(command)
        print()

        output_file = open(output_path, "r")
        d = json.load(output_file)

        k = 0
        while "Solution" + str(k + 1) in d.keys():
            k += 1

        results_file.write(instance_name \
                + "," + str(d["Solution"]["Value"]) \
                + "," \
                + "," + str(d["Solution" + str(k)]["Time"]) \
                + "," + str(d["Solution"]["Time"]) \
                + "\n")

