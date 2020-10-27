# Set Covering Solver

A solver for the (Unicost) Set Covering Problem.

## Implemented algorithms

* Greedy
  * Greedy `-a greedy`
  * based on the one from "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020) `-a greedy_lin`
  * Dual greedy `-a greedy_dual`
* Branch-and-cut (Gurobi) `-a branchandcut_gurobi`
* Row weighting local search (unicost only)
  * based on "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020) `-a "localsearch --threads 4"`
  * based on "An efficient local search heuristic with row weighting for the unicost set covering problem" (Gao et al., 2015) `-a "localsearch_2 --threads 4"`
* Large neighborhood search: fast, simple and minimum memory requirement, but lower quality solutions `-a largeneighborhoodsearch --iteration-limit 100000 --iteration-without-improvment-limit 10000`

## Usage (command line)

Download and uncompress the instances in the `data/` folder:

https://drive.google.com/file/d/1lhbfd_ayrIUEUgGJ33YH-9_QrCjwGO6Y/view?usp=sharing

Compile:
```shell
bazel build -- //...
```

Run:
```shell
./bazel-bin/setcoveringsolver/main -v -i data/faster1994/rail507.txt -f faster --unicost -a localsearch -t 60 -c solution.txt
```

Benchmarks:
```shell
bazel build -- //...
python3 setcoveringsolver/bench.py balas1980_unicost greedy
python3 setcoveringsolver/bench.py balas1980_unicost localsearch 60
```

