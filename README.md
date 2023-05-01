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
./bazel-bin/setcoveringsolver/main -v 1 -i data/beasley1990/scpnrh5.txt -f orlibrary -a largeneighborhoodsearch -t 0.3
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
Total cost:                                   504208
Number of connected components:               1

Algorithm
---------
Large Neighborhood Search

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
       0.009          61           0          61         inf        initial solution
       0.010          59           0          59         inf             iteration 0
       0.011          58           0          58         inf             iteration 1
       0.042          57           0          57         inf            iteration 33
       0.053          56           0          56         inf            iteration 44
       0.268          55           0          55         inf           iteration 273

Final statistics
----------------
Value:                         55
Bound:                         0
Absolute optimality gap:       55
Relative optimality gap (%):   inf
Time (s):                      0.300139
Number of iterations:          306

Solution
--------
Number of sets:                52 / 10000 (0.52%)
Number of uncovered elements:  0 / 1000 (0%)
Feasible:                      1
Cost:                          55
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
Total cost:                                   55515
Number of connected components:               1

Algorithm
---------
Row Weighting Local Search 1

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
       0.089         inf           0         inf         inf                        
       0.108         153           0         153         inf        initial solution
       0.109         152           0         152         inf            iteration 21
       0.116         151           0         151         inf            iteration 34
       0.116         150           0         150         inf            iteration 46
       0.121         149           0         149         inf           iteration 618
       0.121         148           0         148         inf           iteration 650
       0.124         147           0         147         inf          iteration 1096
       0.129         146           0         146         inf          iteration 1680
       0.150         145           0         145         inf          iteration 4001
       0.152         144           0         144         inf          iteration 4190
       0.153         143           0         143         inf          iteration 4215
       0.156         142           0         142         inf          iteration 4582
       0.164         141           0         141         inf          iteration 5468
       0.165         140           0         140         inf          iteration 5622
       0.184         139           0         139         inf          iteration 7599
       0.187         138           0         138         inf          iteration 7843
       0.215         137           0         137         inf         iteration 10881
       0.252         136           0         136         inf         iteration 14722
       0.283         135           0         135         inf         iteration 18092
       0.312         134           0         134         inf         iteration 20601
       0.319         133           0         133         inf         iteration 21075
       0.552         132           0         132         inf         iteration 41779
       0.676         131           0         131         inf         iteration 54229
       1.528         130           0         130         inf        iteration 132511

Final statistics
----------------
Value:                         130
Bound:                         0
Absolute optimality gap:       130
Relative optimality gap (%):   inf
Time (s):                      2.00001
Number of iterations:          171735

Solution
--------
Number of sets:                130 / 55515 (0.234171%)
Number of uncovered elements:  0 / 582 (0%)
Feasible:                      1
Cost:                          130
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
Total cost:                                   89304
Number of connected components:               1

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

Reduced instance
----------------
Number of elements:                           18605
Number of sets:                               72684
Number of arcs:                               3235992
Average number of sets covering an element:   173.931
Average number of elements covered by a set:  44.5214
Total cost:                                   72684
Number of connected components:               1

Compute set neighbors...

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
       0.643         inf           0         inf         inf                        
       0.730         660           0         660         inf        initial solution
       0.734         659           0         659         inf             it 1 comp 0
       0.741         658           0         658         inf             it 3 comp 0
       0.746         657           0         657         inf             it 5 comp 0
       0.749         656           0         656         inf             it 6 comp 0
       0.753         655           0         655         inf             it 7 comp 0
       0.756         654           0         654         inf             it 8 comp 0
       0.759         653           0         653         inf             it 9 comp 0
       0.764         652           0         652         inf            it 12 comp 0
       0.768         651           0         651         inf            it 14 comp 0
       0.771         650           0         650         inf            it 15 comp 0
       0.773         649           0         649         inf            it 16 comp 0
       0.775         648           0         648         inf            it 17 comp 0
       0.779         647           0         647         inf            it 20 comp 0
       0.780         646           0         646         inf            it 21 comp 0
       0.783         645           0         645         inf            it 24 comp 0
       0.785         644           0         644         inf            it 25 comp 0
       0.787         643           0         643         inf            it 26 comp 0
       0.790         642           0         642         inf            it 30 comp 0
       0.791         641           0         641         inf            it 31 comp 0
       0.792         640           0         640         inf            it 33 comp 0
       0.794         639           0         639         inf            it 34 comp 0
       0.797         638           0         638         inf            it 38 comp 0
       0.800         637           0         637         inf            it 40 comp 0
       0.802         636           0         636         inf            it 42 comp 0
       0.807         635           0         635         inf            it 53 comp 0
       0.811         634           0         634         inf            it 57 comp 0
       0.813         633           0         633         inf            it 58 comp 0
       0.815         632           0         632         inf            it 60 comp 0
       0.818         631           0         631         inf            it 63 comp 0
       0.827         630           0         630         inf            it 75 comp 0
       0.829         629           0         629         inf            it 79 comp 0
       0.831         628           0         628         inf            it 81 comp 0
       0.832         627           0         627         inf            it 82 comp 0
       0.835         626           0         626         inf            it 86 comp 0
       0.837         625           0         625         inf            it 90 comp 0
       0.846         624           0         624         inf           it 104 comp 0
       0.848         623           0         623         inf           it 106 comp 0
       0.852         622           0         622         inf           it 111 comp 0
       0.861         621           0         621         inf           it 124 comp 0
       0.865         620           0         620         inf           it 130 comp 0
       0.868         619           0         619         inf           it 135 comp 0
       0.871         618           0         618         inf           it 138 comp 0
       0.876         617           0         617         inf           it 146 comp 0
       0.884         616           0         616         inf           it 168 comp 0
       0.901         615           0         615         inf           it 201 comp 0
       0.908         614           0         614         inf           it 216 comp 0
       0.937         613           0         613         inf           it 264 comp 0
       0.945         612           0         612         inf           it 276 comp 0
       0.949         611           0         611         inf           it 284 comp 0
       0.955         610           0         610         inf           it 295 comp 0
       0.961         609           0         609         inf           it 303 comp 0
       0.968         608           0         608         inf           it 316 comp 0
       0.972         607           0         607         inf           it 322 comp 0
       0.981         606           0         606         inf           it 341 comp 0
       0.996         605           0         605         inf           it 365 comp 0
       1.005         604           0         604         inf           it 378 comp 0
       1.008         603           0         603         inf           it 382 comp 0
       1.038         602           0         602         inf           it 432 comp 0
       1.058         601           0         601         inf           it 465 comp 0
       1.062         600           0         600         inf           it 472 comp 0
       1.079         599           0         599         inf           it 499 comp 0
       1.117         598           0         598         inf           it 567 comp 0
       1.153         597           0         597         inf           it 626 comp 0
       1.157         596           0         596         inf           it 630 comp 0
       1.166         595           0         595         inf           it 643 comp 0
       1.181         594           0         594         inf           it 672 comp 0
       1.188         593           0         593         inf           it 684 comp 0
       1.198         592           0         592         inf           it 703 comp 0
       1.204         591           0         591         inf           it 715 comp 0
       1.211         590           0         590         inf           it 726 comp 0
       1.283         589           0         589         inf           it 845 comp 0
       1.409         588           0         588         inf          it 1066 comp 0
       1.543         587           0         587         inf          it 1296 comp 0
       1.596         586           0         586         inf          it 1378 comp 0
       1.614         585           0         585         inf          it 1405 comp 0
       1.724         584           0         584         inf          it 1601 comp 0
       1.728         583           0         583         inf          it 1606 comp 0
       2.012         582           0         582         inf          it 2141 comp 0
       2.109         581           0         581         inf          it 2303 comp 0
       2.165         580           0         580         inf          it 2402 comp 0
       2.170         579           0         579         inf          it 2408 comp 0
       2.227         578           0         578         inf          it 2497 comp 0
       4.317         577           0         577         inf          it 6058 comp 0
       4.351         576           0         576         inf          it 6104 comp 0
       5.238         575           0         575         inf          it 7750 comp 0
       7.001         574           0         574         inf         it 10939 comp 0
       7.036         573           0         573         inf         it 10991 comp 0
       8.225         572           0         572         inf         it 13081 comp 0
       8.375         571           0         571         inf         it 13384 comp 0
       8.379         570           0         570         inf         it 13388 comp 0
       8.487         569           0         569         inf         it 13569 comp 0
       8.816         568           0         568         inf         it 14113 comp 0
       8.827         567           0         567         inf         it 14131 comp 0

Final statistics
----------------
Value:                         567
Bound:                         0
Absolute optimality gap:       567
Relative optimality gap (%):   inf
Time (s):                      10.0001
Number of iterations:           16095
Neighborhood 1 improvements:    16095
Neighborhood 2 improvements:    0
Neighborhood 1 time:            8.87257
Neighborhood 2 time:            0
Number of weights reductions:   0

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

