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
```

Run:

```shell
./bazel-bin/setcoveringsolver/main -v 1 -i data/faster1994/rail582.txt -f faster --unicost -a localsearch_rowweighting_2 -c solution.txt -t 2
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
Row Weighting Local Search 2

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
./bazel-bin/setcoveringsolver/main -v 1 -i data/gecco2020/AC_15_cover.txt -f gecco2020 --unicost -a localsearch_rowweighting -t 10 -c solution.txt
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
Row Weighting Local Search

Reduction
---------
Number of unfixed elements:  18605/18605 (0 fixed)
Number of unfixed sets:      72684/89304 (16620 fixed)

Compute set neighbors...

       T (s)          UB          LB         GAP     GAP (%)                 Comment
       -----          --          --         ---     -------                 -------
      0.4678       89304           0       89304         inf                        
      0.5587         641           0         641         inf        initial solution
      0.5605         640           0         640         inf             it 3 comp 0
      0.5661         639           0         639         inf             it 8 comp 0
      0.5672         638           0         638         inf             it 9 comp 0
      0.5685         637           0         637         inf            it 10 comp 0
      0.5695         636           0         636         inf            it 11 comp 0
      0.5714         635           0         635         inf            it 13 comp 0
       0.576         634           0         634         inf            it 18 comp 0
      0.5774         633           0         633         inf            it 19 comp 0
      0.5784         632           0         632         inf            it 20 comp 0
      0.5792         631           0         631         inf            it 21 comp 0
      0.5801         630           0         630         inf            it 22 comp 0
      0.5808         629           0         629         inf            it 23 comp 0
      0.5841         628           0         628         inf            it 29 comp 0
       0.587         627           0         627         inf            it 32 comp 0
      0.5957         626           0         626         inf            it 47 comp 0
      0.5978         625           0         625         inf            it 52 comp 0
      0.5995         624           0         624         inf            it 55 comp 0
      0.6019         623           0         623         inf            it 58 comp 0
      0.6056         622           0         622         inf            it 62 comp 0
      0.6098         621           0         621         inf            it 70 comp 0
      0.6123         620           0         620         inf            it 73 comp 0
      0.6167         619           0         619         inf            it 78 comp 0
      0.6199         618           0         618         inf            it 84 comp 0
      0.6499         617           0         617         inf           it 126 comp 0
      0.6571         616           0         616         inf           it 135 comp 0
      0.6622         615           0         615         inf           it 141 comp 0
      0.6631         614           0         614         inf           it 142 comp 0
      0.6678         613           0         613         inf           it 148 comp 0
      0.6887         612           0         612         inf           it 175 comp 0
      0.6975         611           0         611         inf           it 188 comp 0
      0.6994         610           0         610         inf           it 190 comp 0
      0.7073         609           0         609         inf           it 200 comp 0
      0.7118         608           0         608         inf           it 206 comp 0
      0.7237         607           0         607         inf           it 236 comp 0
      0.7256         606           0         606         inf           it 238 comp 0
      0.7554         605           0         605         inf           it 279 comp 0
      0.7621         604           0         604         inf           it 287 comp 0
       0.772         603           0         603         inf           it 302 comp 0
       0.789         602           0         602         inf           it 334 comp 0
      0.8018         601           0         601         inf           it 355 comp 0
      0.8051         600           0         600         inf           it 360 comp 0
      0.8083         599           0         599         inf           it 368 comp 0
       0.848         598           0         598         inf           it 427 comp 0
      0.8851         597           0         597         inf           it 484 comp 0
      0.9396         596           0         596         inf           it 558 comp 0
      0.9439         595           0         595         inf           it 564 comp 0
      0.9511         594           0         594         inf           it 575 comp 0
      0.9648         593           0         593         inf           it 597 comp 0
      0.9978         592           0         592         inf           it 643 comp 0
      1.0578         591           0         591         inf           it 734 comp 0
      1.0677         590           0         590         inf           it 749 comp 0
      1.0926         589           0         589         inf           it 805 comp 0
      1.1084         588           0         588         inf           it 826 comp 0
      1.1163         587           0         587         inf           it 841 comp 0
      1.2482         586           0         586         inf          it 1044 comp 0
      1.2561         585           0         585         inf          it 1054 comp 0
      1.2595         584           0         584         inf          it 1059 comp 0
      1.2793         583           0         583         inf          it 1093 comp 0
      1.4527         582           0         582         inf          it 1324 comp 0
      1.5559         581           0         581         inf          it 1452 comp 0
      2.6289         580           0         580         inf          it 3020 comp 0
      4.7291         579           0         579         inf          it 6183 comp 0
      4.9129         578           0         578         inf          it 6487 comp 0
      5.0021         577           0         577         inf          it 6598 comp 0
      5.2493         576           0         576         inf          it 6927 comp 0
      5.2594         575           0         575         inf          it 6939 comp 0
       5.326         574           0         574         inf          it 7055 comp 0
      5.3695         573           0         573         inf          it 7110 comp 0
      5.8834         572           0         572         inf          it 7804 comp 0
      6.0148         571           0         571         inf          it 7962 comp 0
      7.4605         570           0         570         inf          it 9903 comp 0
      7.9649         569           0         569         inf         it 10714 comp 0

Final statistics
----------------
Value:                 569
Bound:                 0
Gap:                   569
Gap (%):               inf
Time (s):              10.0002
Number of iterations:  13475
```

Benchmarks:
```shell
python3 ../optimizationtools/scripts/bench_run.py --algorithms "greedy"
python3 ../optimizationtools/scripts/bench_run.py --algorithms "greedy_lin"
python3 ../optimizationtools/scripts/bench_run.py --algorithms "greedy_dual"
python3 ../optimizationtools/scripts/bench_process.py --benchmark heuristicshort --timelimit 10 --labels "greedy" "greedy_lin" "greedy_dual"
```

![heuristicshort](img/heuristicshort.png?raw=true "heuristicshort")

