# model-driven-transf-pldi19
Patches for pldi19 paper titled Model-driven Transformations for etc 


Dependencies:
* IBM Cplex. Set environment variable 'CPLEX_ROOT' to root of cplex installation dir.
* autotools
* automake
* libgmp 2.6.1

To run do:

./run-mdt.sh <filename-no-extension> <sched-dims-in-2d+1>  <SKX|KNL|PWR9>

For example (gemm.c, loop-depth=3 --> 7 scheduling dimensions):

./run-mdt.sh gemm 7 SKX
  
Please cite [https://dl.acm.org/doi/10.1145/3314221.3314653](this article as):
  
@inproceedings{10.1145/3314221.3314653,
author = {Kong, Martin and Pouchet, Louis-No\"{e}l},
title = {Model-Driven Transformations for Multi- and Many-Core CPUs},
year = {2019},
isbn = {9781450367127},
publisher = {Association for Computing Machinery},
address = {New York, NY, USA},
url = {https://doi.org/10.1145/3314221.3314653},
doi = {10.1145/3314221.3314653},
abstract = {Modern polyhedral compilers excel at aggressively optimizing codes with static control parts, but the state-of-practice to find high-performance polyhedral transformations especially for different hardware targets still largely involves auto-tuning. In this work we propose a novel customizable polyhedral scheduling technique, with the aim of delivering high performance for several hardware targets. We design constraints and objectives that model several crucial aspects of performance such as stride optimization or the trade-off between parallelism and reuse, while considering important architectural features of the target machine. We evaluate our work using the PolyBench/C benchmark suite and experimentally validate it against large optimization spaces generated with the Pluto compiler on 3 representative architectures: an IBM Power9, an Intel Xeon Phi and an Intel Core-i9. Our results show we can achieve comparable or superior performance to Pluto on the majority of benchmarks, without implementing tiling in the source code nor using experimental autotuning.},
booktitle = {Proceedings of the 40th ACM SIGPLAN Conference on Programming Language Design and Implementation},
pages = {469–484},
numpages = {16},
keywords = {single-shot ILP, composable transformations, polyhedral optimizations, scheduling, Affine transformations},
location = {Phoenix, AZ, USA},
series = {PLDI 2019}
}
