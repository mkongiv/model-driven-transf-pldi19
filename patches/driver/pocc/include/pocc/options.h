/*
 * options.h: this file is part of the PoCC project.
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
#ifndef POCC_OPTIONS_H
# define POCC_OPTIONS_H

# include <stdio.h>
# include <pocc/common.h>
# ifndef CLOOG_INT_GMP
#  define CLOOG_INT_GMP
# endif
# include <cloog/cloog.h>
# include <cloog/options.h>
# include <pluto/pluto.h>
# include <letsee/options.h>


#define POCC_TIMER_CODE_ASM	1
#define POCC_TIMER_CODE_TIME	2

#define POCC_CLOOG_UNDEF	-2

struct s_pocc_options
{
  // In/Out file information
  FILE*		input_file;
  char*		input_file_name;
  FILE*		output_file;
  char*		output_file_name;
  char*		output_scoplib_file_name;
  int		cloogify_schedules;
  // Set to 1 if scop fields (eg, iterator and parameter names) are
  // standard char* strings, set to 0 if it is pointers from random AST
  // node.
  int		names_are_strings;

  // Verbose.
  int		verbose;
  int		quiet;

  // Trash.
  int		trash;

  // Parser options.
  int		clan_bounded_context;
  int		inscop_fakepoccarray;
  int		set_default_parameter_values;
  int*		parameter_values;
  int		read_input_scop_file;

  // Dependence analysis.
  int		candl_pass; // Run Candl (default: yes)
  int		candl_deps_isl_simplify;
  int		candl_deps_prune_transcover;

  // Feature extraction.
  int		polyfeat; // Run PolyFeat (default:no)
  int		polyfeat_legacy;
  int		polyfeat_rar;
  int		polyfeat_linesize;
  int		polyfeat_cachesize;
  int		polyfeat_cache_is_priv;
  int   polyfeat_ast_stats;
  char*		polyfeat_multi_params;

  // Compile command.
  int		compile_program; // Internal field
  char*		compile_command;
  int		execute_program;
  char*		execute_command_args;
  char*		program_exec_result; // Internal field

  // LetSee Options.
  int		letsee; // Run LetSee (default: no)
  int		letsee_space;
  int		letsee_traversal;
  int		letsee_normspace;
  int*		letsee_scheme_m1;
  int		letsee_prune_precut;
  int		letsee_backtrack_multi;
  int		letsee_rtries;
  int		letsee_ilb;
  int		letsee_iUb;
  int		letsee_plb;
  int		letsee_pUb;
  int		letsee_clb;
  int		letsee_cUb;
  int		letsee_dry_run;

  // Ponos options.
  int		ponos;
  int		ponos_quiet;
  int		ponos_debug;
  int		ponos_solver_type;
  int		ponos_solver_precond;
  int		ponos_coef_are_pos;
  int		ponos_build_2d_plus_one;
  int		ponos_maxscale_solver;
  int		ponos_noredundancy_solver;
  int		ponos_legality_constant_K;
  int		ponos_schedule_bound;
  int		ponos_schedule_dim;
  int		ponos_objective;
  int		ponos_objective_list[256];
  int		ponos_pipsolve_lp;
  int		ponos_pipsolve_gmp;

  // PLuTo Options.
  int		pluto; // Run PLuTo (default: no)
  int		pluto_unroll;
  int		pluto_parallel;
  int		pluto_tile;
  int		pluto_rar;
  int		pluto_fuse;
  int		pluto_polyunroll;
  int		pluto_bee;
  int		pluto_prevector;
  int		pluto_ufactor;
  int		pluto_quiet;
  int		pluto_context;
  int		pluto_ft;
  int		pluto_lt;
  int		pluto_multipipe;
  int		pluto_l2tile;
  int		pluto_lastwriter;
  int		pluto_scalpriv;
  int		pluto_external_candl;
  int		pluto_rar_cf;
  int		pluto_tiling_in_scatt;
  int		pluto_bound_coefficients;
  int   pluto_no_skewing;
  int   scop_preprocess_parameterize_slack;
  int   scop_parameterize_constants;
  int   scop_preprocess;

  // Codegen Options.
  int		codegen; // Perform codegen (default: yes)
  CloogOptions*	cloog_options;
  int		cloog_f;
  int		cloog_l;
  int		print_cloog_file;
  int		read_cloog_file;
  int		codegen_timercode;
  int		codegen_timer_asm;
  int		codegen_timer_papi;
  int		timeout;
  int		past_pretty_print;
  void*		output_past_tree;
  int		dump_approx_scop_from_outputast;
  int		generate_dinero_tracer;

  // Pragmatizer.
  int		pragmatizer;

  // PAST IR.
  int		use_past;
  int		past_optimize_loop_bounds;
  int		past_super_optimize_loop_bounds;

  // Ptile.
  int		ptile;
  int		ptile_fts;
  int		ptile_level;
  int		ptile_evolvetile;

  // Punroller.
  int		punroll;
  int		punroll_and_jam;
  int		punroll_size;
  int		nb_registers;

  // Vectorizer.
  int		vectorizer;
  int		vectorizer_mark_par_loops;
  int		vectorizer_mark_vect_loops;
  int		vectorizer_vectorize_loops;
  int		vectorizer_keep_outer_par_loops;
  int		vectorizer_sink_all_candidates;

  // Storage compaction.
  int		storage_compaction;
  int		array_contraction_keep_outer_par_loops;
  int		array_contraction_keep_vectorized_loops;

  // CLAST annotation/translation.
  int		clastannotation_pass;

  // PSimdKzer options
  int		psimdkzer;  // activate psimdkzer
  int		psimdkzer_target;
  int		psimdkzer_vector_isa;
  int		psimdkzer_scalar_datatype;
  int		psimdkzer_reskew;
  int		psimdkzer_proc_fts;
  int		psimdkzer_gen_polybench_script;

  // CDSC-GR pass.
  int		cdscgr;

  // DDG-analyze pass.
  int		ddg_analyze;
  int		ddg_analyze_parameterize_slack;
  int		ddg_analyze_prune_cutoff;
  int		ddg_analyze_stats;

  // Histencil pass.
  int		histencil;
  char*		histencil_optfilename;

  // Context (OpenMP/SIMD) profile.
  int		SIMD_vector_size_in_bytes;
  int		SIMD_supports_fma;
  int		element_size_in_bytes;
  int		num_omp_threads;

  // ASTER.
  int		aster_output;

  // Interpreter mode.
  int		pocc_interpreter;

  int  ponos_chunked;
  int  ponos_chunked_fp_precision;
  int  ponos_chunked_max_fusion;
  int  ponos_chunked_min_fusion;
  int  ponos_chunked_loop_max_stmt;
  int  ponos_chunked_loop_max_ref;
  int  ponos_chunked_loop_max_lat;
  int  ponos_chunked_arch;
  char * ponos_chunked_arch_file;
  int  ponos_chunked_auto;
  int  ponos_mo_space;
  int  ponos_chunked_read_olist;
  int  ponos_pipes_df_program;
  int  ponos_pipes_c_program;
  int  ponos_pipes_c_harness;
  int  ponos_pipes_t_schedule;
  int  ponos_pipes_p_schedule;
  int  ponos_chunked_unroll;
  int  ponos_chunked_annotate_unroll;
  int  ponos_chunked_adaptive_assembly;
  char * ponos_chunked_mdt_db_filename;
};
typedef struct s_pocc_options s_pocc_options_t;


BEGIN_C_DECLS

extern
s_pocc_options_t* pocc_options_malloc ();

extern
void pocc_options_init_cloog (s_pocc_options_t* options);

extern
void pocc_options_free (s_pocc_options_t* options);


END_C_DECLS



#endif // POCC_OPTIONS_H
