/*
 * driver-codegen.c: this file is part of the PoCC project.
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

#ifndef CLOOG_INT_GMP
# define CLOOG_INT_GMP
#endif
#include <cloog/cloog.h>
#include <pocc/driver-codegen.h>
#include <pocc/driver-cloog.h>
#include <pocc/driver-clastops.h>
#include <pocc/driver-pastops.h>
#include <pocc/driver-psimdkzer.h>
#include <clasttools/clast2past.h>
#include <psimdkzer/psimdkzer.h>
#include <pocc/error.h>

#include <time.h>
#include <sys/time.h>

static
double rtclock()
{
    struct timeval Tp;
    int stat;
    stat = gettimeofday (&Tp, NULL);
    if (stat != 0)
      printf ("Error return from gettimeofday: %d", stat);
    return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}

static
void
pocc_driver_codegen_post_processing (FILE* body_file,
				     s_pocc_options_t* poptions)
{
  char* args[4];
  args[2] = args[3] = NULL;
  args[1] = ".body.c";
  if (poptions->pluto_parallel && ! poptions->pragmatizer &&
      (! poptions->use_past || poptions->ddg_analyze) && ! poptions->ptile)
    {
      args[0] = STR_POCC_ROOT_DIR "/generators/scripts/ploog";
      if (poptions->quiet)
	pocc_exec (args, POCC_EXECV_HIDE_OUTPUT);
      else
	pocc_exec (args, POCC_EXECV_SHOW_OUTPUT);
    }
  if (poptions->pluto_unroll)
    {
      // Run plann.
      args[0] = STR_POCC_ROOT_DIR "/generators/scripts/plann";
      args[2] = STR_POCC_ROOT_DIR "/generators/scripts/annotations";
      if (poptions->quiet)
	pocc_exec (args, POCC_EXECV_HIDE_OUTPUT);
      else
	pocc_exec (args, POCC_EXECV_SHOW_OUTPUT);
      args[2] = NULL;
    }
  if (poptions->pluto_prevector && ! poptions->pragmatizer &&
      ! poptions->use_past && ! poptions->ptile)
    {
      args[0] = STR_POCC_ROOT_DIR "/generators/scripts/vloog";
      if (poptions->quiet)
	pocc_exec (args, POCC_EXECV_HIDE_OUTPUT);
      else
	pocc_exec (args, POCC_EXECV_SHOW_OUTPUT);
    }
}


static
int
pocc_driver_codegen_program_finalize (s_pocc_options_t* poptions)
{
  char* args[10];
  if (poptions->inscop_fakepoccarray == 1)
    args[0] = STR_POCC_ROOT_DIR "/generators/scripts/inscop-fakearray";
  else
    args[0] = STR_POCC_ROOT_DIR "/generators/scripts/inscop";
  args[1] = poptions->input_file_name;
  args[2] = ".body.c";
  args[3] = poptions->output_file_name;
  args[4] = args[5] = args[6] = args[7] = args[8] = NULL;
  int mode = poptions->quiet ? POCC_EXECV_HIDE_OUTPUT : POCC_EXECV_SHOW_OUTPUT;
  pocc_exec (args, mode);
  if (poptions->codegen_timercode)
    {
      args[0] = STR_POCC_ROOT_DIR "/generators/scripts/timercode";
      args[1] = poptions->output_file_name;
      args[2] = "time";
      args[3] = NULL;
      pocc_exec (args, mode);
    }
  else if (poptions->codegen_timer_asm)
    {
      args[0] = STR_POCC_ROOT_DIR "/generators/scripts/timercode";
      args[1] = poptions->output_file_name;
      args[2] = "asm";
      args[3] = NULL;
      pocc_exec (args, mode);
    }

  if (poptions->pluto_parallel)
    {
      args[0] = STR_POCC_ROOT_DIR "/generators/scripts/omp";
      args[1] = poptions->output_file_name;
      args[2] = NULL;
      pocc_exec (args, mode);
    }

  if (poptions->psimdkzer && poptions->psimdkzer_gen_polybench_script)
    {
      char nfts[10];
      char visa[10];
      char dtype[10];
      if (poptions->psimdkzer_vector_isa == PSIMDKZER_VECTOR_ISA_SSE)
        sprintf (visa,"sse");
      if (poptions->psimdkzer_vector_isa == PSIMDKZER_VECTOR_ISA_AVX)
        sprintf (visa,"avx");
      if (poptions->psimdkzer_scalar_datatype == PSIMDKZER_SCALAR_DATATYPE_FLOAT)
        sprintf (dtype,"float");
      if (poptions->psimdkzer_scalar_datatype == PSIMDKZER_SCALAR_DATATYPE_DOUBLE)
        sprintf (dtype,"double");

      sprintf (nfts,"%d",poptions->psimdkzer_proc_fts);
      args[0] = STR_POCC_ROOT_DIR "/generators/scripts/psimdk";
      args[1] = poptions->output_file_name;
      args[2] = nfts;
      if (poptions->psimdkzer_target == PSIMDKZER_TARGET_NAIVE)
      {
        args[3] = visa;
      }
      else
      {
        args[3] = "N/A";
      }
      args[4] = dtype;
      args[5] = poptions->input_file_name;
      args[6] = (poptions->psimdkzer_gen_polybench_script ? "1" : "0");
      args[7] = NULL;
      pocc_exec (args, mode);
    }

  // Compile the program, if necessary.
  int compile_success = 0;
  if (poptions->compile_program)
    {
      int offset = 0;
      char buffer[8192];
      if (poptions->timeout > 0)
	{
	  args[0] = "perl";
	  args[1] = "-e";
	  args[2] = "alarm shift @ARGV; exec @ARGV";
	  sprintf (buffer, "%d", poptions->timeout);
	  args[3] = strdup (buffer);
	  offset = 4;
	}
      args[offset] = STR_POCC_ROOT_DIR "/generators/scripts/compile";
      args[offset + 1] = poptions->output_file_name;
      strcpy (buffer, poptions->compile_command);
      strcat (buffer, " -lm");
      if (poptions->codegen_timer_asm || poptions->codegen_timercode)
	strcat (buffer, " -DTIME");
      args[offset + 2] = buffer;
      args[offset + 3] = XMALLOC(char, strlen (poptions->output_file_name) + 2);
      strcpy (args[offset + 3], poptions->output_file_name);
      // Remove the .xxx extension, if any.
      int pos = strlen (args[offset + 3]) - 1;
      while (args[offset + 3][pos] != '.' && --pos)
	;
      if (pos != 0)
	args[offset + 3][pos] = '\0';
      args[offset + 4] = NULL;

      char* res = pocc_exec_string_noexit (args, mode);
      if (res != NULL)
	{
	  compile_success = 1;
	  XFREE(res);
	}
      else
	return EXIT_FAILURE;
    }

  // Run the program, if necessary.
  if (poptions->compile_program && poptions->execute_program && compile_success)
    {
      int offset = 0;
      char buffer[8192];
      if (poptions->timeout > 0)
	{
	  args[0] = "perl";
	  args[1] = "-e";
	  args[2] = "alarm shift @ARGV; exec @ARGV";
	  sprintf (buffer, "%d", poptions->timeout);
	  args[3] = strdup (buffer);
	  offset = 4;
	}
      args[offset] = STR_POCC_ROOT_DIR "/generators/scripts/execute";
      ++offset;
      int len = strlen (poptions->output_file_name) + 5;
      if (poptions->execute_command_args != NULL)
	len += strlen (poptions->execute_command_args);
      args[offset] = XMALLOC(char, len);
      strcpy (args[offset], "./");
      strcat (args[offset], poptions->output_file_name);
      int pos = strlen (args[offset]) - 1;
      while (args[offset][pos] != '.' && --pos)
	;
      if (pos != 0)
	args[offset][pos] = '\0';
      strcat (args[offset], " ");
      if (poptions->execute_command_args != NULL)
	strcat (args[offset], poptions->execute_command_args);
      args[offset + 1] = NULL;
      if (! poptions->quiet)
	{
	  if (poptions->timeout == 0)
	    printf ("[PoCC] Running program %s\n", args[offset]);
	  else
	    printf ("[PoCC] Running program %s (with timeout of %ds)\n",
		     args[offset], poptions->timeout);
	}
      poptions->program_exec_result =
	pocc_exec_string_noexit (args, POCC_EXECV_HIDE_OUTPUT);
      if (poptions->program_exec_result == NULL)
	{
	  if (! poptions->quiet)
	    {
	      if (poptions->timeout == 0)
		printf ("[PoCC] Program %s aborted\n", args[offset]);
	      else
		printf ("[PoCC] Program %s aborted (timeout of %ds)\n",
			args[offset], poptions->timeout);
	    }
	  return EXIT_FAILURE;
	}
    }

  return EXIT_SUCCESS;
}


/**
 *  Generate code for transformed scop.
 *
 * (1) Create fake tile iterators inside the .scop: polyhedral
 *     tiling performed by Pluto does not update the iterators list.
 * (2) Generate statement macros.
 * (3) Convert the .scop to CloogProgram structure.
 * (4) Generate declaration for the new iterators.
 * (5) Generate polyhedral scanning code with CLooG algorithm
 * (6) Call pocc_driver_clastops, to generate AST-based code and
 *     pretty-print it.
 * (7) Final post-processing using PoCC internal scripts (timer
 *     code, unrolling, etc.) and full code generation.
 *
 */
void
pocc_driver_codegen (scoplib_scop_p program,
		     s_pocc_options_t* poptions,
		     s_pocc_utils_options_t* puoptions)
{
  if (! poptions->quiet)
    printf ("[PoCC] Starting Codegen\n");
  /* Backup the default output file. */
  FILE* out_file = poptions->output_file;
  FILE* body_file = fopen (".body.c", "w");
  if (body_file == NULL)
    pocc_error ("Cannot create file .body.c\n");
  poptions->output_file = body_file;

  

  /* (0) Reskew pluto schedule if requested by PSimdKzer module */
  if (poptions->psimdkzer && poptions->psimdkzer_reskew)
  {
    pocc_driver_psimdkzer_schedule_reskew (program,poptions,puoptions);
  }

  /* (1) Update statement iterators with tile iterators, if needed. */
  scoplib_statement_p stm;
  int i;
  for (stm = program->statement; stm; stm = stm->next)
    {
      int nb_it = stm->domain->elt->NbColumns - program->context->NbColumns;

      if (stm->nb_iterators != nb_it)
	{
	  char** iters = XMALLOC(char*, nb_it);
	  for (i = 0; i < nb_it - stm->nb_iterators; ++i)
	    {
	      iters[i] = XMALLOC(char, 16);
	      sprintf (iters[i], "fk%d", i);
	    }
	  for (; i < nb_it; ++i)
	    iters[i] = stm->iterators[i - nb_it + stm->nb_iterators];
	  XFREE(stm->iterators);
	  stm->iterators = iters;
	  stm->nb_iterators = nb_it;
	}
    }

  double start_time = rtclock ();

  /* (2) Generate polyhedral scanning code with CLooG. */
  if (! poptions->quiet)
    printf ("[PoCC] Running CLooG\n");
  CloogOptions* coptions = poptions->cloog_options;
  if (coptions == NULL)
    {
      CloogState* cstate = cloog_state_malloc ();
      poptions->cloog_options = coptions = cloog_options_malloc (cstate);
    }
  coptions->language = 'c';

  if (poptions->cloog_f != POCC_CLOOG_UNDEF)
    coptions->f = poptions->cloog_f;
  if (poptions->cloog_l != POCC_CLOOG_UNDEF)
    coptions->l = poptions->cloog_l;

  struct clast_stmt* root =
    pocc_driver_cloog (program, coptions, poptions, puoptions);

  /* (3) Call Clast modules (and pretty-print if required). */
  pocc_driver_clastops (program, root, poptions, puoptions);

  /* (4) Call PAST modules (and pretty-print if required) */
  if (poptions->use_past)
    {
      // Convert to PAST IR.
      if (! poptions->quiet)
	printf ("[PAST] Converting CLAST to PoCC AST\n");
      s_past_node_t* pastroot = clast2past (root, 1);
      pocc_driver_pastops (program, pastroot, poptions, puoptions);
      poptions->output_past_tree = pastroot;
    }

  if (poptions->past_pretty_print)
    {
      /* Perform PoCC-specific syntactic post-processing. */
      fclose (poptions->output_file);
      pocc_driver_codegen_post_processing (body_file, poptions);
      
      /* (5) Build the final output file template. */
      if (pocc_driver_codegen_program_finalize (poptions) == EXIT_FAILURE)
	{
	  if (! poptions->quiet)
	    printf ("[PoCC] Fatal error with program %s\n",
		    poptions->output_file_name);
	}
      else
	if (! poptions->quiet)
	  printf ("[PoCC] Output file is %s.\n", poptions->output_file_name);
      /* Restore the default output file. */
      poptions->output_file = out_file;
    }

  double stop_time = rtclock ();
  printf ("[PoCC/Codegen] time: %.4fsec\n",stop_time - start_time);
}
