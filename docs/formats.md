# File formats

This page describes the instance file formats read by `setcoveringsolver` (option `--format`) and the certificate file formats used to write solutions (option `--certificate`) and to read initial solutions (option `--initial-solution`).

In all formats, numbers are separated by whitespace. Unless stated otherwise, the line structure shown in the examples is not required.

A set covering instance consists of elements and of sets with a cost, each set covering some of the elements. A solution is a selection of sets such that each element is covered by at least one selected set. Its cost is the sum of the costs of the selected sets.

The option `--unicost` sets the cost of all sets to 1, whatever the format.

## Instance formats

| Format | Aliases | Costs | Set indices | Datasets in `data/` |
|---|---|---|---|---|
| [`balas1980`](#balas1980-orlibrary) | `orlibrary` | yes | 1 to n | `balas1980`, `beasley1987`, `beasley1990`, `grossman1997` |
| [`balas1996`](#balas1996) | | yes | 1 to n (implicit) | `balas1996` |
| [`faster1994`](#faster1994-wedelin1995) | `faster`, `wedelin1995`, `wedelin` | yes | 1 to n (implicit) | `faster1994`, `wedelin1995` |
| [`fulkerson1974`](#fulkerson1974-sts) | `sts` | no | 1 to n | `fulkerson1974` |
| [`gecco2020`](#gecco2020) | `gecco` | no | 0 to n - 1 | `gecco2020` |
| [`pace2025`](#pace2025) | | no | 1 to n | `pace2025` |
| [`pace2025_ds`](#pace2025_ds) | | no | 1 to n | `pace2025_ds` |
| [`pace2019_vc`](#pace2019_vc) | | no | 1 to n | |
| [`dimacs2010_vc`](#dimacs2010_vc) | | no | 1 to n | |

In formats without costs, all sets have cost 1.

The examples below describe the same instance with 3 elements and 3 sets, when the format allows it:

| Set | Cost | Elements |
|---|---|---|
| 1 | 2 | 1, 2 |
| 2 | 3 | 2, 3 |
| 3 | 4 | 1, 3 |

Its optimal solution selects sets 1 and 2, for a cost of 5.

### balas1980 (orlibrary)

Format of the [OR-Library](https://people.brunel.ac.uk/~mastjjb/jeb/orlib/scpinfo.html) set covering instances.

- The number of elements and the number of sets.
- The cost of each set.
- For each element: the number of sets covering it, followed by these sets (from 1 to n).

```
3 3
2 3 4
2 1 3
2 1 2
2 2 3
```

### balas1996

- The number of sets and the number of elements (in this order).
- The cost of each set.
- For each set: the number of elements it covers, followed by these elements (from 1 to m).

Sets are not numbered in the file: the i-th set listed is set i.

```
3 3
2 3 4
2 1 2
2 2 3
2 1 3
```

### faster1994 (wedelin1995)

- The number of elements and the number of sets.
- For each set: its cost, the number of elements it covers, followed by these elements (from 1 to m).

Sets are not numbered in the file: the i-th set listed is set i.

```
3 3
2 2 1 2
3 2 2 3
4 2 1 3
```

### fulkerson1974 (sts)

Format of the Steiner triple system instances, in which each element is covered by exactly 3 sets.

- The number of sets and the number of elements (in this order).
- For each element: the 3 sets covering it (from 1 to n).

```
3 2
1 2 3
1 2 3
```

### gecco2020

Format of the instances of the GECCO 2020 competition on optimal camera placement and unicost set covering.

- The number of elements and the number of sets.
- For each element: its index (ignored), the number of sets covering it, followed by these sets (from 0 to n - 1).

```
3 3
0
2
0 2
1
2
0 1
2
2
1 2
```

### pace2025

Hitting set format of the [PACE 2025 challenge](https://pacechallenge.org/2025/). A hitting set instance is a set covering instance in which vertices are sets and hyperedges are elements.

- Lines starting with `c` are comments.
- The problem line `p hs <number of sets> <number of elements>`.
- Then one line per element, listing the sets covering it (from 1 to n).

Each line describes one element, so the line structure is required.

```
c S1 = {1, 2}, S2 = {2, 3}, S3 = {1, 3}
p hs 3 3
1 3
1 2
2 3
```

### pace2025_ds

Dominating set format of the [PACE 2025 challenge](https://pacechallenge.org/2025/). Each vertex of the graph is both a set and an element: the set of vertex v covers v and its neighbors, so a solution is a dominating set.

- Lines starting with `c` are comments.
- The problem line `p ds <number of vertices> <number of edges>`.
- Then one line per edge: its two endpoints (from 1 to n).

The line structure is required.

```
c path 1 - 2 - 3
p ds 3 2
1 2
2 3
```

The optimal solution selects vertex 2.

### pace2019_vc

Graph format of the vertex cover track of the PACE 2019 challenge. Each vertex is a set and each edge is an element covered by its two endpoints, so a solution is a vertex cover.

- Lines starting with `c` are comments.
- The problem line `p <descriptor> <number of vertices> <number of edges>`. The descriptor is not checked.
- Then one line per edge: its two endpoints (from 1 to n).

The line structure is required.

```
c path 1 - 2 - 3
p vc 3 2
1 2
2 3
```

The optimal solution selects vertex 2.

### dimacs2010_vc

Graph format of the [10th DIMACS implementation challenge](https://sites.cc.gatech.edu/dimacs10/) (METIS format). As for `pace2019_vc`, each vertex is a set and each edge is an element covered by its two endpoints.

- The first line: the number of vertices and the number of edges, optionally followed by a format field, which is ignored (vertex and edge weights are not supported).
- Then line i lists the neighbors of vertex i (from 1 to n). An isolated vertex has an empty line.

The line structure is required. Comment lines are not supported.

```
3 2
2
1 3
2
```

The optimal solution selects vertex 2.

## Certificate formats

Certificate files contain the number of selected sets, followed by the selected sets.

Each instance format has a certificate format with the same name (aliases included), which uses the same set indices as the instance format. When `--certificate-format` is not provided, the certificate format of the instance format is used, both for writing the certificate (`--certificate`) and for reading the initial solution (`--initial-solution`).

| Certificate formats | Set indices | Layout written |
|---|---|---|
| `gecco2020`, `gecco` | 0 to n - 1 | the number of sets on the first line, then the sets on one line |
| all other formats | 1 to n | the number of sets on the first line, then one set per line |

For formats in which sets are not numbered in the file (`balas1996`, `faster1994`), set i is the i-th set listed in the file.

When reading an initial solution, the line structure is not required, and an error is reported if the file contains fewer sets than announced, a set index out of range or a set listed several times.

Certificate of the optimal solution of the example instance, in the `balas1980` certificate format:

```
2
1
2
```

The same solution in the `gecco2020` certificate format:

```
2
0 1
```
