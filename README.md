## Project Overview
This project involves designing and implementing a cache simulator.
The simulator reads a memory access trace from a file and determines whether each memory access is a hit or a miss, then calculates the hit rate.
Additionally, the project includes an analysis paper that investigates the performance (hit rate) of three cache designs under different configurations.

## Cache Designs
The simulator implements three cache designs: direct mapped, set associative, and fully associative.
Each cache design has different characteristics in terms of associativity, cache size, and replacement policy.

## Simulation Process
The simulator reads a memory access trace file containing memory addresses accessed during program execution.
It translates memory addresses into cache indices, tags, and offsets.
It simulates cache access for each memory address and determines whether it's a hit or a miss.
Hit rates are calculated based on the number of hits and misses recorded during simulation.

## Screenshot
![image](https://github.com/thomas-martin-uf/Cache-Simulation/assets/109101463/0ea04b73-df8c-415f-8527-db96fe0e46aa)
