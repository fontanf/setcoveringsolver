# Set Covering Solver

A solver for the (Unicost) Set Covering Problem.

## Implemented algorithms

- Greedy
  - Greedy `-a greedy`
  - based on the one from "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020) `-a greedy_lin`
  - Dual greedy `-a greedy_dual`

- Mixed-Integer Linear Program (Gurobi) `-a milp_gurobi`

- Row weighting local search (unicost only)
  - based on "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020) `-a "localsearch_rowweighting"`
  - based on "An efficient local search heuristic with row weighting for the unicost set covering problem" (Gao et al., 2015) `-a "localsearch_rowweighting_2"`

- Large neighborhood search based on "Note: A local‐search heuristic for large set‐covering problems" (Jacobs et Brusco, 1995). A move consists in removing some sets and then filing the solution until it becomes feasible again.
  - Sets are removed to uncover a randomly selected element. This implementation is fast, simple and robust, but returns lower quality solutions `-a "largeneighborhoodsearch --iteration-limit 100000 --iteration-without-improvment-limit 10000"`
  - Sets are removed and added following a scoring scheme similar to the one from "NuMWVC: A novel local search for minimum weighted vertex cover problem" (Li et al., 2020). This implementation has decent performances on literature benchmarks except on the  `gecco2020` benchmark. `-a "largeneighborhoodsearch_2 --iteration-limit 100000 --iteration-without-improvment-limit 10000"`

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
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy_lin"
python3 ../optimizationtools/optimizationtools/bench_run.py --algorithms "greedy_dual"
python3 ../optimizationtools/optimizationtools/bench_process.py --benchmark heuristicshort --timelimit 10 --labels "greedy" "greedy_lin" "greedy_dual"
```

![heuristicshort](img/heuristicshort.png?raw=true "heuristicshort")

