# SetCoveringSolver

A solver for the (unicost) set covering and set packing problems.

In the set packing problem solved, elements may be covered multiple times and there might be multiple copies of a set.

## Implemented algorithms

### Set covering

- Greedy
  - Greedy `-a greedy`
  - based on the one from "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020) `-a greedy-lin`
  - Dual greedy `-a greedy-dual`

- Mixed-integer linear program
  - Solved with Cbc `-a milp-cbc`
  - Solved with Gurobi `-a milp-gurobi`

- Row weighting local search (unicost only)
  - `-a local-search-row-weighting-1`
    - Neighborhood: toggle a set (remove and then add)
    - based on "An efficient local search heuristic with row weighting for the unicost set covering problem" (Gao et al., 2015)
  - `-a local-search-row-weighting-2`
    - Neighborhood: swap two sets
    - `-a local-search-row-weighting-2 --wu 1` implements the algorithm from "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020)
    - `-a local-search-row-weighting-2 --n1 1 --n2 2 --wu 1` implements the algorithm from "Weighting-based Variable Neighborhood Search for Optimal Camera Placement" (Su et al., 2021)

- Large neighborhood search `-a large-neighborhood-search --maximum-number-of-iterations 100000 --maximum-number-of-iterations-without-improvement 10000`

## Usage (command line)

Compile:
```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
cmake --install build --config Release --prefix install
```

Download data:
```shell
python3 scripts/download_data.py
python3 scripts/download_data.py --data gecco2020
```

Run:

```shell
./install/bin/setcoveringsolver -v 1 -i data/wedelin1995/sasd9imp2.dat -f wedelin1995 -a milp-cbc
```
```
=====================================
          SetCoveringSolver          
=====================================

Problem
-------
Set covering problem

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

Parameters
----------
Time limit:            inf
Messages
    Verbosity level:   1
    Standard output:   1
    File path:         
    # streams:         0
Logger
    Has logger:        0
    Standard error:    0
    File path:         
Reduction
    Enable:            1
    Max. # of rounds:  10
    Remove dominated:  0

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
       0.000         inf           0         inf         inf                        
       0.653         inf     5213088         inf         inf                        
      10.433     5264470     5213088       51382        0.99                        
      11.832     5263000     5213088       49912        0.96                        
      40.901     5262990     5213088       49902        0.96                        
      43.524     5262040     5213088       48952        0.94                        
      48.579     5262040     5214040       48000        0.92                        
      48.582     5262040     5262040           0        0.00                        

Final statistics
----------------
Value:                        5262040
Bound:                        5262040
Absolute optimality gap:      0
Relative optimality gap (%):  0
Time (s):                     48.582

Solution
--------
Number of sets:                466 / 25032 (1.86162%)
Number of uncovered elements:  0 / 1366 (0%)
Feasible:                      1
Cost:                          5262040
```

```shell
./install/bin/setcoveringsolver -v 1 -i data/beasley1990/scpnrh5.txt -f orlibrary -a large-neighborhood-search -t 0.3
```
```
=====================================
          SetCoveringSolver          
=====================================

Problem
-------
Set covering problem

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
Time limit:                          0.3
Messages
    Verbosity level:                 1
    Standard output:                 1
    File path:                       
    # streams:                       0
Logger
    Has logger:                      0
    Standard error:                  0
    File path:                       
Reduction
    Enable:                          1
    Max. # of rounds:                10
    Remove dominated:                0
Max. # of iterations:                -1
Max. # of iterations without impr.:  -1

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
       0.000         inf           0         inf         inf                        
       0.008          61           0          61         inf        initial solution
       0.009          59           0          59         inf             iteration 0
       0.010          58           0          58         inf             iteration 1
       0.039          57           0          57         inf            iteration 33
       0.050          56           0          56         inf            iteration 44
       0.254          55           0          55         inf           iteration 273

Final statistics
----------------
Value:                        55
Bound:                        0
Absolute optimality gap:      55
Relative optimality gap (%):  inf
Time (s):                     0.300108
Number of iterations:         323

Solution
--------
Number of sets:                52 / 10000 (0.52%)
Number of uncovered elements:  0 / 1000 (0%)
Feasible:                      1
Cost:                          55
```

```shell
./install/bin/setcoveringsolver -v 1 -i data/faster1994/rail582.txt -f faster --unicost -a local-search-row-weighting-1 -c solution.txt -t 2
```
```
=====================================
          SetCoveringSolver          
=====================================

Problem
-------
Set covering problem

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
Time limit:                          2
Messages
    Verbosity level:                 1
    Standard output:                 1
    File path:                       
    # streams:                       0
Logger
    Has logger:                      0
    Standard error:                  0
    File path:                       
Reduction
    Enable:                          1
    Max. # of rounds:                10
    Remove dominated:                0
Max. # of iterations:                -1
Max. # of iterations without impr.:  -1

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
       0.000         inf           0         inf         inf                        
       0.094         153           0         153         inf        initial solution
       0.095         152           0         152         inf            iteration 21
       0.101         151           0         151         inf            iteration 34
       0.102         150           0         150         inf            iteration 46
       0.106         149           0         149         inf           iteration 618
       0.106         148           0         148         inf           iteration 650
       0.110         147           0         147         inf          iteration 1096
       0.114         146           0         146         inf          iteration 1680
       0.132         145           0         145         inf          iteration 4001
       0.133         144           0         144         inf          iteration 4190
       0.134         143           0         143         inf          iteration 4215
       0.136         142           0         142         inf          iteration 4582
       0.144         141           0         141         inf          iteration 5468
       0.145         140           0         140         inf          iteration 5622
       0.161         139           0         139         inf          iteration 7599
       0.164         138           0         138         inf          iteration 7843
       0.187         137           0         137         inf         iteration 10881
       0.218         136           0         136         inf         iteration 14722
       0.244         135           0         135         inf         iteration 18092
       0.269         134           0         134         inf         iteration 20601
       0.275         133           0         133         inf         iteration 21075
       0.467         132           0         132         inf         iteration 41779
       0.571         131           0         131         inf         iteration 54229
       1.274         130           0         130         inf        iteration 132511

Final statistics
----------------
Value:                        130
Bound:                        0
Absolute optimality gap:      130
Relative optimality gap (%):  inf
Time (s):                     2.00003
Number of iterations:         207256

Solution
--------
Number of sets:                130 / 55515 (0.234171%)
Number of uncovered elements:  0 / 582 (0%)
Feasible:                      1
Cost:                          130
```

```shell
./install/bin/setcoveringsolver -v 1 -i data/gecco2020/AC_15_cover.txt -f gecco2020 --unicost -a local-search-row-weighting-2 -t 10 -c solution.txt
```
```
=====================================
          SetCoveringSolver          
=====================================

Problem
-------
Set covering problem

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
Time limit:                          10
Messages
    Verbosity level:                 1
    Standard output:                 1
    File path:                       
    # streams:                       0
Logger
    Has logger:                      0
    Standard error:                  0
    File path:                       
Reduction
    Enable:                          1
    Max. # of rounds:                10
    Remove dominated:                0
Max. # of iterations:                -1
Max. # of iterations without impr.:  -1

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
       0.000         inf           0         inf         inf                        
       0.781         660           0         660         inf        initial solution
       0.785         659           0         659         inf             it 1 comp 0
       0.789         658           0         658         inf             it 3 comp 0
       0.791         657           0         657         inf             it 5 comp 0
       0.792         656           0         656         inf             it 6 comp 0
       0.794         655           0         655         inf             it 7 comp 0
       0.795         654           0         654         inf             it 8 comp 0
       0.797         653           0         653         inf             it 9 comp 0
       0.800         652           0         652         inf            it 12 comp 0
       0.802         651           0         651         inf            it 14 comp 0
       0.804         650           0         650         inf            it 15 comp 0
       0.805         649           0         649         inf            it 16 comp 0
       0.807         648           0         648         inf            it 17 comp 0
       0.810         647           0         647         inf            it 20 comp 0
       0.811         646           0         646         inf            it 21 comp 0
       0.814         645           0         645         inf            it 24 comp 0
       0.816         644           0         644         inf            it 25 comp 0
       0.817         643           0         643         inf            it 26 comp 0
       0.820         642           0         642         inf            it 30 comp 0
       0.821         641           0         641         inf            it 31 comp 0
       0.823         640           0         640         inf            it 33 comp 0
       0.824         639           0         639         inf            it 34 comp 0
       0.828         638           0         638         inf            it 38 comp 0
       0.830         637           0         637         inf            it 40 comp 0
       0.833         636           0         636         inf            it 42 comp 0
       0.838         635           0         635         inf            it 53 comp 0
       0.842         634           0         634         inf            it 57 comp 0
       0.843         633           0         633         inf            it 58 comp 0
       0.846         632           0         632         inf            it 60 comp 0
       0.848         631           0         631         inf            it 63 comp 0
       0.858         630           0         630         inf            it 75 comp 0
       0.860         629           0         629         inf            it 79 comp 0
       0.862         628           0         628         inf            it 81 comp 0
       0.863         627           0         627         inf            it 82 comp 0
       0.865         626           0         626         inf            it 86 comp 0
       0.868         625           0         625         inf            it 90 comp 0
       0.877         624           0         624         inf           it 104 comp 0
       0.879         623           0         623         inf           it 106 comp 0
       0.883         622           0         622         inf           it 111 comp 0
       0.893         621           0         621         inf           it 124 comp 0
       0.896         620           0         620         inf           it 130 comp 0
       0.900         619           0         619         inf           it 135 comp 0
       0.903         618           0         618         inf           it 138 comp 0
       0.908         617           0         617         inf           it 146 comp 0
       0.916         616           0         616         inf           it 168 comp 0
       0.935         615           0         615         inf           it 201 comp 0
       0.942         614           0         614         inf           it 216 comp 0
       0.972         613           0         613         inf           it 264 comp 0
       0.980         612           0         612         inf           it 276 comp 0
       0.985         611           0         611         inf           it 284 comp 0
       0.991         610           0         610         inf           it 295 comp 0
       0.997         609           0         609         inf           it 303 comp 0
       1.004         608           0         608         inf           it 316 comp 0
       1.008         607           0         607         inf           it 322 comp 0
       1.016         606           0         606         inf           it 341 comp 0
       1.033         605           0         605         inf           it 365 comp 0
       1.041         604           0         604         inf           it 378 comp 0
       1.045         603           0         603         inf           it 382 comp 0
       1.076         602           0         602         inf           it 432 comp 0
       1.097         601           0         601         inf           it 465 comp 0
       1.101         600           0         600         inf           it 472 comp 0
       1.119         599           0         599         inf           it 499 comp 0
       1.158         598           0         598         inf           it 567 comp 0
       1.194         597           0         597         inf           it 626 comp 0
       1.198         596           0         596         inf           it 630 comp 0
       1.207         595           0         595         inf           it 643 comp 0
       1.222         594           0         594         inf           it 672 comp 0
       1.229         593           0         593         inf           it 684 comp 0
       1.240         592           0         592         inf           it 703 comp 0
       1.246         591           0         591         inf           it 715 comp 0
       1.253         590           0         590         inf           it 726 comp 0
       1.328         589           0         589         inf           it 845 comp 0
       1.460         588           0         588         inf          it 1066 comp 0
       1.595         587           0         587         inf          it 1296 comp 0
       1.648         586           0         586         inf          it 1378 comp 0
       1.665         585           0         585         inf          it 1405 comp 0
       1.780         584           0         584         inf          it 1601 comp 0
       1.784         583           0         583         inf          it 1606 comp 0
       2.078         582           0         582         inf          it 2141 comp 0
       2.176         581           0         581         inf          it 2303 comp 0
       2.231         580           0         580         inf          it 2402 comp 0
       2.236         579           0         579         inf          it 2408 comp 0
       2.294         578           0         578         inf          it 2497 comp 0
       4.412         577           0         577         inf          it 6058 comp 0
       4.451         576           0         576         inf          it 6104 comp 0
       5.352         575           0         575         inf          it 7750 comp 0
       7.143         574           0         574         inf         it 10939 comp 0
       7.179         573           0         573         inf         it 10991 comp 0
       8.388         572           0         572         inf         it 13081 comp 0
       8.543         571           0         571         inf         it 13384 comp 0
       8.546         570           0         570         inf         it 13388 comp 0
       8.657         569           0         569         inf         it 13569 comp 0
       8.990         568           0         568         inf         it 14113 comp 0
       9.001         567           0         567         inf         it 14131 comp 0

Final statistics
----------------
Value:                         567
Bound:                         0
Absolute optimality gap:       567
Relative optimality gap (%):   inf
Time (s):                      10.0003
Number of iterations:          15834
Neighborhood 1 improvements:   15834
Neighborhood 2 improvements:   0
Neighborhood 1 time:           8.85167
Neighborhood 2 time:           0
Number of weights reductions:  0

Solution
--------
Number of sets:                567 / 89304 (0.63491%)
Number of uncovered elements:  0 / 18605 (0%)
Feasible:                      1
Cost:                          567
```
