# Set Covering Solver

A solver for the (Unicost) Set Covering Problem.

## Implemented algorithms

- Greedy
  - Greedy `-a greedy`
  - based on the one from "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020) `-a greedy_lin`
  - Dual greedy `-a greedy_dual`

- Mixed-Integer Linear Program
  - Solved with Cbc `-a milp_cbc`
  - Solved with Gurobi `-a milp_gurobi`

- Row weighting local search (unicost only)
  - `-a "localsearch_rowweighting_1"`
    - Neighborhood: toggle a set (remove and then add)
    - based on "An efficient local search heuristic with row weighting for the unicost set covering problem" (Gao et al., 2015)
  - `-a "localsearch_rowweighting_2"`
    - Neighborhood: swap two sets
    - `-a "localsearch_rowweighting_2 --wu 1"` implements the algorithm from "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020)
    - `-a "localsearch_rowweighting_2 --n1 1 --n2 2 --wu 1"` implements the algorithm from "Weighting-based Variable Neighborhood Search for Optimal Camera Placement" (Su et al., 2021)

- Large neighborhood search `-a "largeneighborhoodsearch --iterations 100000 --iterations-without-improvment 10000"`

## Usage (command line)

Download and uncompress the instances in the `data/` folder:

https://drive.google.com/file/d/1lhbfd_ayrIUEUgGJ33YH-9_QrCjwGO6Y/view?usp=sharing

Compile:
```shell
bazel build -- //...

# To use algorithm "milp_cbc":
bazel build --define coinor=true -- //...

# To use algorithm "milp_gurobi":
bazel build --define gurobi=true -- //...
```

Run:

```shell
./bazel-bin/setcoveringsolver/main -v 1 -i data/wedelin1995/sasd9imp2.dat -f wedelin1995 -a milp_cbc
```
```
=====================================
         Set Covering Solver         
=====================================

Instance
--------
Number of elements:                           1366
Number of sets:                               25032
Number of arcs:                               110881
Average number of sets covering an element:   81.172
Average number of elements covered by a set:  4.42957
Number of connected components:               9
Average cost:                                 12426.6

Algorithm
---------
MILP (CBC)

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
       0.000   311062130           0   311062130         inf                        
       0.673   311062130     5261088   305801042     5812.51                        
      14.626     5263570     5261088        2482   0.0471766                        
      15.029     5262040     5261088         952   0.0180951                        
      20.842     5262040     5261498         542   0.0103012                        
      21.794     5262040     5262040           0           0                        

Final statistics
----------------
Value:                         5262040
Bound:                         5262040
Gap:                           0
Gap (%):                       0
Time (s):                      21.7971
```

```shell
./bazel-bin/setcoveringsolver/main -v 1 -i data/beasley1990/scpnrh5.txt -f orlibrary -a largeneighborhoodsearch -t 0.2
```
```
=====================================
         Set Covering Solver         
=====================================

Instance
--------
Number of elements:                           1000
Number of sets:                               10000
Number of arcs:                               499179
Average number of sets covering an element:   499.179
Average number of elements covered by a set:  49.9179
Number of connected components:               1
Average cost:                                 50.4208

Algorithm
---------
Large Neighborhood Search

Parameters
----------
Maximum number of iterations:                      -1
Maximum number of iterations without improvement:  -1

Reduction
---------
Number of unfixed elements:  1000/1000 (0 fixed)
Number of mandatory sets:    0/10000
Number of unfixed sets:      10000/10000 (0 fixed)

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
       0.002      504208           0      504208         inf                        
       0.005          61           0          61         inf        initial solution
       0.006          59           0          59         inf             iteration 0
       0.007          58           0          58         inf             iteration 1
       0.025          57           0          57         inf            iteration 19
       0.030          56           0          56         inf            iteration 24
       0.120          55           0          55         inf           iteration 111

Final statistics
----------------
Value:                         55
Bound:                         0
Gap:                           55
Gap (%):                       inf
Time (s):                      0.200135
Iterations:                    188
```

```shell
./bazel-bin/setcoveringsolver/main -v 1 -i data/faster1994/rail582.txt -f faster --unicost -a localsearch_rowweighting_1 -c solution.txt -t 2
```
```
=====================================
         Set Covering Solver         
=====================================

Instance
--------
Number of elements:                           582
Number of sets:                               55515
Number of arcs:                               401708
Average number of sets covering an element:   690.22
Average number of elements covered by a set:  7.23603
Number of connected components:               1
Average cost:                                 1

Algorithm
---------
Row Weighting Local Search 1

Reduction
---------
Number of unfixed elements:  579/582 (3 fixed)
Number of unfixed sets:      55388/55515 (127 fixed)

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
      0.0147       55515           0       55515         inf                        
      0.0358         153           0         153         inf        initial solution
      0.0366         152           0         152         inf            iteration 31
      0.0427         151           0         151         inf           iteration 846
      0.0454         150           0         150         inf          iteration 1171
       0.047         149           0         149         inf          iteration 1333
      0.0473         148           0         148         inf          iteration 1347
      0.0525         147           0         147         inf          iteration 2086
       0.053         146           0         146         inf          iteration 2134
      0.0538         145           0         145         inf          iteration 2204
      0.0603         144           0         144         inf          iteration 3090
      0.0635         143           0         143         inf          iteration 3443
      0.0654         142           0         142         inf          iteration 3640
      0.0757         141           0         141         inf          iteration 4834
      0.0784         140           0         140         inf          iteration 5131
      0.0846         139           0         139         inf          iteration 5943
      0.0922         138           0         138         inf          iteration 6728
      0.1088         137           0         137         inf          iteration 8522
      0.1236         136           0         136         inf         iteration 10269
      0.1252         135           0         135         inf         iteration 10370
      0.2399         134           0         134         inf         iteration 21847
      0.4493         133           0         133         inf         iteration 43380
      0.4564         132           0         132         inf         iteration 43977
      0.7914         131           0         131         inf         iteration 77801
        1.49         130           0         130         inf        iteration 144999
      1.5386         129           0         129         inf        iteration 151057

Final statistics
----------------
Value:                 129
Bound:                 0
Gap:                   129
Gap (%):               inf
Time (s):              2
Number of iterations:  199431
```

```shell
./bazel-bin/setcoveringsolver/main -v 1 -i data/gecco2020/AC_15_cover.txt -f gecco2020 --unicost -a localsearch_rowweighting_2 -t 10 -c solution.txt
```
```
=====================================
         Set Covering Solver         
=====================================

Instance
--------
Number of elements:                           18605
Number of sets:                               89304
Number of arcs:                               3972412
Average number of sets covering an element:   213.513
Average number of elements covered by a set:  44.4819
Number of connected components:               1
Average cost:                                 1

Algorithm
---------
Row Weighting Local Search 2

Parameters
----------
Neighborhood 1:                                    0
Neighborhood 2:                                    0
Weights update strategy:                           0
Maximum number of iterations:                      -1
Maximum number of iterations without improvement:  -1

Reduction
---------
Number of unfixed elements:  18605/18605 (0 fixed)
Number of mandatory sets:    0/89304
Number of unfixed sets:      72684/89304 (16620 fixed)

Compute set neighbors...

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
       0.495       89304           0       89304         inf                        
       0.587         641           0         641         inf        initial solution
       0.592         640           0         640         inf             it 2 comp 0
       0.599         639           0         639         inf             it 7 comp 0
       0.600         638           0         638         inf             it 8 comp 0
       0.601         637           0         637         inf             it 9 comp 0
       0.602         636           0         636         inf            it 10 comp 0
       0.604         635           0         635         inf            it 12 comp 0
       0.609         634           0         634         inf            it 17 comp 0
       0.610         633           0         633         inf            it 18 comp 0
       0.611         632           0         632         inf            it 19 comp 0
       0.612         631           0         631         inf            it 20 comp 0
       0.613         630           0         630         inf            it 21 comp 0
       0.614         629           0         629         inf            it 22 comp 0
       0.617         628           0         628         inf            it 28 comp 0
       0.620         627           0         627         inf            it 31 comp 0
       0.630         626           0         626         inf            it 46 comp 0
       0.632         625           0         625         inf            it 51 comp 0
       0.634         624           0         624         inf            it 54 comp 0
       0.636         623           0         623         inf            it 57 comp 0
       0.640         622           0         622         inf            it 61 comp 0
       0.645         621           0         621         inf            it 69 comp 0
       0.648         620           0         620         inf            it 72 comp 0
       0.652         619           0         619         inf            it 77 comp 0
       0.656         618           0         618         inf            it 83 comp 0
       0.689         617           0         617         inf           it 125 comp 0
       0.697         616           0         616         inf           it 134 comp 0
       0.703         615           0         615         inf           it 140 comp 0
       0.703         614           0         614         inf           it 141 comp 0
       0.708         613           0         613         inf           it 147 comp 0
       0.732         612           0         612         inf           it 174 comp 0
       0.741         611           0         611         inf           it 187 comp 0
       0.743         610           0         610         inf           it 189 comp 0
       0.752         609           0         609         inf           it 199 comp 0
       0.757         608           0         608         inf           it 205 comp 0
       0.770         607           0         607         inf           it 235 comp 0
       0.772         606           0         606         inf           it 237 comp 0
       0.806         605           0         605         inf           it 278 comp 0
       0.813         604           0         604         inf           it 286 comp 0
       0.824         603           0         603         inf           it 301 comp 0
       0.842         602           0         602         inf           it 333 comp 0
       0.857         601           0         601         inf           it 354 comp 0
       0.860         600           0         600         inf           it 359 comp 0
       0.863         599           0         599         inf           it 367 comp 0
       0.908         598           0         598         inf           it 426 comp 0
       0.949         597           0         597         inf           it 483 comp 0
       1.011         596           0         596         inf           it 557 comp 0
       1.015         595           0         595         inf           it 563 comp 0
       1.023         594           0         594         inf           it 574 comp 0
       1.048         593           0         593         inf           it 605 comp 0
       1.056         592           0         592         inf           it 617 comp 0
       1.082         591           0         591         inf           it 648 comp 0
       1.102         590           0         590         inf           it 676 comp 0
       1.110         589           0         589         inf           it 685 comp 0
       1.279         588           0         588         inf           it 905 comp 0
       1.447         587           0         587         inf          it 1118 comp 0
       1.557         586           0         586         inf          it 1249 comp 0
       1.578         585           0         585         inf          it 1279 comp 0
       1.606         584           0         584         inf          it 1319 comp 0
       1.630         583           0         583         inf          it 1351 comp 0
       1.658         582           0         582         inf          it 1384 comp 0
       3.094         581           0         581         inf          it 3387 comp 0
       3.106         580           0         580         inf          it 3403 comp 0
       3.123         579           0         579         inf          it 3439 comp 0
       4.520         578           0         578         inf          it 5099 comp 0
       5.606         577           0         577         inf          it 6471 comp 0
       6.487         576           0         576         inf          it 7629 comp 0
       6.645         575           0         575         inf          it 7807 comp 0
       7.148         574           0         574         inf          it 8475 comp 0
       7.501         573           0         573         inf          it 8901 comp 0
       8.326         572           0         572         inf          it 9892 comp 0
       8.336         571           0         571         inf          it 9904 comp 0
       8.801         570           0         570         inf         it 10505 comp 0
       9.209         569           0         569         inf         it 11137 comp 0
       9.460         568           0         568         inf         it 11436 comp 0
       9.946         567           0         567         inf         it 12044 comp 0

Final statistics
----------------
Value:                         567
Bound:                         0
Gap:                           567
Gap (%):                       inf
Time (s):                      10.0001
Number of iterations:          12121
Neighborhood 1 improvements:   12121
Neighborhood 2 improvements:   0
Neighborhood 1 time:           9.14862
Neighborhood 2 time:           0
Number of weights reductions:  0
```

Benchmarks:
```shell
python3 ../optimizationtools/scripts/bench_run.py --algorithms "greedy"
python3 ../optimizationtools/scripts/bench_run.py --algorithms "greedy_lin"
python3 ../optimizationtools/scripts/bench_run.py --algorithms "greedy_dual"
python3 ../optimizationtools/scripts/bench_process.py --benchmark heuristicshort --timelimit 10 --labels "greedy" "greedy_lin" "greedy_dual"
```

![heuristicshort](img/heuristicshort.png?raw=true "heuristicshort")

