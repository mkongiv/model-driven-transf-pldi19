/*
 * driver-pluto.c: this file is part of the PoCC project.
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
#if HAVE_CONFIG_H
# include <pocc-utils/config.h>
#endif

# include <pocc/driver-ponos.h>
# include <pocc/driver-cloog.h>



int
pocc_driver_ponos (scoplib_scop_p program,
		  s_pocc_options_t* poptions,
		  s_pocc_utils_options_t* puoptions)
{
  if (! poptions->quiet)
    printf ("[PoCC] Running Ponos\n");

  s_ponos_options_t* popts = ponos_options_malloc ();
  popts->debug = poptions->ponos_debug;
  popts->build_2d_plus_one = poptions->ponos_build_2d_plus_one;
  popts->maxscale_solver = poptions->ponos_maxscale_solver;
  popts->noredundancy_solver = poptions->ponos_noredundancy_solver;
  popts->legality_constant = poptions->ponos_legality_constant_K;
  popts->schedule_bound = poptions->ponos_schedule_bound;
  popts->schedule_size = poptions->ponos_schedule_dim;
  popts->solver = poptions->ponos_solver_type;
  popts->solver_precond = poptions->ponos_solver_precond;
  popts->quiet = poptions->ponos_quiet;
  popts->schedule_coefs_are_pos = poptions->ponos_coef_are_pos;
  popts->objective = poptions->ponos_objective;
  popts->pipsolve_lp = poptions->ponos_pipsolve_lp;
  popts->pipsolve_gmp = poptions->ponos_pipsolve_gmp;
  popts->output_file_template = strdup (poptions->output_file_name);
  popts->legal_space_normalization = poptions->ponos_solver_precond;

  // Ponos chunked options
	popts->chunked               = poptions->ponos_chunked;
	popts->chunked_max_fusion    = poptions->ponos_chunked_max_fusion;
	popts->chunked_min_fusion    = poptions->ponos_chunked_min_fusion;
	popts->chunked_loop_max_stmt = poptions->ponos_chunked_loop_max_stmt;
	popts->chunked_loop_max_ref  = poptions->ponos_chunked_loop_max_ref;
	popts->chunked_loop_max_lat  = poptions->ponos_chunked_loop_max_lat;
  popts->chunked_arch          = poptions->ponos_chunked_arch;
  popts->fp_precision          = poptions->ponos_chunked_fp_precision;
  popts->chunked_auto          = poptions->ponos_chunked_auto;
  popts->chunked_olist         = poptions->ponos_chunked_read_olist;
  popts->extract_pipes_t_schedule = poptions->ponos_pipes_t_schedule;
  popts->extract_pipes_p_schedule = poptions->ponos_pipes_p_schedule;
  popts->generate_pipes_df        = poptions->ponos_pipes_df_program;
  popts->generate_pipes_c         = poptions->ponos_pipes_c_program;
  popts->generate_pipes_c_harness = poptions->ponos_pipes_c_harness;
  popts->chunked_multi_obj_space = poptions->ponos_mo_space;
  popts->chunked_adaptive_assembly = poptions->ponos_chunked_adaptive_assembly;

  printf ("Made it here\n");
  if (poptions->ponos_chunked_mdt_db_filename)
    popts->mdt_db_file = strdup (poptions->ponos_chunked_mdt_db_filename);
  else
    popts->mdt_db_file = NULL;
  printf ("Made it there\n");

  if (poptions->ponos_chunked_arch_file &&
      poptions->ponos_chunked_arch == PONOS_CHUNKED_ARCH_FILE)
    popts->chunked_arch_file     = strdup (poptions->ponos_chunked_arch_file);

  
  if (popts->output_file_template)
    {
      int len = strlen (popts->output_file_template);
      if (len >= 2)
	popts->output_file_template[len - 2] = '\0';
    }
  int i;
  for (i = 0; poptions->ponos_objective_list[i] != -1; ++i)
    popts->objective_list[i] = poptions->ponos_objective_list[i];
  popts->objective_list[i] = -1;
  // Candl options: candl is called in ponos for the moment.
  /// FIXME: enable reading of external deps.
  popts->candl_deps_isl_simplify = poptions->candl_deps_isl_simplify;
  popts->candl_deps_prune_transcover = poptions->candl_deps_prune_transcover;

  //
  ponos_scheduler (program, popts);

  // Dump the scop file, if needed.
  if (poptions->output_scoplib_file_name)
    {
      scoplib_scop_p tempscop = scoplib_scop_dup (program);
      if (poptions->cloogify_schedules)
	pocc_cloogify_scop (tempscop);
      FILE* scopf = fopen (poptions->output_scoplib_file_name, "w");
      if (scopf)
	{
    if (poptions->ponos_chunked && poptions->ponos_mo_space)
	    scoplib_scop_print_dot_scop_options (scopf, tempscop, SCOPLIB_SCOP_PRINT_ARRAYSTAG);
    else
	    scoplib_scop_print_dot_scop (scopf, tempscop);
	  fclose (scopf);
	}
      scoplib_scop_free (tempscop);
    }


  ponos_options_free (popts);

  return EXIT_SUCCESS;
}
