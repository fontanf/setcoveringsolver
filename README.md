# SetCoveringSolver

A solver for the (unicost) set covering problem.

## Implemented algorithms

- Greedy
  - Greedy `-a greedy`
  - based on the one from "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020) `-a greedy-lin`
  - Dual greedy `-a greedy-dual`

- Mixed-Integer Linear Program
  - Solved with Cbc `-a milp-cbc`
  - Solved with Gurobi `-a milp-gurobi`

- Row weighting local search (unicost only)
  - `-a "local-search-row-weighting-1"`
    - Neighborhood: toggle a set (remove and then add)
    - based on "An efficient local search heuristic with row weighting for the unicost set covering problem" (Gao et al., 2015)
  - `-a "local-search-row-weighting-2"`
    - Neighborhood: swap two sets
    - `-a "local-search-row-weighting-2 --wu 1"` implements the algorithm from "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020)
    - `-a "local-search-row-weighting-2 --n1 1 --n2 2 --wu 1"` implements the algorithm from "Weighting-based Variable Neighborhood Search for Optimal Camera Placement" (Su et al., 2021)

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
./bazel-bin/setcoveringsolver/main -v 1 -i data/wedelin1995/sasd9imp2.dat -f wedelin1995 -a milp-cbc
```
```
=====================================
          SetCoveringSolver          
=====================================

Instance
--------
Number of elements:                           1366
Number of sets:                               25032
Number of arcs:                               110881
Average number of sets covering an element:   81.172
Average number of elements covered by a set:  4.42957
Total cost:                                   311062130
Number of connected components:               9

Algorithm
---------
MILP (CBC)

Reduced instance
----------------
Number of elements:                           1360
Number of sets:                               25026
Number of arcs:                               110875
Average number of sets covering an element:   81.5257
Average number of elements covered by a set:  4.43039
Total cost:                                   311014130
Number of connected components:               3

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
       0.018         inf           0         inf         inf                        
       0.588         inf     5213088         inf         inf                        
       9.548     5264470     5213088       51382        0.99                        
      10.907     5263000     5213088       49912        0.96                        
      38.135     5262990     5213088       49902        0.96                        
      40.542     5262040     5213088       48952        0.94                        
      45.385     5262040     5214040       48000        0.92                        
      45.388     5262040     5262040           0        0.00                        

Final statistics
----------------
Value:                         5262040
Bound:                         5262040
Absolute optimality gap:       0
Relative optimality gap (%):   0
Time (s):                      45.3884

Solution
--------
Number of sets:                466 / 25032 (1.86162%)
Number of uncovered elements:  0 / 1366 (0%)
Feasible:                      1
Cost:                          5262040
```

```shell
./bazel-bin/setcoveringsolver/main -v 1 -i data/beasley1990/scpnrh5.txt -f orlibrary -a large-neighborhood-search -t 0.3
```
```
=====================================
          SetCoveringSolver          
=====================================

Instance
--------
Number of elements:                           1000
Number of sets:                               10000
Number of arcs:                               499179
Average number of sets covering an element:   499.179
Average number of elements covered by a set:  49.9179
Total cost:                                   504208
Number of connected components:               1

Algorithm
---------
Large neighborhood search

Parameters
----------
Maximum number of iterations:                      -1
Maximum number of iterations without improvement:  -1

Reduced instance
----------------
Number of elements:                           1000
Number of sets:                               10000
Number of arcs:                               499179
Average number of sets covering an element:   499.179
Average number of elements covered by a set:  49.9179
Total cost:                                   504208
Number of connected components:               1

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
       0.006         inf           0         inf         inf                        
       0.008          61           0          61         inf        initial solution
       0.009          59           0          59         inf             iteration 0
       0.010          58           0          58         inf             iteration 1
       0.039          57           0          57         inf            iteration 33
       0.050          56           0          56         inf            iteration 44
       0.255          55           0          55         inf           iteration 273

Final statistics
----------------
Value:                         55
Bound:                         0
Absolute optimality gap:       55
Relative optimality gap (%):   inf
Time (s):                      0.300841
Number of iterations:          322

Solution
--------
Number of sets:                52 / 10000 (0.52%)
Number of uncovered elements:  0 / 1000 (0%)
Feasible:                      1
Cost:                          55
```

```shell
./bazel-bin/setcoveringsolver/main -v 1 -i data/faster1994/rail582.txt -f faster --unicost -a local-search-row-weighting-1 -c solution.txt -t 2
```
```
=====================================
          SetCoveringSolver          
=====================================

Instance
--------
Number of elements:                           582
Number of sets:                               55515
Number of arcs:                               401708
Average number of sets covering an element:   690.22
Average number of elements covered by a set:  7.23603
Total cost:                                   55515
Number of connected components:               1

Algorithm
---------
Row weighting local search 1

Parameters
----------
Maximum number of iterations:                      -1
Maximum number of iterations without improvement:  -1

Reduced instance
----------------
Number of elements:                           566
Number of sets:                               55380
Number of arcs:                               399452
Average number of sets covering an element:   705.746
Average number of elements covered by a set:  7.21293
Total cost:                                   55380
Number of connected components:               1

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
       0.079         inf           0         inf         inf                        
       0.097         153           0         153         inf        initial solution
       0.097         152           0         152         inf            iteration 21
       0.098         151           0         151         inf            iteration 34
       0.098         150           0         150         inf            iteration 46
       0.103         149           0         149         inf           iteration 618
       0.103         148           0         148         inf           iteration 650
       0.106         147           0         147         inf          iteration 1096
       0.111         146           0         146         inf          iteration 1680
       0.131         145           0         145         inf          iteration 4001
       0.133         144           0         144         inf          iteration 4190
       0.133         143           0         143         inf          iteration 4215
       0.136         142           0         142         inf          iteration 4582
       0.144         141           0         141         inf          iteration 5468
       0.145         140           0         140         inf          iteration 5622
       0.163         139           0         139         inf          iteration 7599
       0.166         138           0         138         inf          iteration 7843
       0.192         137           0         137         inf         iteration 10881
       0.227         136           0         136         inf         iteration 14722
       0.257         135           0         135         inf         iteration 18092
       0.285         134           0         134         inf         iteration 20601
       0.292         133           0         133         inf         iteration 21075
       0.515         132           0         132         inf         iteration 41779
       0.633         131           0         131         inf         iteration 54229
       1.446         130           0         130         inf        iteration 132511

Final statistics
----------------
Value:                         130
Bound:                         0
Absolute optimality gap:       130
Relative optimality gap (%):   inf
Time (s):                      2.00001
Number of iterations:          181017

Solution
--------
Number of sets:                130 / 55515 (0.234171%)
Number of uncovered elements:  0 / 582 (0%)
Feasible:                      1
Cost:                          130
```

```shell
./bazel-bin/setcoveringsolver/main -v 1 -i data/gecco2020/AC_15_cover.txt -f gecco2020 --unicost -a local-search-row-weighting-2 -t 10 -c solution.txt
```
```
=====================================
          SetCoveringSolver          
=====================================

Instance
--------
Number of elements:                           18605
Number of sets:                               89304
Number of arcs:                               3972412
Average number of sets covering an element:   213.513
Average number of elements covered by a set:  44.4819
Total cost:                                   89304
Number of connected components:               1

Algorithm
---------
Row weighting local search 2

Parameters
----------
Neighborhood 1:                                    0
Neighborhood 2:                                    0
Weights update strategy:                           0
Maximum number of iterations:                      -1
Maximum number of iterations without improvement:  -1

Reduced instance
----------------
Number of elements:                           18605
Number of sets:                               72684
Number of arcs:                               3235992
Average number of sets covering an element:   173.931
Average number of elements covered by a set:  44.5214
Total cost:                                   72684
Number of connected components:               1

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
       0.696         inf           0         inf         inf                        
       0.782         660           0         660         inf        initial solution
       0.786         659           0         659         inf             it 1 comp 0
       0.790         658           0         658         inf             it 3 comp 0
       0.792         657           0         657         inf             it 5 comp 0
       0.793         656           0         656         inf             it 6 comp 0
       0.795         655           0         655         inf             it 7 comp 0
       0.796         654           0         654         inf             it 8 comp 0
       0.798         653           0         653         inf             it 9 comp 0
       0.801         652           0         652         inf            it 12 comp 0
       0.803         651           0         651         inf            it 14 comp 0
       0.804         650           0         650         inf            it 15 comp 0
       0.806         649           0         649         inf            it 16 comp 0
       0.807         648           0         648         inf            it 17 comp 0
       0.810         647           0         647         inf            it 20 comp 0
       0.812         646           0         646         inf            it 21 comp 0
       0.814         645           0         645         inf            it 24 comp 0
       0.816         644           0         644         inf            it 25 comp 0
       0.817         643           0         643         inf            it 26 comp 0
       0.820         642           0         642         inf            it 30 comp 0
       0.821         641           0         641         inf            it 31 comp 0
       0.822         640           0         640         inf            it 33 comp 0
       0.824         639           0         639         inf            it 34 comp 0
       0.827         638           0         638         inf            it 38 comp 0
       0.830         637           0         637         inf            it 40 comp 0
       0.832         636           0         636         inf            it 42 comp 0
       0.837         635           0         635         inf            it 53 comp 0
       0.841         634           0         634         inf            it 57 comp 0
       0.842         633           0         633         inf            it 58 comp 0
       0.844         632           0         632         inf            it 60 comp 0
       0.847         631           0         631         inf            it 63 comp 0
       0.856         630           0         630         inf            it 75 comp 0
       0.859         629           0         629         inf            it 79 comp 0
       0.860         628           0         628         inf            it 81 comp 0
       0.861         627           0         627         inf            it 82 comp 0
       0.864         626           0         626         inf            it 86 comp 0
       0.866         625           0         625         inf            it 90 comp 0
       0.875         624           0         624         inf           it 104 comp 0
       0.877         623           0         623         inf           it 106 comp 0
       0.881         622           0         622         inf           it 111 comp 0
       0.890         621           0         621         inf           it 124 comp 0
       0.893         620           0         620         inf           it 130 comp 0
       0.897         619           0         619         inf           it 135 comp 0
       0.900         618           0         618         inf           it 138 comp 0
       0.905         617           0         617         inf           it 146 comp 0
       0.913         616           0         616         inf           it 168 comp 0
       0.930         615           0         615         inf           it 201 comp 0
       0.937         614           0         614         inf           it 216 comp 0
       0.966         613           0         613         inf           it 264 comp 0
       0.974         612           0         612         inf           it 276 comp 0
       0.978         611           0         611         inf           it 284 comp 0
       0.984         610           0         610         inf           it 295 comp 0
       0.989         609           0         609         inf           it 303 comp 0
       0.996         608           0         608         inf           it 316 comp 0
       1.000         607           0         607         inf           it 322 comp 0
       1.009         606           0         606         inf           it 341 comp 0
       1.024         605           0         605         inf           it 365 comp 0
       1.033         604           0         604         inf           it 378 comp 0
       1.036         603           0         603         inf           it 382 comp 0
       1.066         602           0         602         inf           it 432 comp 0
       1.086         601           0         601         inf           it 465 comp 0
       1.090         600           0         600         inf           it 472 comp 0
       1.107         599           0         599         inf           it 499 comp 0
       1.145         598           0         598         inf           it 567 comp 0
       1.181         597           0         597         inf           it 626 comp 0
       1.185         596           0         596         inf           it 630 comp 0
       1.194         595           0         595         inf           it 643 comp 0
       1.209         594           0         594         inf           it 672 comp 0
       1.215         593           0         593         inf           it 684 comp 0
       1.225         592           0         592         inf           it 703 comp 0
       1.232         591           0         591         inf           it 715 comp 0
       1.238         590           0         590         inf           it 726 comp 0
       1.310         589           0         589         inf           it 845 comp 0
       1.436         588           0         588         inf          it 1066 comp 0
       1.568         587           0         587         inf          it 1296 comp 0
       1.621         586           0         586         inf          it 1378 comp 0
       1.639         585           0         585         inf          it 1405 comp 0
       1.749         584           0         584         inf          it 1601 comp 0
       1.753         583           0         583         inf          it 1606 comp 0
       2.040         582           0         582         inf          it 2141 comp 0
       2.139         581           0         581         inf          it 2303 comp 0
       2.194         580           0         580         inf          it 2402 comp 0
       2.199         579           0         579         inf          it 2408 comp 0
       2.256         578           0         578         inf          it 2497 comp 0
       4.340         577           0         577         inf          it 6058 comp 0
       4.373         576           0         576         inf          it 6104 comp 0
       5.248         575           0         575         inf          it 7750 comp 0
       6.996         574           0         574         inf         it 10939 comp 0
       7.030         573           0         573         inf         it 10991 comp 0
       8.197         572           0         572         inf         it 13081 comp 0
       8.354         571           0         571         inf         it 13384 comp 0
       8.358         570           0         570         inf         it 13388 comp 0
       8.465         569           0         569         inf         it 13569 comp 0
       8.796         568           0         568         inf         it 14113 comp 0
       8.807         567           0         567         inf         it 14131 comp 0

Final statistics
----------------
Value:                         567
Bound:                         0
Absolute optimality gap:       567
Relative optimality gap (%):   inf
Time (s):                      10.0001
Number of iterations:          16129
Neighborhood 1 improvements:   16129
Neighborhood 2 improvements:   0
Neighborhood 1 time:           8.85267
Neighborhood 2 time:           0
Number of weights reductions:  0

Solution
--------
Number of sets:                567 / 89304 (0.63491%)
Number of uncovered elements:  0 / 18605 (0%)
Feasible:                      1
Cost:                          567
```

Benchmarks:
```shell
python3 ../optimizationtools/scripts/bench_run.py --algorithms "greedy"
python3 ../optimizationtools/scripts/bench_run.py --algorithms "greedy_lin"
python3 ../optimizationtools/scripts/bench_run.py --algorithms "greedy_dual"
python3 ../optimizationtools/scripts/bench_process.py --benchmark heuristicshort --timelimit 10 --labels "greedy" "greedy_lin" "greedy_dual"
```

![heuristicshort](img/heuristicshort.png?raw=true "heuristicshort")

