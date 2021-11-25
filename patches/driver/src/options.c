/*
 * options.c: this file is part of the PoCC project.
 *
 * PoCC, the Polyhedral Compiler Collection package
 *
 * Copyright (C) 2009 Louis-Noel Pouchet
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * The complete GNU General Public Licence Notice can be found as the
 * `COPYING.LESSER' file in the root directory.
 *
 * Author:
 * Louis-Noel Pouchet <Louis-Noel.Pouchet@inria.fr>
 *
 */
#include <libgen.h>
#if HAVE_CONFIG_H
# include <pocc-utils/config.h>
#endif

#include "options.h"
#include <ponos/options.h>
#include <pocc/driver-psimdkzer.h>
#include <pocc/error.h>

#ifdef POCC_DEVEL_MODE
static const struct s_opt       opts[POCC_NB_OPTS] =
{
  { 'h', "help", 0, "Print this help" , "\t\t(S)" },
  { 'v', "version", 0, "Print version information" , "\t(S)" },
  { 'o', "output", 1, "Output file [filename.pocc.c]" , "\t(S)" },
  { '\0', "output-scop", 0, "Output scoplib file to\n\t\t\t\t\tfilename.pocc.scop" , "\t(S)" },
  { '\0', "cloogify-scheds", 0, "Create CLooG-compatible schedules in\n\t\t\t\t\tthe scop" , "(S)" },
  { '\0', "bounded-ctxt", 0, "Parser: bound all global parameters\n\t\t\t\t\tto >= -1" , "\t(S)" },
  { '\0', "default-ctxt", 0, "Parser: bound all global parameters\n\t\t\t\t\tto>= 32" , "\t(S)" },
  { '\0', "user-ctxt", 1, "Parser: set  parameters to value\n\t\t\t\t\t \"42,31,...\"" , "(S)" },
  { '\0', "inscop-fakearray", 0, "Parser: use FAKEARRAY[i] to explicitly\n\t\t\t\t\tdeclare write dependences" , "(E)" },
  { '\0', "read-scop", 0, "Parser: read SCoP file instead of C file\n\t\t\t\t\tas input\n" , "\t(S)" },
  { '\0', "no-candl", 0, "Dependence analysis: don't run candl" , "\t(S)" },
  { '\0', "candl-dep-isl-simp", 0, "Dependence analysis: simplify with ISL" , "(E)" },
  { '\0', "candl-dep-prune", 0, "Dependence analysis: prune redundant\n\t\t\t\t\tdeps\n" , "(E)" },
  { '\0', "polyfeat", 0, "Run Polyhedral Feature Extraction" , "\t(E)" },
  { '\0', "polyfeat-leg", 0, "Use legacy PolyFeat only" , "\t(E)" },
  { '\0', "polyfeat-rar", 0, "Consider RAR dependences in PolyFeat" , "\t(E)" },
  { '\0', "polyfeat-ls", 1, "Cache line size (in scalars)" , "(E)" },

  { '\0', "polyfeat-cs", 1, "Cache size (in lines)" , "(E)" },
  { '\0', "polyfeat-priv", 0, "Cache is private" , "(E)" },
  { '\0', "polyfeat-multi", 1, "Series of values (ls,cs)\n" , "(E)" },

  { 'd', "delete-files", 0, "Delete files previously generated\n\t\t\t\t\tby PoCC\n" , "\t(S)" },
  { '\0', "verbose", 0, "Verbose output" , "\t(S)" },
  { '\0', "quiet", 0, "Minimal output\n" , "\t(S)" },
  { 'l', "letsee", 0, "Optimize with LetSee" , "\t(S)" },
  { '\0', "letsee-space", 1, "LetSee: search space: [precut], schedule" , "(E)" },
  { '\0', "letsee-walk", 1, "LetSee: traversal heuristic:\n\t\t\t\t\t[exhaust], random, skip, m1, dh, ga" , "(B)" },
  { '\0', "letsee-dry-run", 0, "Only generate source files" , "(S)" },
  { '\0', "letsee-normspace", 0, "LetSee: normalize search space" , "(E)" },
  { '\0', "letsee-bounds", 1, "LetSee: search space bounds\n\t\t\t\t\t[-1,1,-1,1,-1,1]" , "(E)" },
  { '\0', "letsee-mode-m1", 1, "LetSee: scheme for M1 traversal\n\t\t\t\t\t[i+p,i,0]" , "(E)" },
  { '\0', "letsee-rtries", 1, "LetSee: set number of random draws [50]" , "(E)" },
  { '\0', "letsee-prune-precut", 0, "LetSee: prune precut space" , "(E)" },
  { '\0', "letsee-backtrack", 0, "LetSee: allow bactracking in\n\t\t\t\t\tschedule mode\n" , "(E)" },
  { 'p', "pluto", 0, "Optimize with PLuTo" , "\t(S)" },
  { '\0', "pluto-parallel", 0, "PLuTo: Wavefront parallelization " , "(S)" },
  { '\0', "pluto-tile", 0, "PLuTo: polyhedral tiling" , "\t(S)" },
  { '\0', "pluto-l2tile", 0, "PLuTo: perform L2 tiling" , "\t(E)" },
  { '\0', "pluto-fuse", 1, "PLuTo: fusion heuristic:\n\t\t\t\t\tmaxfuse, [smartfuse], nofuse" , "(S)" },
  { '\0', "pluto-unroll", 0, "PLuTo: unroll loops" , "\t(B)" },
  { '\0', "pluto-ufactor", 1, "PLuTo: unrolling factor [4]" , "(B)" },
  { '\0', "pluto-polyunroll", 0, "PLuTo: polyhedral unrolling" , "(B)" },
  { '\0', "pluto-prevector", 0, "PLuTo: perform prevectorization" , "(S)" },
  { '\0', "pluto-multipipe", 0, "PLuTo: multipipe" , "(E)" },
  { '\0', "pluto-rar", 0, "PLuTo: consider RAR dependences" , "\t(S)" },
  { '\0', "pluto-rar-cf", 0, "PLuTo: consider RAR dependences for \n\t\t\t\t\tcost function only" , "\t(S)" },
  { '\0', "pluto-lastwriter", 0, "PLuTo: perform lastwriter dependence\n\t\t\t\t\tsimplification" , "(E)" },
  { '\0', "pluto-scalpriv", 0, "PLuTo: perform scalar privatization" , "(S)" },
  { '\0', "pluto-bee", 0, "PLuTo: use Bee" , "\t(B)" },
  { '\0', "pluto-quiet", 0, "PLuTo: be quiet" , "\t(S)" },
  { '\0', "pluto-ft", 0, "PLuTo: CLooG ft " , "\t(S)" },
  { '\0', "pluto-lt", 0, "PLuTo: CLooG lt" , "\t(S)" },
  { '\0', "pluto-ext-candl", 0, "PLuTo: Read dependences from SCoP" , "(S)" },
  { '\0', "pluto-tile-scat", 0, "PLuTo: Perform tiling inside scatterings" , "(S)" },
  { '\0', "pluto-bounds", 1, "PLuTo: Transformation coefficients\n\t\t\t\t\tbounds [+inf]\n" , "(S)" },

  { 'n', "no-codegen", 0, "Do not generate code" , "\t(S)" },
  { '\0', "cloog-cloogf", 1, "CLooG: first level to scan [1]" , "(S)" },
  { '\0', "cloog-cloogl", 1, "CLooG: last level to scan [-1]" , "(S)" },
  { '\0', "print-cloog-file", 0, "CLooG: print input CLooG file" , "(S)" },
  { '\0', "no-past", 0, "Do not use the PAST back-end" , "\t(B)" },
  { '\0', "past-hoist-lb", 0, "Hoist loop bounds" , "(S)" },
  { '\0', "pragmatizer", 0, "Use the AST pragmatizer\n" , "\t(S)" },

  { '\0', "ptile", 0, "Use PTile for parametric tiling" , "\t(S)" },
  { '\0', "ptile-fts", 0, "Use full-tile separation in PTile" , "\t(S)" },
  { '\0', "ptile-level", 1, "Level(s) of tiling [1]" , "(E)" },
  { '\0', "evolvetile", 0, "Aaptive tiled code generation\n" , "\t(E)" },

  { '\0', "punroll", 0, "Use PAST loop unrolling" , "\t(S)" },
  { '\0', "register-tiling", 0, "PAST register tiling" , "(E)" },
  { '\0', "punroll-size", 1, "PAST unrolling size [4]" , "(S)" },
  { '\0', "vectorizer", 0, "Post-transform for vectorization" , "\t(S)" },
  { '\0', "simdizer", 0, "SIMDize inner vectorizable loops" , "\t(E)" },
  { '\0', "codegen-timercode", 0, "Codegen: insert timer code" , "(E)" },
  { '\0', "codegen-timer-asm", 0, "Codegen: insert ASM timer code" , "(E)" },
  { '\0', "codegen-timer-papi", 0, "Codegen: insert PAPI timer code" , "(E)" },
  { '\0', "codegen-dinero", 0, "Codegen: insert DineroIV trace code\n" , "(S)" },
  { 'c', "compile", 0, "Compile program with C compiler" , "\t(S)" },
  { '\0', "compile-cmd", 1, "Compilation command [gcc -O3 -lm]" , "(S)" },
  { '\0', "run-cmd-args", 1, "Program execution arguments []" , "(S)" },
  { '\0', "prog-timeout", 1, "Timeout for compilation and execution,\n\t\t\t\t\tin second\n", "(E)" },

  { 's', "ponos", 0, "Optimize with Ponos" , "\t(E)" },
  { '\0', "ponos-quiet", 0, "Ponos: be quiet" , "\t(E)" },
  { '\0', "ponos-debug", 0, "Ponos: debug mode" , "\t(E)" },
  { '\0', "ponos-sched-sz", 1, "Ponos: schedule dimensionality [1]" , "(E)" },
  { '\0', "ponos-coef-N", 0, "Ponos: all variables are >= 0" , "\t(E)" },
  { '\0', "ponos-build-2dp1", 0, "Ponos: build 2d+1 schedule" , "(E)" },
  { '\0', "ponos-solver", 1, "Ponos: solver: pip,cplex,cplex-incr" , "(E)" },
  { '\0', "ponos-solver-pre", 0, "Ponos: precondition the PIP" , "(E)" },
  { '\0', "ponos-farkas-max", 0, "Ponos: maxscale FM solver" , "(E)" },
  { '\0', "ponos-farkas-nored", 0, "Ponos: remove redundancy with FM" , "(E)" },
  { '\0', "ponos-K", 1, "Ponos: value for the K constant [10]" , "(E)" },
  { '\0', "ponos-coef", 1, "Ponos: schedule coefficients bound [10]" , "(E)" },
  { '\0', "ponos-obj", 1, "Ponos: objective constraints [none],\n\t\t\t\t\tcodelet,pluto" , "(E)" },
  { '\0', "ponos-obj-list", 1, "Ponos: constraints/objectives list from\n\t\t\t\t\tsumiterpos,paramcoef0,maxouterpar,\n\t\t\t\t\tmaxinnerpar,maxperm,mindepdist,\n\t\t\t\t\tmaxdepsolve,linearind,gammapos,\n\t\t\t\t\ttasched,minitercoef" , "(E)" },
  { '\0', "ponos-pipsolve-lp", 0, "Ponos: Relax PIP to rational" , "(E)" },
  { '\0', "ponos-pip-gmp", 0, "Ponos: use PIP-gmp\n" , "(E)" },

  { '\0', "past-super-hoist", 0, "Hoist loop bounds (super aggresive)" , "(E)" },
  { '\0', "read-cloog", 0, "read cloog file as input\n" , "\t(E)" },

  { '\0', "psimdkzer", 0, "Generate vector codelets" , "\t(E)" },
  { '\0', "pk-target", 1, "PSimdKzer code generator target:\n\t\t\t\t\tprevector,spiral,naive" , "(E)" },
  { '\0', "pk-vector-isa", 1, "PSimdKzer vector ISA:\n\t\t\t\t\tSSE,AVX" , "(E)" },
  { '\0', "pk-datatype", 1, "PSimdKzer scalar datatype:\n\t\t\t\t\tsingle,double" , "(E)" },
  { '\0', "pk-reskew", 1, "PSimdKzer reskew PLuTo schedule" , "(E)" },
  { '\0', "pk-gen-build-script", 0, "PSimdKzer: create Polybench script\n" , "(E)" },
  { '\0', "simd-width", 1, "SIMD width in bytes" , "(S)" },
  { '\0', "simd-fma", 0, "SIMD ISA has FMA support" , "\t(S)" },
  { '\0', "scalar-size", 1, "Scalar element size in bytes" , "(S)" },
  { '\0', "num-omp-thread", 1, "Number of OpenMP threads (1 per core)\n" , "(S)" },

  { '\0', "dump-approxscop", 0, "Dump over-approximated SCoP file of the\n\t\t\t\t\tgenerated AST\n" , "(E)" },
  { '\0', "output-aster", 0, "Output transfo as .aster file\n" , "\t(E)" },
  { 'i', "interpreter", 0, "Enter PoCC interpreter\n" , "\t(E)" },

  { '\0', "mark-par-loops", 0, "Detect parallel loops in generated\n\t\t\t\t\tcode" , "(E)" },
  { '\0', "no-outer-par-loops", 0, "Don't preserve outer parallel loop\n\t\t\t\t\tduring vectorization" , "(E)" },
  { '\0', "sink-all-loops", 0, "Sink all candidate vector loops during\n\t\t\t\t\tvectorization" , "(E)" },
  { '\0', "array-contraction", 0, "Perform array contraction [off]" , "(E)" },
  { '\0', "ac-keep-outer-par", 0, "Array contraction: preserve outer\n\t\t\t\t\tparallel loops" , "(E)" },
  { '\0', "ac-keep-vector-par", 0, "Array contraction: preserve inner\n\t\t\t\t\tparallel loops\n" , "(E)" },

  { '\0', "cdscgr", 0, "Call only the CDSC-GR pass\n" , "\t(E)" },
  { '\0', "ddg-analyze", 0, "Call only the DDG-analyze pass" , "\t(E)" },
  { '\0', "ddg-param", 1, "Slack for parameters in DDG [10000]\n" , "\t(E)" },
  { '\0', "ddg-prune", 1, "Cutoff dep. size for DDG removal [1]\n" , "\t(E)" },
  { '\0', "ddg-stats", 0, "Compute statistics\n" , "\t(E)" },
  { '\0', "ponos-chunked", 0, "Ponos: chunked codelet mode" , "\t\t(E)" },
  { '\0', "ponos-chunked-max-fusion", 0, "Ponos: chunked codelet max-fusion mode" , "\t(E)" },
  { '\0', "ponos-chunked-min-fusion", 0, "Ponos: chunked codelet min-fusion mode" , "\t(E)" },
  { '\0', "ponos-chunked-loop-max-stmt", 1, "Ponos: chunked codelet max stmts / loop" , "(E)" },
  { '\0', "ponos-chunked-loop-max-refs", 1, "Ponos: chunked codelet max refs / loop" , "(E)" },
  { '\0', "ponos-chunked-loop-max-lat", 1, "Ponos: chunked codelet max latency / loop" , "(E)" },
  { '\0', "ponos-chunked-arch", 1, "Ponos: chunked architecture (KNL,SKX,PWR9)" , "(E)" },
  { '\0', "ponos-chunked-arch-file", 1, "Ponos: chunked architecture spec filename" , "(E)" },
  { '\0', "ponos-chunked-fp-precision", 1, "Ponos: chunked architecture FP precision (bits)" , "(E)" },
  { '\0', "ponos-chunked-auto", 0, "Ponos: chunked auto-select objective from vocabulary" , "(E)" },
  { '\0', "ponos-chunked-read-olist", 0, "Ponos: chunked read objective list" , "(E)" },
  { '\0', "ponos-pipes-df-program", 0, "Ponos: generate .pipes file" , "(E)" },
  { '\0', "ponos-pipes-c-program", 0, "Ponos: generate dsa-fied .C program" , "(E)" },
  { '\0', "ponos-pipes-c-harness", 0, "Ponos: generate complete .C test harness" , "(E)" },
  { '\0', "ponos-pipes-schedule-t", 0, "Ponos: generate PIPES t-schedule" , "(E)" },
  { '\0', "ponos-pipes-schedule-p", 0, "Ponos: generate PIPES p-schedule" , "(E)" },
  { '\0', "ponos-mo-space", 0, "Ponos: chunked auto-select objective from vocabulary (no codegen)" , "(E)" }
};

#else
static const struct s_opt       opts[POCC_NB_OPTS] =
{
  { 'h', "help", 0, "Print this help" , "\t\t(S)" },
  { 'v', "version", 0, "Print version information" , "\t(S)" },
  { 'o', "output", 1, "Output file [filename.pocc.c]" , "\t(S)" },
  { '\0', "output-scop", 0, "Output scoplib file to\n\t\t\t\t\tfilename.pocc.scop" , "\t(S)" },
  { '\0', "cloogify-scheds", 0, "Create CLooG-compatible schedules in\n\t\t\t\t\tthe scop" , "(S)" },
  { '\0', "bounded-ctxt", 0, "Parser: bound all global parameters\n\t\t\t\t\tto >= -1" , "\t(S)" },
  { '\0', "default-ctxt", 0, "Parser: bound all global parameters\n\t\t\t\t\tto>= 32" , "\t(S)" },
  { '\0', "user-ctxt", 1, "Parser: set  parameters to value\n\t\t\t\t\t \"42,31,...\"" , "\t(S)" },
  { '\0', "inscop-fakearray", 0, "Parser: use FAKEARRAY[i] to explicitly\n\t\t\t\t\tdeclare write dependences" , "(E)" },
  { '\0', "read-scop", 0, "Parser: read SCoP file instead of C file\n\t\t\t\t\tas input\n" , "\t(S)" },
  { '\0', "no-candl", 0, "Dependence analysis: don't run candl" , "\t(S)" },
  { '\0', "candl-dep-isl-simp", 0, "Dependence analysis: simplify with ISL" , "(E)" },
  { '\0', "candl-dep-prune", 0, "Dependence analysis: prune redundant\n\t\t\t\t\tdeps\n" , "(E)" },
  { '\0', "polyfeat", 0, "Run Polyhedral Feature Extraction" , "\t(E)" },
  { '\0', "polyfeat-leg", 0, "Use legacy PolyFeat only" , "\t(E)" },
  { '\0', "polyfeat-rar", 0, "Consider RAR dependences in PolyFeat" , "\t(E)" },
  { '\0', "polyfeat-ls", 1, "Cache line size (in scalars)" , "(E)" },
  { '\0', "polyfeat-cs", 1, "Cache size (in lines)" , "(E)" },
  { '\0', "polyfeat-priv", 0, "Cache is private" , "(E)" },
  { '\0', "polyfeat-multi", 1, "Series of values (ls,cs)\n" , "(E)" },
  { 'd', "delete-files", 0, "Delete files previously generated\n\t\t\t\t\tby PoCC\n" , "\t(S)" },
  { '\0', "verbose", 0, "Verbose output" , "\t(S)" },
  { '\0', "quiet", 0, "Minimal output\n" , "\t(S)" },
  { 'l', "letsee", 0, "Optimize with LetSee" , "\t(S)" },
  { '\0', "letsee-space", 1, "LetSee: search space: [precut], schedule" , "(E)" },
  { '\0', "letsee-walk", 1, "LetSee: traversal heuristic:\n\t\t\t\t\t[exhaust], random, skip, m1, dh, ga" , "(B)" },
  { '\0', "letsee-dry-run", 0, "Only generate source files" , "(S)" },
  { '\0', "letsee-normspace", 0, "LetSee: normalize search space" , "(E)" },
  { '\0', "letsee-bounds", 1, "LetSee: search space bounds\n\t\t\t\t\t[-1,1,-1,1,-1,1]" , "(E)" },
  { '\0', "letsee-mode-m1", 1, "LetSee: scheme for M1 traversal\n\t\t\t\t\t[i+p,i,0]" , "(E)" },
  { '\0', "letsee-rtries", 1, "LetSee: set number of random draws [50]" , "(E)" },
  { '\0', "letsee-prune-precut", 0, "LetSee: prune precut space" , "(E)" },
  { '\0', "letsee-backtrack", 0, "LetSee: allow bactracking in\n\t\t\t\t\tschedule mode\n" , "(E)" },
  { 'p', "pluto", 0, "Optimize with PLuTo" , "\t(S)" },
  { '\0', "pluto-parallel", 0, "PLuTo: Wavefront parallelization " , "(S)" },
  { '\0', "pluto-tile", 0, "PLuTo: polyhedral tiling" , "\t(S)" },
  { '\0', "pluto-l2tile", 0, "PLuTo: perform L2 tiling" , "\t(E)" },
  { '\0', "pluto-fuse", 1, "PLuTo: fusion heuristic:\n\t\t\t\t\tmaxfuse, [smartfuse], nofuse" , "(S)" },
  { '\0', "pluto-unroll", 0, "PLuTo: unroll loops" , "\t(E)" },
  { '\0', "pluto-ufactor", 1, "PLuTo: unrolling factor [4]" , "(E)" },
  { '\0', "pluto-polyunroll", 0, "PLuTo: polyhedral unrolling" , "(B)" },
  { '\0', "pluto-prevector", 0, "PLuTo: perform prevectorization" , "(S)" },
  { '\0', "pluto-multipipe", 0, "PLuTo: multipipe" , "(E)" },
  { '\0', "pluto-rar", 0, "PLuTo: consider RAR dependences" , "\t(S)" },
  { '\0', "pluto-rar-cf", 0, "PLuTo: consider RAR dependences for \n\t\t\t\t\tcost function only" , "\t(S)" },
  { '\0', "pluto-lastwriter", 0, "PLuTo: perform lastwriter dependence\n\t\t\t\t\tsimplification" , "(E)" },
  { '\0', "pluto-scalpriv", 0, "PLuTo: perform scalar privatization" , "(S)" },
  { '\0', "pluto-bee", 0, "PLuTo: use Bee" , "\t(B)" },
  { '\0', "pluto-quiet", 0, "PLuTo: be quiet" , "\t(S)" },
  { '\0', "pluto-ft", 0, "PLuTo: CLooG ft " , "\t(S)" },
  { '\0', "pluto-lt", 0, "PLuTo: CLooG lt" , "\t(S)" },
  { '\0', "pluto-ext-candl", 0, "PLuTo: Read dependences from SCoP" , "(S)" },
  { '\0', "pluto-tile-scat", 0, "PLuTo: Perform tiling inside scatterings" , "(S)" },
  { '\0', "pluto-bounds", 1, "PLuTo: Transformation coefficients\n\t\t\t\t\tbounds [+inf]\n" , "(S)" },

  { 'n', "no-codegen", 0, "Do not generate code" , "\t(S)" },
  { '\0', "cloog-cloogf", 1, "CLooG: first level to scan [1]" , "(S)" },
  { '\0', "cloog-cloogl", 1, "CLooG: last level to scan [-1]" , "(S)" },
  { '\0', "print-cloog-file", 0, "CLooG: print input CLooG file" , "(S)" },
  { '\0', "no-past", 0, "Do not use the PAST back-end" , "\t(E)" },
  { '\0', "past-hoist-lb", 0, "Hoist loop bounds" , "(S)" },
  { '\0', "pragmatizer", 0, "Use the AST pragmatizer\n" , "\t(S)" },

  { '\0', "ptile", 0, "Use PTile for parametric tiling" , "\t(S)" },
  { '\0', "ptile-fts", 0, "Use full-tile separation in PTile" , "\t(S)" },
  { '\0', "ptile-level", 1, "Level(s) of tiling [1]" , "(E)" },
  { '\0', "evolvetile", 0, "Aaptive tiled code generation\n" , "\t(E)" },

  { '\0', "punroll", 0, "Use PAST loop unrolling" , "\t(S)" },
  { '\0', "register-tiling", 0, "PAST register tiling" , "(E)" },
  { '\0', "punroll-size", 1, "PAST unrolling size [4]" , "(S)" },
  { '\0', "vectorizer", 0, "Post-transform for vectorization" , "\t(S)" },
  { '\0', "simdizer", 0, "SIMDize inner vectorizable loops" , "\t(E)" },
  { '\0', "codegen-timercode", 0, "Codegen: insert timer code" , "(E)" },
  { '\0', "codegen-timer-asm", 0, "Codegen: insert ASM timer code" , "(E)" },
  { '\0', "codegen-timer-papi", 0, "Codegen: insert PAPI timer code" , "(E)" },
  { '\0', "codegen-dinero", 0, "Codegen: insert DineroIV trace code\n" , "(S)" },
  { 'c', "compile", 0, "Compile program with C compiler" , "\t(S)" },
  { '\0', "compile-cmd", 1, "Compilation command [gcc -O3 -lm]" , "(S)" },
  { '\0', "run-cmd-args", 1, "Program execution arguments []" , "(S)" },
  { '\0', "prog-timeout", 1, "Timeout for compilation and execution,\n\t\t\t\t\tin second\n", "(E)" },

  { 's', "ponos", 0, "Optimize with Ponos" , "\t(E)" },
  { '\0', "ponos-quiet", 0, "Ponos: be quiet" , "\t(E)" },
  { '\0', "ponos-debug", 0, "Ponos: debug mode" , "\t(E)" },
  { '\0', "ponos-sched-sz", 1, "Ponos: schedule dimensionality [1]" , "(E)" },
  { '\0', "ponos-coef-N", 0, "Ponos: all variables are >= 0" , "\t(E)" },
  { '\0', "ponos-build-2dp1", 0, "Ponos: build 2d+1 schedule" , "(E)" },
  { '\0', "ponos-solver", 1, "Ponos: solver: pip,cplex,cplex-incr" , "(E)" },
  { '\0', "ponos-solver-pre", 0, "Ponos: precondition the PIP" , "(E)" },
  { '\0', "ponos-farkas-max", 0, "Ponos: maxscale FM solver" , "(E)" },
  { '\0', "ponos-farkas-nored", 0, "Ponos: remove redundancy with FM" , "(E)" },
  { '\0', "ponos-K", 1, "Ponos: value for the K constant [10]" , "(E)" },
  { '\0', "ponos-coef", 1, "Ponos: schedule coefficients bound [10]" , "(E)" },
  { '\0', "ponos-obj", 1, "Ponos: objective constraints [none],\n\t\t\t\t\tcodelet,pluto" , "(E)" },
  { '\0', "ponos-obj-list", 1, "Ponos: constraints/objectives list from\n\t\t\t\t\tsumiterpos,paramcoef0,maxouterpar,\n\t\t\t\t\tmaxinnerpar,maxperm,mindepdist,\n\t\t\t\t\tmaxdepsolve,linearind,gammapos,\n\t\t\t\t\ttasched,minitercoef" , "(E)" },
  { '\0', "ponos-pipsolve-lp", 0, "Ponos: Relax PIP to rational" , "(E)" },
  { '\0', "ponos-pip-gmp", 0, "Ponos: use PIP-gmp\n" , "(E)" },

  { '\0', "past-super-hoist", 0, "Hoist loop bounds (super aggresive)" , "(E)" },
  { '\0', "read-cloog", 0, "read cloog file as input\n" , "\t(E)" },

  { '\0', "psimdkzer", 0, "Generate vector codelets" , "\t(E)" },
  { '\0', "pk-target", 1, "PSimdKzer code generator target:\n\t\t\t\t\tprevector,spiral,naive" , "(E)" },
  { '\0', "pk-vector-isa", 1, "PSimdKzer vector ISA:\n\t\t\t\t\tSSE,AVX" , "(E)" },
  { '\0', "pk-datatype", 1, "PSimdKzer scalar datatype:\n\t\t\t\t\tsingle,double" , "(E)" },
  { '\0', "pk-reskew", 1, "PSimdKzer reskew PLuTo schedule" , "(E)" },
  { '\0', "pk-gen-build-script", 0, "PSimdKzer: create Polybench script\n" , "(E)" },
  { '\0', "simd-width", 1, "SIMD vector width in bytes" , "(S)" },
  { '\0', "simd-fma", 0, "SIMD ISA has FMA support" , "(S)" },
  { '\0', "scalar-size", 1, "Scalar element size in bytes" , "(S)" },
  { '\0', "num-omp-threads", 1, "Number of OpenMP threads (1 per core)" , "(S)" },

  { '\0', "dump-approxscop", 0, "Dump over-approximated SCoP file of the\n\t\t\t\t\tgenerated AST\n" , "(E)" },
  { '\0', "output-aster", 0, "Output transfo as .aster file\n" , "\t(E)" },
  { 'i', "interpreter", 0, "Enter PoCC interpreter\n" , "\t(E)" },
  { '\0', "ponos-chunked", 0, "Ponos: chunked codelet mode" , "(E)" },
  { '\0', "ponos-chunked-max-fusion", 0, "Ponos: chunked codelet max-fusion mode" , "(E)" },
  { '\0', "ponos-chunked-min-fusion", 0, "Ponos: chunked codelet min-fusion mode" , "(E)" },
  { '\0', "ponos-chunked-loop-max-stmt", 1, "Ponos: chunked codelet max stmts / loop" , "(E)" },
  { '\0', "ponos-chunked-loop-max-refs", 1, "Ponos: chunked codelet max refs / loop" , "(E)" },
  { '\0', "ponos-chunked-loop-max-lat", 1, "Ponos: chunked codelet max latency / loop" , "(E)" },
  { '\0', "ponos-chunked-arch", 1, "Ponos: chunked architecture (KNL,SKX,PWR9)" , "(E)" },
  { '\0', "ponos-chunked-arch-file", 1, "Ponos: chunked architecture spec filename" , "(E)" },
  { '\0', "ponos-chunked-fp-precision", 1, "Ponos: chunked architecture FP precision (bits)" , "(E)" },
  { '\0', "ponos-chunked-auto", 0, "Ponos: chunked auto-select objective from vocabulary" , "(E)" },
  { '\0', "ponos-chunked-read-olist", 0, "Ponos: chunked read objective list" , "(E)" },
  { '\0', "ponos-pipes-df-program", 0, "Ponos: generate .pipes file" , "(E)" },
  { '\0', "ponos-pipes-c-program", 0, "Ponos: generate dsa-fied .C program" , "(E)" },
  { '\0', "ponos-pipes-c-harness", 0, "Ponos: generate complete .C test harness" , "(E)" },
  { '\0', "ponos-pipes-schedule-t", 0, "Ponos: generate PIPES t-schedule" , "(E)" },
  { '\0', "ponos-pipes-schedule-p", 0, "Ponos: generate PIPES p-schedule" , "(E)" },
  { '\0', "ponos-mo-space", 0, "Ponos: chunked auto-select objective from vocabulary (no codegen)" , "(E)" },
  { '\0', "ponos-chunked-unroll", 0, "Ponos: enable MDT unrolling (chunked-driven)" , "(E)" },
  { '\0', "ponos-chunked-annotate-unroll", 0, "Ponos: inform of unrolling (chunked-driven)" , "(E)" },
  { '\0', "ponos-chunked-adaptive", 0, "Ponos: chunked adaptive objective selection" , "(E)" },
  { '\0', "ponos-mdt-db-file", 1, "Ponos: chunked-mdt database complete filename " , "(E)" }
};
#endif


static const struct s_option    option =
{
  opts,
  {"Unknown argument: -", "unknown argument: --",
    "Expected argument for option: "},
  POCC_NB_OPTS,
  1
};


static void     print_version (void)
{
  printf ("PoCC, the Polyhedral Compiler Collection, version "
      PACKAGE_VERSION ".\n\n");
  printf ("Written by Louis-Noel Pouchet <" PACKAGE_BUGREPORT ">\n");
  printf ("Major contributions by Cedric Bastoul and Uday Bondhugula.\n\n");
  printf("PoCC packages several Free Software:\n");
  printf ("* Clan \t\thttp://www.lri.fr/~bastoul/development/clan\n");
  printf ("* Candl \thttp://www.lri.fr/~bastoul/development/candl\n");
  printf ("* LetSee \thttp://www-rocq.inria.fr/~pouchet/software/letsee\n");
  printf ("* PLuTo \thttp://www.cse.ohio-state.edu/~bondhugu/pluto\n");
  printf ("* CLooG \thttp://www.cloog.org\n");
  printf ("* PIPLib \thttp://www.piplib.org\n");
  printf ("* PolyLib \thttp://icps.u-strasbg.fr/polylib\n");
  printf ("* FM \t\thttp://www-rocq.inria.fr/~pouchet/software/fm\n");
  exit (1);
}


static void     print_help (void)
{
  int           i;

  printf ("PoCC, the Polyhedral Compiler Collection, version "
      PACKAGE_VERSION ".\n\n");
  printf ("Written by Louis-Noel Pouchet <" PACKAGE_BUGREPORT ">\n");
  printf ("Major contributions by Cedric Bastoul (Clan, Candl, CLooG), Uday Bondhugula (Pluto), ");
  printf ("and Sven Verdoolaege (ISL).\n\n");

  printf("Available options for PoCC are: \n");
  printf("(S) -> stable option, should work as expected\n");
  printf("(E) -> experimental option, use at your own risk\n");
  printf("(B) -> broken option, will likely not work\n\n");
  printf("\n");
  printf("short\t\tlong\t\tstatus\t\tdescription\n\n");

  for (i = 0; i < POCC_NB_OPTS; ++i)
    if (opts[i].short_opt != '\0')
    {
      if (opts[i].expect_arg)
        printf ("-%c\t--%s <arg> \t%s\t%s\n",
            opts[i].short_opt,
            opts[i].long_opt,
            opts[i].status,
            opts[i].description);
      else
        printf ("-%c\t--%s \t%s\t%s\n",
            opts[i].short_opt,
            opts[i].long_opt,
            opts[i].status,
            opts[i].description);
    }
    else
    {
      if (opts[i].expect_arg)
        printf ("  \t--%s <arg> \t%s\t%s\n",
            opts[i].long_opt,
            opts[i].status,
            opts[i].description);
      else
        printf ("  \t--%s \t%s\t%s\n",
            opts[i].long_opt,
            opts[i].status,
            opts[i].description);
    }
  exit (1);
}


  void
pocc_usage ()
{
  fprintf (stderr, "Usage: pocc [options] source-file\n");
  exit (1);
}


  int
pocc_getopts (s_pocc_options_t* options, int argc, char** argv)
{
  char          **opt_tab;
  int		ret;
  unsigned	i;

  opt_tab = malloc (sizeof (char *) * POCC_NB_OPTS);
  for (i = 0; i < POCC_NB_OPTS; ++i)
    opt_tab[i] = NULL;
  ret = get_cmdline_opts (&option, 1, argc, argv, opt_tab);

  // Help.
  if (opt_tab[POCC_OPT_HELP])
    print_help ();
  // Version.
  if (opt_tab[POCC_OPT_VERSION])
    print_version ();

  if (opt_tab[POCC_OPT_INTERPRETER])
  {
    options->pocc_interpreter = 1;
    return 1;
  }

  // Input file.
  if (ret)
  {
    options->input_file = fopen (argv[ret], "r");
    if (options->input_file == NULL)
    {
      fprintf (stderr, "[PoCC] Error: Unable to open file %s\n",
          argv[ret]);
      pocc_usage ();
    }
    options->input_file_name = strdup (argv[ret]);
    if (ret < argc)
      ret = get_cmdline_opts (&option, ret + 1, argc, argv, opt_tab);
  }

  if (ret != argc)
    pocc_usage ();
  // Output file.
  if (opt_tab[POCC_OPT_OUTFILE])
    options->output_file_name = strdup (opt_tab[POCC_OPT_OUTFILE]);
  else
  {
    char buffer[2048];
    strcpy (buffer, options->input_file_name);
    int ii = strlen (buffer) - 1;
    while (ii >= 0 && buffer[ii--] != '.')
      ;
    buffer[++ii] = '\0';
    strcat (buffer, ".pocc.c");
    options->output_file_name = strdup (buffer);
  }
  if (opt_tab[POCC_OPT_OUTFILE_SCOP])
  {
    options->output_scoplib_file_name =
      XMALLOC(char, strlen(options->output_file_name) + 6);
    strcpy (options->output_scoplib_file_name,
        options->output_file_name);
    strcat (options->output_scoplib_file_name, ".scop");
  }

  // Help, again.
  if (opt_tab[POCC_OPT_HELP])
    print_help ();
  // Version.
  if (opt_tab[POCC_OPT_VERSION])
    print_version ();

  // Parser options.
  if (opt_tab[POCC_OPT_BOUNDED_CTXT])
    options->clan_bounded_context = 1;
  if (opt_tab[POCC_OPT_DEFAULT_CTXT])
    options->set_default_parameter_values = 1;

  // Load parameter values from command line.
  if (opt_tab[POCC_OPT_USER_CTXT])
  {
    int len = strlen (opt_tab[POCC_OPT_USER_CTXT]);
    if (len)
    {
      char buffer[len + 1];
      char* tmp = &(buffer[0]);
      strcpy(buffer, opt_tab[POCC_OPT_USER_CTXT]);
      int num = 1;
      int ii;
      for (ii = 0; buffer[ii]; ++ii)
        if (buffer[ii] == ',')
          ++num;
      options->parameter_values = XMALLOC(int, num + 1);
      ii = 0;
      while (*tmp && ii < num)
      {
        char buff[64];
        int pos = 0;
        while (pos < 63 && (*tmp >= '0' && *tmp <= '9'))
          buff[pos++] = *(tmp++);
        buff[pos] = '\0';
        options->parameter_values[ii++] = atoi(buff);
        while (*tmp && (*tmp <= '0' || *tmp >= '9'))
          ++tmp;
      }
      options->parameter_values[ii] = -1;
    }
  }
  if (opt_tab[POCC_OPT_INSCOP_FAKEARRAY])
    options->inscop_fakepoccarray = 1;
  if (opt_tab[POCC_OPT_READ_SCOP_FILE])
    options->read_input_scop_file = 1;

  if (opt_tab[POCC_OPT_CLOOGIFY_SCHED])
    options->cloogify_schedules = 1;

  // Dependence analysis.
  if (opt_tab[POCC_OPT_NO_CANDL])
    options->candl_pass = 0;
  if (opt_tab[POCC_OPT_CANDL_DEP_ISL_SIMP])
    options->candl_deps_isl_simplify = 1;
  if (opt_tab[POCC_OPT_CANDL_DEP_PRUNE_DUPS])
    options->candl_deps_prune_transcover = 1;

  // Feature extraction.
  if (opt_tab[POCC_OPT_POLYFEAT])
    options->polyfeat = 1;
  if (opt_tab[POCC_OPT_POLYFEAT_RAR])
    options->polyfeat_rar = 1;
  if (opt_tab[POCC_OPT_POLYFEAT_LEGACY])
    options->polyfeat_legacy = 1;
  if (opt_tab[POCC_OPT_POLYFEAT_LINESIZE])
    options->polyfeat_linesize = atoi (opt_tab[POCC_OPT_POLYFEAT_LINESIZE]);
  if (opt_tab[POCC_OPT_POLYFEAT_CACHESIZE])
    options->polyfeat_cachesize = atoi (opt_tab[POCC_OPT_POLYFEAT_CACHESIZE]);
  if (opt_tab[POCC_OPT_POLYFEAT_CACHE_IS_PRIV])
    options->polyfeat_cache_is_priv = 1;
  if (opt_tab[POCC_OPT_POLYFEAT_MULTI_PARAMS])
    options->polyfeat_multi_params =
      strdup (opt_tab[POCC_OPT_POLYFEAT_MULTI_PARAMS]);
  else
  {
    char buffer[1024];
    sprintf (buffer, "%d,%d,%d", options->polyfeat_linesize,
        options->polyfeat_cachesize,
        options->polyfeat_cache_is_priv);
    options->polyfeat_multi_params = strdup (buffer);
  }

  // Trash.
  if (opt_tab[POCC_OPT_TRASH])
    options->trash = 1;
  // Verbose.
  if (opt_tab[POCC_OPT_VERBOSE])
    options->verbose = 1;
  // Quiet.
  if (opt_tab[POCC_OPT_QUIET])
  {
    options->quiet = 1;
    options->pluto_quiet = 1;
    options->verbose = 0;
  }

  // LetSee options.
  if (opt_tab[POCC_OPT_LETSEE])
    options->letsee = 1;
  if (opt_tab[POCC_OPT_LETSEE_SEARCHSPACE])
  {
    if (! strcmp(opt_tab[POCC_OPT_LETSEE_SEARCHSPACE], "precut"))
      options->letsee_space = LS_TYPE_FS;
    else if (! strcmp(opt_tab[POCC_OPT_LETSEE_SEARCHSPACE], "schedule"))
      options->letsee_space = LS_TYPE_MULTI;
    options->letsee = 1;
  }
  if (opt_tab[POCC_OPT_LETSEE_TRAVERSAL])
  {
    if (! strcmp(opt_tab[POCC_OPT_LETSEE_TRAVERSAL], "exhaust"))
      options->letsee_traversal = LS_HEURISTIC_EXHAUST;
    else if (! strcmp(opt_tab[POCC_OPT_LETSEE_TRAVERSAL], "dh"))
      options->letsee_traversal = LS_HEURISTIC_DH;
    else if (! strcmp(opt_tab[POCC_OPT_LETSEE_TRAVERSAL], "random"))
      options->letsee_traversal = LS_HEURISTIC_RANDOM;
    else if (! strcmp(opt_tab[POCC_OPT_LETSEE_TRAVERSAL], "m1"))
      options->letsee_traversal = LS_HEURISTIC_M1;
    else if (! strcmp(opt_tab[POCC_OPT_LETSEE_TRAVERSAL], "skip"))
      options->letsee_traversal = LS_HEURISTIC_SKIP;
    else if (! strcmp(opt_tab[POCC_OPT_LETSEE_TRAVERSAL], "ga"))
      pocc_error("GA heuristics implementation are not publicly available\n");
    options->letsee = 1;
  }
  if (opt_tab[POCC_OPT_LETSEE_NORMSPACE])
  {
    options->letsee_normspace = 1;
    options->letsee = 1;
  }
  if (opt_tab[POCC_OPT_LETSEE_SCHEME_M1])
  {
    char buff[1024];
    int i, j, dim;
    options->letsee_traversal = LS_HEURISTIC_M1;
    options->letsee_scheme_m1 = XMALLOC(int, LS_HEURISTIC_MAX_SCHEME_SIZE);
    for (i = 0; i < LS_HEURISTIC_MAX_SCHEME_SIZE; ++i)
      options->letsee_scheme_m1[i] = LS_HEURISTIC_M1_SCHEME_NONE;
    for (dim = i = j = 0; opt_tab[POCC_OPT_LETSEE_SCHEME_M1][i]; ++dim, j = 0)
    {
      while (opt_tab[POCC_OPT_LETSEE_SCHEME_M1][i] &&
          opt_tab[POCC_OPT_LETSEE_SCHEME_M1][i] != ',')
        buff[j++] = opt_tab[POCC_OPT_LETSEE_SCHEME_M1][i++];
      buff[j] = '\0';
      if (! strcmp (buff, "i"))
        options->letsee_scheme_m1[dim] = LS_HEURISTIC_M1_SCHEME_ITER;
      else if (! strcmp (buff, "i+p"))
        options->letsee_scheme_m1[dim] = LS_HEURISTIC_M1_SCHEME_ITERPARAM;
      else if (! strcmp (buff, "i+p+c"))
        options->letsee_scheme_m1[dim] = LS_HEURISTIC_M1_SCHEME_FULL;
      else if (! strcmp (buff, "0"))
        options->letsee_scheme_m1[dim] = LS_HEURISTIC_M1_SCHEME_NONE;
      else
        print_help ();
      if (opt_tab[POCC_OPT_LETSEE_SCHEME_M1][i])
        ++i;
    }
    options->letsee = 1;
  }
  if (opt_tab[POCC_OPT_LETSEE_PRUNE_PRECUT])
    options->letsee_prune_precut = options->letsee = 1;
  if (opt_tab[POCC_OPT_LETSEE_BACKTRACK_MULTI])
    options->letsee_backtrack_multi = options->letsee = 1;
  if (opt_tab[POCC_OPT_LETSEE_RTRIES])
  {
    options->letsee_rtries = atoi (opt_tab[POCC_OPT_LETSEE_RTRIES]);
    options->letsee = 1;
  }


  // Pluto options.
  if (opt_tab[POCC_OPT_PLUTO])
    options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_PARALLEL])
    options->pluto_parallel = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_TILE])
    options->pluto_tile = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_L2TILE])
    options->pluto_l2tile = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_FUSE])
  {
    if (! strcmp(opt_tab[POCC_OPT_PLUTO_FUSE], "maxfuse"))
      options->pluto_fuse = MAXIMAL_FUSE;
    else if (! strcmp(opt_tab[POCC_OPT_PLUTO_FUSE], "smartfuse"))
      options->pluto_fuse = SMART_FUSE;
    else if (! strcmp(opt_tab[POCC_OPT_PLUTO_FUSE], "nofuse"))
      options->pluto_fuse = NO_FUSE;
    options->pluto = 1;
  }
  if (opt_tab[POCC_OPT_PLUTO_UNROLL])
    options->pluto_unroll = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_UFACTOR])
  {
    options->pluto_ufactor = atoi (opt_tab[POCC_OPT_PLUTO_UFACTOR]);
    options->pluto = 1;
  }
  if (opt_tab[POCC_OPT_PLUTO_POLYUNROLL])
    options->pluto_polyunroll = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_PREVECTOR])
    options->pluto_prevector = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_MULTIPIPE])
    options->pluto_multipipe = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_RAR])
    options->pluto_rar = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_RAR_CF])
    options->pluto_rar_cf = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_LASTWRITER])
    options->pluto_lastwriter = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_SCALPRIV])
    options->pluto_scalpriv = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_BEE])
    options->pluto_bee = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_QUIET])
    options->pluto_quiet = options->pluto = 1;
  if (opt_tab[POCC_OPT_PLUTO_FT])
  {
    options->pluto_ft = atoi (opt_tab[POCC_OPT_PLUTO_FT]);
    options->pluto = 1;
  }
  if (opt_tab[POCC_OPT_PLUTO_LT])
  {
    options->pluto_ft = atoi (opt_tab[POCC_OPT_PLUTO_LT]);
    options->pluto = 1;
  }
  if (opt_tab[POCC_OPT_PLUTO_EXTERNAL_CANDL])
  {
    options->pluto_external_candl = 1;
    options->pluto = 1;
  }
  if (opt_tab[POCC_OPT_PLUTO_TILING_IN_SCATT])
  {
    options->pluto_tiling_in_scatt = 1;
    options->pluto = 1;
  }
  if (opt_tab[POCC_OPT_PLUTO_BOUND_COEF])
  {
    options->pluto_bound_coefficients =
      atoi (opt_tab[POCC_OPT_PLUTO_BOUND_COEF]);
    options->pluto = 1;
  }
  if (opt_tab[POCC_OPT_LETSEE_BOUNDS])
  {
    char buff[16];
    char* str = opt_tab[POCC_OPT_LETSEE_BOUNDS];
    int pos = 0;
    int count = 0;
#define dirty_parse(val, check)					\
    count = 0;						\
    while (str[pos] && str[pos] != ',')			\
    buff[count++] = str[pos++];				\
    if (check && ! str[pos])					\
    pocc_error ("Missing argument in --letsee-bounds\n");	\
    buff[count] = '\0';					\
    val = atoi (buff);					\
    ++pos;
    dirty_parse(options->letsee_ilb, 1);
    dirty_parse(options->letsee_iUb, 1);
    dirty_parse(options->letsee_plb, 1);
    dirty_parse(options->letsee_pUb, 1);
    dirty_parse(options->letsee_clb, 1);
    dirty_parse(options->letsee_cUb, 0);
  }

  // Compile.
  if (options->letsee || opt_tab[POCC_OPT_COMPILE])
    options->compile_program = 1;
  // Compile command.
  if (opt_tab[POCC_OPT_COMPILE_CMD])
  {
    options->compile_command = strdup (opt_tab[POCC_OPT_COMPILE_CMD]);
    options->compile_program = 1;
  }
  else
    options->compile_command = strdup ("gcc -O3 -lm");
  // Execution command arguments.
  if (opt_tab[POCC_OPT_RUN_CMD_ARGS])
    options->execute_command_args = strdup (opt_tab[POCC_OPT_RUN_CMD_ARGS]);

  if (opt_tab[POCC_OPT_LETSEE_DRY_RUN])
  {
    options->compile_program = 0;
    options->letsee_dry_run = 1;
    options->letsee = 1;
  }
  // Timeout
  if (opt_tab[POCC_OPT_PROGRAM_TIMEOUT])
    options->timeout = atoi (opt_tab[POCC_OPT_PROGRAM_TIMEOUT]);

  // Codegen/Cloog options.
  if (opt_tab[POCC_OPT_NOCODEGEN])
  {
    options->codegen = 0;
    options->compile_program = 0;
  }
  if (opt_tab[POCC_OPT_CLOOG_F])
    options->cloog_f = atoi (opt_tab[POCC_OPT_CLOOG_F]);
  if (opt_tab[POCC_OPT_CLOOG_L])
    options->cloog_l = atoi (opt_tab[POCC_OPT_CLOOG_L]);
  if (opt_tab[POCC_OPT_CODEGEN_TIMERCODE])
    options->codegen_timercode = 1;
  if (opt_tab[POCC_OPT_CODEGEN_TIMER_ASM])
    options->codegen_timer_asm = 1;
  if (opt_tab[POCC_OPT_CODEGEN_TIMER_PAPI])
    options->codegen_timer_papi = 1;
  if (opt_tab[POCC_OPT_CODEGEN_DINERO])
    options->generate_dinero_tracer = 1;
  if (opt_tab[POCC_OPT_PRINT_CLOOG_FILE])
    options->print_cloog_file = 1;

  // PAST options.
  if (opt_tab[POCC_OPT_NO_PAST])
    options->use_past = 0;
  if (opt_tab[POCC_OPT_PAST_OPTIMIZE_LOOP_BOUND])
    options->past_optimize_loop_bounds = 1;

  // Pragmatizer options.
  if (opt_tab[POCC_OPT_PRAGMATIZER])
    options->pragmatizer = 1;

  // PTile options.
  if (opt_tab[POCC_OPT_PTILE])
  {
    options->ptile = 1;
    options->use_past = 1;
  }
  if (opt_tab[POCC_OPT_PTILE_FTS])
  {
    options->ptile_fts = 1;
    options->ptile = 1;
    options->use_past = 1;
  }
  if (opt_tab[POCC_OPT_PTILE_LEVEL])
  {
    options->ptile = 1;
    options->use_past = 1;
    options->ptile_level = atoi (opt_tab[POCC_OPT_PTILE_LEVEL]);
  }
  if (opt_tab[POCC_OPT_PTILE_EVOLVETILE])
  {
    options->ptile = 1;
    options->use_past = 1;
    options->ptile_evolvetile = 1;
  }


  // PUnroller options.
  if (opt_tab[POCC_OPT_PUNROLL])
  {
    options->punroll = 1;
    options->use_past = 1;
  }
  if (opt_tab[POCC_OPT_PUNROLL_AND_JAM])
  {
    options->punroll = 1;
    options->punroll_and_jam = 1;
    options->use_past = 1;
  }
  if (opt_tab[POCC_OPT_PUNROLL_SIZE])
  {
    options->punroll = 1;
    options->punroll_size = atoi (opt_tab[POCC_OPT_PUNROLL_SIZE]);
    options->use_past = 1;
  }


  // Vectorizer options.
  if (opt_tab[POCC_OPT_VECTORIZER])
    options->vectorizer = 1;
  if (opt_tab[POCC_OPT_VECTORIZER_SIMDIZE])
    options->vectorizer_vectorize_loops = 1;

  // Ponos options.
  if (opt_tab[POCC_OPT_PONOS])
    options->ponos = 1;
  if (opt_tab[POCC_OPT_PONOS_QUIET])
    options->ponos_quiet = 1;
  if (opt_tab[POCC_OPT_PONOS_DEBUG])
    options->ponos_debug = 1;
  if (opt_tab[POCC_OPT_PONOS_SCHED_DIMENSION])
    options->ponos_schedule_dim =
      atoi (opt_tab[POCC_OPT_PONOS_SCHED_DIMENSION]);
  if (opt_tab[POCC_OPT_PONOS_COEF_ARE_POS])
    options->ponos_coef_are_pos = 1;
  if (opt_tab[POCC_OPT_PONOS_BUILD_2DP1])
    options->ponos_build_2d_plus_one = 1;
  if (opt_tab[POCC_OPT_PONOS_SOLVER_TYPE])
  {
    if (! strcmp(opt_tab[POCC_OPT_PONOS_SOLVER_TYPE], "pip"))
      options->ponos_solver_type = PONOS_SOLVER_PIP;
    if (! strcmp(opt_tab[POCC_OPT_PONOS_SOLVER_TYPE], "cplex"))
      options->ponos_solver_type = PONOS_SOLVER_CPLEX;
    if (! strcmp(opt_tab[POCC_OPT_PONOS_SOLVER_TYPE], "cplex-incr"))
      options->ponos_solver_type = PONOS_SOLVER_CPLEX_INCREMENTAL;
  }
  if (opt_tab[POCC_OPT_PONOS_OBJECTIVE])
  {
    if (! strcmp(opt_tab[POCC_OPT_PONOS_OBJECTIVE], "none"))
      options->ponos_objective = PONOS_OBJECTIVES_NONE;
    else if (! strcmp(opt_tab[POCC_OPT_PONOS_OBJECTIVE], "codelet"))
      options->ponos_objective = PONOS_OBJECTIVES_CODELET;
    else if (! strcmp(opt_tab[POCC_OPT_PONOS_OBJECTIVE], "pluto"))
      options->ponos_objective = PONOS_OBJECTIVES_PLUTO;
    else if (! strcmp(opt_tab[POCC_OPT_PONOS_OBJECTIVE], "custom"))
      options->ponos_objective = PONOS_OBJECTIVES_CUSTOM;
  }
  if (opt_tab[POCC_OPT_PONOS_MAXSCALE_SOLVER])
    options->ponos_maxscale_solver = 1;
  if (opt_tab[POCC_OPT_PONOS_PIPSOLVE_LP])
    options->ponos_pipsolve_lp = 1;
  if (opt_tab[POCC_OPT_PONOS_PIPSOLVE_GMP])
    options->ponos_pipsolve_gmp = 1;
  if (opt_tab[POCC_OPT_PONOS_NOREDUNDANCY_SOLVER])
    options->ponos_noredundancy_solver = 1;
  if (opt_tab[POCC_OPT_PONOS_SOLVER_PRECOND])
    options->ponos_solver_precond = 1;
  if (opt_tab[POCC_OPT_PONOS_LEGALITY_CONSTANT_K])
    options->ponos_legality_constant_K =
      atoi (opt_tab[POCC_OPT_PONOS_LEGALITY_CONSTANT_K]);
  if (opt_tab[POCC_OPT_PONOS_SCHED_COEF_BOUND])
    options->ponos_schedule_bound =
      atoi (opt_tab[POCC_OPT_PONOS_SCHED_COEF_BOUND]);

  int pos = 0;
  int idx = 0;
  if (opt_tab[POCC_OPT_PONOS_OBJECTIVE_LIST])
  {
    char* str = opt_tab[POCC_OPT_PONOS_OBJECTIVE_LIST];
    while (pos < strlen (opt_tab[POCC_OPT_PONOS_OBJECTIVE_LIST]))
    {
      if (! strncmp (str, "sumiterpos", strlen ("sumiterpos")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_CONSTRAINTS_SUM_ITER_POS;
        pos += strlen ("sumiterpos") + 1;
        str += strlen ("sumiterpos") + 1;
      }
      else if (! strncmp (str, "paramcoef0", strlen ("paramcoef0")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_CONSTRAINTS_PARAM_COEF_ZERO;
        pos += strlen ("paramcoef0") + 1;
        str += strlen ("paramcoef0") + 1;
      }
      else if (! strncmp (str, "maxouterpar", strlen ("maxouterpar")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_OBJECTIVES_MAX_OUTER_PAR;
        pos += strlen ("maxouterpar") + 1;
        str += strlen ("maxouterpar") + 1;
      }
      else if (! strncmp (str, "maxinnerpar", strlen ("maxinnerpar")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_OBJECTIVES_MAX_INNER_PAR;
        pos += strlen ("maxinnerpar") + 1;
        str += strlen ("maxinnerpar") + 1;
      }
      else if (! strncmp (str, "maxperm", strlen ("maxperm")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_OBJECTIVES_MAX_PERMUTABILITY;
        pos += strlen ("maxperm") + 1;
        str += strlen ("maxperm") + 1;
      }
      else if (! strncmp (str, "mindepdist", strlen ("mindepdist")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_OBJECTIVES_MIN_DEP_DISTANCE;
        pos += strlen ("mindepdist") + 1;
        str += strlen ("mindepdist") + 1;
      }
      else if (! strncmp (str, "maxdepsolve", strlen ("maxdepsolve")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_OBJECTIVES_MAX_DEP_SOLVE;
        pos += strlen ("maxdepsolve") + 1;
        str += strlen ("maxdepsolve") + 1;
      }
      else if (! strncmp (str, "linearind", strlen ("linearind")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_CONSTRAINTS_LINEAR_INDEP;
        pos += strlen ("linearind") + 1;
        str += strlen ("linearind") + 1;
      }
      else if (! strncmp (str, "gammapos", strlen ("gammapos")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_OBJECTIVES_GAMMA_POS;
        pos += strlen ("gammapos") + 1;
        str += strlen ("gammapos") + 1;
      }
      else if (! strncmp (str, "tasched", strlen ("tasched")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_OBJECTIVES_TASCHED;
        pos += strlen ("tasched") + 1;
        str += strlen ("tasched") + 1;
      }
      else if (! strncmp (str, "minitercoef", strlen ("minitercoef")))
      {
        options->ponos_objective_list[idx++] =
          PONOS_OBJECTIVES_MIN_THETA_ITER;
        pos += strlen ("minitercoef") + 1;
        str += strlen ("minitercoef") + 1;
      }
      else
      {
        printf ("[PoCC][ERROR] unsupported argument: %s\n",
            opt_tab[POCC_OPT_PONOS_OBJECTIVE_LIST]);
        exit (1);
      }
    }
  }
  options->ponos_objective_list[idx] = -1;

  // Past options.
  if (opt_tab[POCC_OPT_PAST_SUPER_OPT_LOOP_BOUND])
    options->past_super_optimize_loop_bounds = 1;

  // Past options.
  if (opt_tab[POCC_OPT_READ_CLOOG_FILE])
    options->read_cloog_file = 1;

  // PSIMDZER (PONOS + Vector Code Generation)
  if (opt_tab[POCC_OPT_PSIMDKZER])
  {
    options->psimdkzer = 1;
    options->pluto = 1;
    options->codegen = 1;
    options->use_past = 1;
    options->ptile = 1;
    options->ptile_fts = 1;
    options->pragmatizer = 1;
    options->past_optimize_loop_bounds = 1;
    // ponos options.
    options->ponos = 0;
    options->ponos_build_2d_plus_one = 1;
    options->ponos_objective = PONOS_OBJECTIVES_CODELET;
    options->ponos_noredundancy_solver = 1;
//    options->ponos_solver_precond = 1;
    options->ponos_maxscale_solver = 1;
    options->ponos_coef_are_pos = 1;
    options->ponos_quiet = 0;
  }

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_LOOP_MAX_STMT] ||
      opt_tab[POCC_OPT_PONOS_CHUNKED_LOOP_MAX_REF] ||
      opt_tab[POCC_OPT_PONOS_CHUNKED_LOOP_MAX_LAT])
  {
    opt_tab[POCC_OPT_PONOS_CHUNKED] = 1;
    opt_tab[POCC_OPT_PONOS_CHUNKED_MAX_FUSION] = 0;
    opt_tab[POCC_OPT_PONOS_CHUNKED_MIN_FUSION] = 0;
    if (opt_tab[POCC_OPT_PONOS_CHUNKED_LOOP_MAX_STMT])
      options-> ponos_chunked_loop_max_stmt = atoi(opt_tab[POCC_OPT_PONOS_CHUNKED_LOOP_MAX_STMT]);
    if (opt_tab[POCC_OPT_PONOS_CHUNKED_LOOP_MAX_REF])
      options-> ponos_chunked_loop_max_ref = atoi(opt_tab[POCC_OPT_PONOS_CHUNKED_LOOP_MAX_REF]);
    if (opt_tab[POCC_OPT_PONOS_CHUNKED_LOOP_MAX_LAT])
      options-> ponos_chunked_loop_max_lat = atoi(opt_tab[POCC_OPT_PONOS_CHUNKED_LOOP_MAX_LAT]);
  }

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_MAX_FUSION])
  {
    opt_tab[POCC_OPT_PONOS_CHUNKED] = 1;
    options->ponos_chunked_max_fusion = 1;
    options->ponos_chunked_min_fusion = 0;
  }

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_UNROLL])
    options->ponos_chunked_unroll = 1; 

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_ANNOTATE_UNROLL])
    options->ponos_chunked_annotate_unroll = 1; 

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_ADAPTIVE_ASSEMBLY])
    options->ponos_chunked_adaptive_assembly = 1;


  if (opt_tab[POCC_OPT_PONOS_CHUNKED_MIN_FUSION])
  {
    opt_tab[POCC_OPT_PONOS_CHUNKED] = 1;
    options->ponos_chunked_min_fusion = 1;
    options->ponos_chunked_max_fusion = 0;
  }

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_ARCH])
  {
    opt_tab[POCC_OPT_PONOS_CHUNKED] = 1;
    if (! strcmp(opt_tab[POCC_OPT_PONOS_CHUNKED_ARCH], "KNL"))
      options->ponos_chunked_arch = PONOS_CHUNKED_ARCH_KNL;
    else if (! strcmp(opt_tab[POCC_OPT_PONOS_CHUNKED_ARCH], "SKX"))
      options->ponos_chunked_arch = PONOS_CHUNKED_ARCH_SKX;
    else if (! strcmp(opt_tab[POCC_OPT_PONOS_CHUNKED_ARCH], "PWR9"))
      options->ponos_chunked_arch = PONOS_CHUNKED_ARCH_PWR9;
    else if (! strcmp(opt_tab[POCC_OPT_PONOS_CHUNKED_ARCH], "CNC"))
      options->ponos_chunked_arch = PONOS_CHUNKED_ARCH_CNC;
    else if (! strcmp(opt_tab[POCC_OPT_PONOS_CHUNKED_ARCH], "TCNC"))
      options->ponos_chunked_arch = PONOS_CHUNKED_ARCH_TILED_CNC;
    else if (! strcmp(opt_tab[POCC_OPT_PONOS_CHUNKED_ARCH], "FAKE"))
      options->ponos_chunked_arch = PONOS_CHUNKED_ARCH_FAKE;
    else
      options->ponos_chunked_arch = PONOS_CHUNKED_ARCH_NONE;
  }

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_ARCH_FILE])
  {
    // Use architecture specification file.
    opt_tab[POCC_OPT_PONOS_CHUNKED] = 1;
    options->ponos_chunked_arch_file = strdup 
      (opt_tab[POCC_OPT_PONOS_CHUNKED_ARCH_FILE]);
    options->ponos_chunked_arch = PONOS_CHUNKED_ARCH_FILE;
  }

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_MDT_DB_FILENAME])
  {
    options->ponos_chunked_mdt_db_filename = 
      strdup(opt_tab[POCC_OPT_PONOS_CHUNKED_MDT_DB_FILENAME]);
  }

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_FP_PRECISION])
  {
    sscanf (opt_tab[POCC_OPT_PONOS_CHUNKED_FP_PRECISION],
      "%d", &options->ponos_chunked_fp_precision);
  }

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_AUTO])
  {
    options->ponos_chunked_auto = 1;
  }

  if (opt_tab[POCC_OPT_PONOS_MO_SPACE])
  {
    options->ponos_mo_space = 1;
  }

  if (opt_tab[POCC_OPT_PONOS_PIPES_T_SCHEDULE])
  {
    options->ponos_pipes_t_schedule = 1;
  }

  if (opt_tab[POCC_OPT_PONOS_PIPES_P_SCHEDULE])
  {
    options->ponos_pipes_p_schedule = 1;
  }

  if (opt_tab[POCC_OPT_PONOS_PIPES_DF_PROGRAM])
  {
    options->ponos_pipes_df_program = 1;
  }

  if (opt_tab[POCC_OPT_PONOS_PIPES_C_PROGRAM])
  {
    options->ponos_pipes_c_program = 1;
  }

  if (opt_tab[POCC_OPT_PONOS_PIPES_C_HARNESS])
  {
    options->ponos_pipes_c_harness = 1;
  }

  if (opt_tab[POCC_OPT_PONOS_CHUNKED_READ_OLIST])
  {
    options->ponos_chunked_read_olist = 1;
  }

  if (opt_tab[POCC_OPT_PONOS_CHUNKED])
  {
    /*
    options->psimdkzer = 1;
    options->pluto = 1;
    options->codegen = 1;
    options->use_past = 1;
    options->pragmatizer = 1;
    options->past_optimize_loop_bounds = 1;
    */

    options->ponos_chunked = 1;
    options->use_past = 1;
    options->pragmatizer = 1;
    options->past_optimize_loop_bounds = 1;
    // ponos options.
    options->ponos = 1;
    options->ponos_build_2d_plus_one = 1;
    options->ponos_objective = PONOS_OBJECTIVES_CHUNKED;
          options->ponos_noredundancy_solver = 1;
//    options->ponos_solver_precond = 1;
    options->ponos_maxscale_solver = 1;
    options->ponos_coef_are_pos = 1;
    options->ponos_quiet = 0;
  }


  if (opt_tab[POCC_OPT_PSIMDKZER_GEN_POLYBENCH_SCRIPT])
  {
    options->psimdkzer_gen_polybench_script = 1;
  }
  if (opt_tab[POCC_OPT_PSIMDKZER_RESKEW])
  {
    printf ("\n*******************************************************\n");
    printf ("Warning: Using PSimdKzer reskewing option. This must be\n");
    printf ("         equal to the maximum unrolling factor used in\n");
    printf ("         psimdkzer.config\n");
    printf ("\n*******************************************************\n");
    sscanf (opt_tab[POCC_OPT_PSIMDKZER_RESKEW],"%d",&options->psimdkzer_reskew);
  }
  if (opt_tab[POCC_OPT_PSIMDKZER_TARGET])
  {
    if (! strcmp(opt_tab[POCC_OPT_PSIMDKZER_TARGET], "prevector"))
      options->psimdkzer_target = PSIMDKZER_TARGET_PREVECTOR;
    if (! strcmp(opt_tab[POCC_OPT_PSIMDKZER_TARGET], "spiral"))
      options->psimdkzer_target = PSIMDKZER_TARGET_SPIRAL;
    if (! strcmp(opt_tab[POCC_OPT_PSIMDKZER_TARGET], "naive"))
      options->psimdkzer_target = PSIMDKZER_TARGET_NAIVE;
    if (! strcmp(opt_tab[POCC_OPT_PSIMDKZER_TARGET], "novec"))
      options->psimdkzer_target = PSIMDKZER_TARGET_NOVEC;
  }
  if (opt_tab[POCC_OPT_PSIMDKZER_VECTOR_ISA])
  {
    if (! strcmp(opt_tab[POCC_OPT_PSIMDKZER_VECTOR_ISA], "sse"))
      options->psimdkzer_vector_isa = PSIMDKZER_VECTOR_ISA_SSE;
    if (! strcmp(opt_tab[POCC_OPT_PSIMDKZER_VECTOR_ISA], "avx"))
      options->psimdkzer_vector_isa = PSIMDKZER_VECTOR_ISA_AVX;
  }
  if (opt_tab[POCC_OPT_PSIMDKZER_SCALAR_DATATYPE])
  {
    if (! strcmp(opt_tab[POCC_OPT_PSIMDKZER_SCALAR_DATATYPE], "single"))
      options->psimdkzer_scalar_datatype = PSIMDKZER_SCALAR_DATATYPE_FLOAT;
    if (! strcmp(opt_tab[POCC_OPT_PSIMDKZER_SCALAR_DATATYPE], "double"))
      options->psimdkzer_scalar_datatype = PSIMDKZER_SCALAR_DATATYPE_DOUBLE;
  }

  // Letsee precut mode implies Pluto mode.
  if (options->letsee && options->letsee_space == LS_TYPE_FS)
    options->pluto = 1;

  // Deal with opening the output file.
  if (! options->letsee && options->codegen)
  {
    options->output_file = fopen (options->output_file_name, "w");
    if (options->output_file == NULL)
      pocc_error ("Cannot open output file\n");
  }

  // Context (OpenMP/SIMD) options.
  if (opt_tab[POCC_OPT_SIMD_LENGTH])
    options->SIMD_vector_size_in_bytes = atoi (opt_tab[POCC_OPT_SIMD_LENGTH]);
  if (opt_tab[POCC_OPT_SIMD_FMA])
    options->SIMD_supports_fma = 1;
  if (opt_tab[POCC_OPT_ELEMENT_SIZE_IN_BYTES])
    options->element_size_in_bytes =
      atoi (opt_tab[POCC_OPT_ELEMENT_SIZE_IN_BYTES]);
  if (opt_tab[POCC_OPT_NUM_OMP_THREADS])
    options->num_omp_threads =
      atoi (opt_tab[POCC_OPT_NUM_OMP_THREADS]);


#ifdef POCC_DEVEL_MODE
  // Vectorizer (legacy).
  if (opt_tab[POCC_OPT_VECT_MARK_PAR_LOOPS])
    options->vectorizer_mark_par_loops = 1;
  if (opt_tab[POCC_OPT_VECT_NO_KEEP_OUTER_PAR_LOOPS])
    options->vectorizer_keep_outer_par_loops = 0;
  if (opt_tab[POCC_OPT_VECT_SINK_ALL_LOOPS])
    options->vectorizer_sink_all_candidates = 1;

  // Storage compaction options.
  if (opt_tab[POCC_OPT_STORCOMPACT])
    options->storage_compaction = 1;
  if (opt_tab[POCC_OPT_AC_KEEP_OUTERPAR])
    options->array_contraction_keep_outer_par_loops = 1;
  if (opt_tab[POCC_OPT_AC_KEEP_VECTORIZED])
    options->array_contraction_keep_vectorized_loops = 1;

  // CDSC-GR.
  if (opt_tab[POCC_OPT_CDSCGR])
    options->cdscgr = 1;

  // DDG-analyze.
  if (opt_tab[POCC_OPT_DDGANALYZE])
    options->ddg_analyze = 1;
  if (opt_tab[POCC_OPT_DDGANALYZE_PARAMETERIZE])
    options->ddg_analyze_parameterize_slack = atoi (opt_tab[POCC_OPT_DDGANALYZE_PARAMETERIZE]);
  if (opt_tab[POCC_OPT_DDGANALYZE_PRUNE])
    options->ddg_analyze_prune_cutoff = atoi (opt_tab[POCC_OPT_DDGANALYZE_PRUNE]);
  if (opt_tab[POCC_OPT_DDGANALYZE_STATS])
    options->ddg_analyze_stats = 1;

#endif

  // Experimental dump of an approximated scoplib of the generated AST.
  if (opt_tab[POCC_OPT_OUTPUT_APPROXAST_SCOP])
    options->dump_approx_scop_from_outputast = 1;

  // Output output_file.aster containing the transformation in ASTER format.
  if (opt_tab[POCC_OPT_ASTER_OUTPUT])
    options->aster_output = 1;


  free (opt_tab);
  return 0;
}
