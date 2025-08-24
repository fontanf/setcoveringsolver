# SetCoveringSolver

A solver for the (unicost) set covering and set packing problems.

In the set packing problem solved, elements may be covered multiple times and there might be multiple copies of a set.

## Implemented algorithms

### Set covering

- Greedy
  - Greedy `--algorithm greedy`
  - Greedy Lin `--algorithm greedy-lin`: based on the one from "Weighting-Based Parallel Local Search for Optimal Camera Placement and Unicost Set Covering" (Lin et al., 2020)
  - Dual greedy `--algorithm greedy-dual`
  - Reverse greedy `--algorithm greedy-reverse`
  - Greedy or reverse greedy `--algorithm greedy-or-greedy-reverse`: runs the best suited greedy algorithms for the instance

- Mixed-integer linear program `--algorithm milp --solver highs`

- Row weighting local search (unicost only) `--algorithm local-search-row-weighting`

- Large neighborhood search `--algorithm large-neighborhood-search --maximum-number-of-iterations 100000 --maximum-number-of-iterations-without-improvement 10000`

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
python3 scripts/download_data.py --data pace2025
python3 scripts/download_data.py --data pace2025_ds
```

Run:

```shell
./install/bin/setcoveringsolver  --verbosity-level 1  --input data/wedelin1995/sasd9imp2.dat --format wedelin1995  --algorithm milp --solver cbc
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
./install/bin/setcoveringsolver  --verbosity-level 1  --input data/faster1994/rail582.txt --format faster --unicost  --algorithm local-search-row-weighting  --certificate solution.txt  --time-limit 10
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
Average number of set neighbors estimate:     20584.4
Average number of elt. neighbors estimate:    4706.25
Total cost:                                   55515
Number of connected components:               1

Algorithm
---------
Row weighting local search

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
    Time limit:                      inf
    Max. # of rounds:                999
    Set folding:                     1
    Twin:                            1
    Unconfined sets:                 1
    Dominated sets removal:          1
    Dominated elts removal:          1
Max. # of iterations:                -1
Max. # of iterations without impr.:  -1

Reduced instance
----------------
Number of elements:                           543
Number of sets:                               26520
Number of arcs:                               189992
Average number of sets covering an element:   349.893
Average number of elements covered by a set:  7.1641
Average number of set neighbors estimate:     8031.2
Average number of elt. neighbors estimate:    2380.2
Total cost:                                   26520
Number of connected components:               1

    Time (s)       Value       Bound         Gap     Gap (%)                 Comment
    --------       -----       -----         ---     -------                 -------
       0.000       55515           0       55515         inf                        
       5.056       55515           7       55508   792971.43                        
       5.062       26527           7       26520   378857.14           trivial bound
       5.064       26527          53       26474    49950.94           trivial bound
       5.087         140          53          87      164.15        initial solution
       5.089         139          53          86      162.26                   it 14
       5.156         138          53          85      160.38                 it 4883
       5.194         137          53          84      158.49                 it 7437
       5.225         136          53          83      156.60                 it 9741
       5.245         135          53          82      154.72                it 11268
       5.329         134          53          81      152.83                it 17924
       5.354         133          53          80      150.94                it 20291
       5.372         132          53          79      149.06                it 20912
       5.457         131          53          78      147.17                it 26284
       5.632         130          53          77      145.28                it 39613
       5.655         129          53          76      143.40                it 40844
       6.045         128          53          75      141.51                it 67827
       6.669         127          53          74      139.62               it 101802

Final statistics
----------------
Value:                         127
Bound:                         53
Absolute optimality gap:       74
Relative optimality gap (%):   139.623
Time (s):                      10.0004
Number of iterations:          319919

Solution
--------
Number of sets:                127 / 55515 (0.228767%)
Number of uncovered elements:  0 / 582 (0%)
Feasible:                      1
Cost:                          127
```
