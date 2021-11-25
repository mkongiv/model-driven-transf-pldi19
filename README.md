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
