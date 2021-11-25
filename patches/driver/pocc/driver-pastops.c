/*
 * driver-pastops.c: this file is part of the PoCC project.
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

#include <pocc/driver-pastops.h>
#include <past/past_api.h>
#include <past/pprint.h>
#include <irconverter/past2scop.h>
#include <candl/candl.h>
#include <past/symbols.h>
#include <pvectorizer/vectorize.h>
#include <punroller/punroll.h>
#include <ptile/pocc_driver.h>
#include <pocc/driver-psimdkzer.h>
#include <pocc/driver-ptile.h>
#ifdef POCC_DEVEL_MODE
# include <pocc/driver-cdscgr.h>
# include <psimdkzer/scop2past.h>
#endif

#ifdef POCC_HOST_MODE
/* # include <whatever is needed> */
#endif

//////////////

/**
 * Replace past_cloogstmt by actual statement sub-trees. 'scop' must
 * be the scop input to 'driver_codegen', and 'tree' must be the tree
 * produced by 'driver_codegen'. A scoplib_statement_p->usr must be
 * set to the PAST sub-tree representing this statement body.
 *
 */
s_past_node_t*
pocc_driver_pastops_rebuild_statement_bodies(scoplib_scop_p scop,
					     s_past_node_t* tree)
{
  int i;
  s_past_node_t* ret = tree;
  s_past_node_t** cstmts =
    past_collect_nodetype (ret, past_cloogstmt);
  // Ensure there's something to rebuild
  if (! cstmts || cstmts[0] == NULL)
    return tree;
  scoplib_statement_p* mapstmt =
    XMALLOC(scoplib_statement_p,
	    past_count_nodetype (ret, past_cloogstmt));
  for (i = 0; cstmts && cstmts[i]; ++i)
    {
      PAST_DECLARE_TYPED(cloogstmt, pc, cstmts[i]);
      int j;
      scoplib_statement_p stm;
      // Statements are 1-indexed.
      for (j = 1, stm = scop->statement;  j < pc->stmt_number;
	   ++j, stm = stm->next)
	;
      mapstmt[i] = stm;
    }
  for (i = 0; cstmts && cstmts[i]; ++i)
    {
      PAST_DECLARE_TYPED(cloogstmt, pc, cstmts[i]);
      if (mapstmt[i]->usr == NULL)
	continue;
      s_past_node_t* stmt = (s_past_node_t*) mapstmt[i]->usr;
      s_past_node_t* newstmt = past_clone (stmt);
      s_past_node_t** allrefs =
	past_collect_nodetype (newstmt, past_varref);
      s_past_node_t* sb;
      int ipos;
      for (sb = pc->substitutions, ipos = 0; sb;
	   sb = sb->next, ipos++)
	{
	  s_past_node_t* nxt = sb->next;
	  sb->next = NULL;
	  int j;
	  for (j = 0; allrefs && allrefs[j]; ++j)
	    {
	      PAST_DECLARE_TYPED(varref, pv, allrefs[j]);
	      char tmpbuff[32];
	      if (pv->symbol->name_str)
		{
		  if (! strcmp (pv->symbol->name_str,
				mapstmt[i]->iterators[ipos]))
		    past_replace_node (allrefs[j], past_clone (sb));
		}
	      else
		{
		  sprintf (tmpbuff, "addr_%p", pv->symbol->data);
		  if (! strcmp (tmpbuff,
				mapstmt[i]->iterators[ipos]))
		    past_replace_node (allrefs[j], past_clone (sb));
		}
	    }
	  sb->next = nxt;
	}
      XFREE(allrefs);
      newstmt->next = cstmts[i]->next;
      past_replace_node (cstmts[i], newstmt);
    }
  XFREE(mapstmt);
  XFREE(cstmts);

  return ret;
}




void
pocc_driver_tilescheduler (s_past_node_t* root,
			   scoplib_scop_p program,
			   s_pocc_options_t* poptions,
			   s_pocc_utils_options_t* puoptions)
{
  printf ("[PoCC][TileScheduler] Starting...\n");

  // 1- Get the scop representation of the tree.
  scoplib_scop_p scop =
    past2scop_control_only (root, program, 1);
  CandlOptions* coptions = candl_options_malloc ();
  coptions->scalar_privatization = poptions->pluto_scalpriv;
  //coptions->verbose = 1;
  CandlProgram* cprogram = candl_program_convert_scop (scop, NULL);
  CandlDependence* cdeps = candl_dependence (cprogram, coptions);


  scoplib_scop_print (stdout, scop);

  past_pprint (stdout, root);
  int nb_loops = past_count_nodetype (root, past_for);
  // Print DDV.
  int i;
  for (i = 0; i < nb_loops; ++i)
    {
      CandlDDV* ddv = candl_ddv_extract_in_loop(cprogram, cdeps, i);
      /* printf ("DDV for loop #%d\n", i); */
      /* candl_ddv_print (stdout, ddv); */
      candl_ddv_free (ddv);
    }


  // Be clean.
  candl_dependence_free (cdeps);
  candl_program_free (cprogram);
  candl_options_free (coptions);
  scoplib_scop_shallow_free (scop);

  printf ("[PoCC][TileScheduler] All done.\n");
}


//////////////


struct s_process_data
{
  s_past_node_t*	fornode;
  int			forid;
  int			is_parallel;
};
typedef struct s_process_data s_process_data_t;

static
void traverse_tree_index_for (s_past_node_t* node, void* data)
{
  if (past_node_is_a (node, past_for))
    {
      int i;
      s_process_data_t* pd = (s_process_data_t*) data;
      for (i = 0; pd[i].fornode != NULL; ++i)
	;
      pd[i].fornode = node;
      pd[i].forid = i;
      pd[i].is_parallel = 0;
    }
  if (past_node_is_a (node, past_cloogstmt))
    {
      // Special case: statements not surrouded by any loop in the
      // tree that are surrounded by a fake loop in the scop representation.
      s_past_node_t* parent;
      for (parent = node->parent; parent && !past_node_is_a (parent, past_for);
	   parent = parent->parent)
	;
      if (!parent)
	{
	  int i;
	  s_process_data_t* pd = (s_process_data_t*) data;
	  for (i = 0; pd[i].fornode != NULL; ++i)
	    ;
	  pd[i].fornode = node;
	  pd[i].forid = -i;
	}
    }
}


/**
 * Translate all past_for nodes which are sync-free parallel into
 * past_parfor.
 *
 */
static
void
translate_past_for (scoplib_scop_p original_scop,
		   s_past_node_t** root,
		   int data_is_char,
		   s_pocc_options_t* poptions)
{
  // It assumes annotate_past_for has been run already.

  s_past_node_t** forloops = past_collect_nodetype (*root, past_for);
  int i;
  for (i = 0; forloops && forloops[i]; ++i)
    {
      PAST_DECLARE_TYPED(for, pf, forloops[i]);
      if (pf->property == e_past_parallel_loop)
	{
	  // The loop is sync-free parallel, translate it to past_parfor.
	  s_past_node_t* pf = past_for_to_parfor (forloops[i]);
	  if (*root == forloops[i])
	    *root = pf;
	  past_set_parent (pf);
	}
    }
  XFREE(forloops);
}


/**
 * Annotate all past_for nodes with info about parallelism and
 * permutability.
 */
static
void
annotate_past_for (scoplib_scop_p original_scop,
		   s_past_node_t** root,
		   int data_is_char,
		   s_pocc_options_t* poptions)
{
  // 1- Get the scop representation of the tree.
  scoplib_scop_p scop =
    past2scop_control_only (*root, original_scop, data_is_char);
  /* scoplib_scop_print (stdout, scop); */
  /* past_pprint (stdout, *root); */
  CandlOptions* coptions = candl_options_malloc ();
  coptions->scalar_privatization = poptions->pluto_scalpriv;
  //coptions->verbose = 1;
  CandlProgram* cprogram = candl_program_convert_scop (scop, NULL);
  /* candl_program_print (stdout, cprogram); */
  CandlDependence* cdeps = candl_dependence (cprogram, coptions);
  int num_for_loops = past_count_for_loops (*root);
  int num_stmts = past_count_statements (*root);

  // Oversize the data structure, to deal with fake iterators.
  s_process_data_t prog_loops[num_for_loops + num_stmts + 1];
  int i, j;
  for (i = 0; i < num_for_loops + num_stmts + 1; ++i)
    prog_loops[i].fornode = NULL;
  past_visitor (*root, traverse_tree_index_for, (void*)prog_loops, NULL, NULL);

  // Recompute the number of actual for loops.
  for (num_for_loops = 0; prog_loops[num_for_loops].fornode; ++num_for_loops)
    ;

  // 2- Iterate on all loops.
  for (i = 0; i < num_for_loops; ++i)
    {
      // Skip fake loops.
      if (prog_loops[i].forid < 0)
	continue;
      // Skip... non-loops ;-)
      if (! past_node_is_a (prog_loops[i].fornode, past_for))
	continue;
      PAST_DECLARE_TYPED(for, pf, prog_loops[i].fornode);
      // Check if a dependence is loop-carried.
      CandlDependence* d;
      for (d = cdeps; d; d = d->next)
	if (candl_dependence_is_loop_carried (cprogram, d, i))
	  break;
      if (d == NULL)
	pf->property = e_past_parallel_loop;
      else
	{
	  // Check if all dependences are forward-only (permutable loop).
	  CandlDDV* ddv = candl_ddv_extract_in_loop(cprogram, cdeps, i);
	  CandlDDV* tmp;
	  int loop_depth = past_loop_depth ((s_past_node_t*)pf);
	  int is_perm = 1;
	  for (tmp = ddv; tmp; tmp = tmp->next)
	    {
	      if (tmp->data[loop_depth - 1].type == candl_dv_star ||
		  tmp->data[loop_depth - 1].type == candl_dv_minus ||
		  (tmp->data[loop_depth - 1].type == candl_dv_scalar &&
		   tmp->data[loop_depth - 1].value < 0))
		{
		  is_perm = 0;
		  break;
		}
	    }
	  if (is_perm)
	    pf->property = e_past_fco_loop;
	  candl_ddv_free (ddv);
	}
    }

  candl_dependence_free (cdeps);
  candl_program_free (cprogram);
  candl_options_free (coptions);
  scoplib_scop_shallow_free (scop);
  past_set_parent (*root);
}



static
void traverse_collect_iterators (s_past_node_t* node, void* data)
{
  if (past_node_is_a (node, past_for))
    {
      PAST_DECLARE_TYPED(for, pf, node);
      s_symbol_t** iters = data;
      int i;
      for (i = 0; iters[i] && !symbol_equal (iters[i], pf->iterator);
	   ++i)
	;
      if (! iters[i])
	iters[i] = pf->iterator;
    }
  else if (past_node_is_a (node, past_statement))
    {
      PAST_DECLARE_TYPED(statement, ps, node);
      // Special case of otl loops.
      if (past_node_is_a (ps->body, past_assign))
	{
	  PAST_DECLARE_TYPED(binary, pb, ps->body);
	  if (past_node_is_a (pb->lhs, past_varref))
	    {
	      PAST_DECLARE_TYPED(varref, pv, pb->lhs);
	      if (pv->symbol->name_str &&
		  ((char*)pv->symbol->name_str)[0] == 'c')
		{
		  s_symbol_t** iters = data;
		  int i;
		  for (i = 0; iters[i] &&
			 !symbol_equal (iters[i], pv->symbol); ++i)
		    ;
		  if (! iters[i])
		    iters[i] = pv->symbol;
		}
	    }
	}
    }
}

static
s_symbol_t** collect_all_loop_iterators (s_past_node_t* node)
{
  int num_fors = past_count_for_loops (node);
  s_symbol_t** iterators = XMALLOC(s_symbol_t*, num_fors + 1);
  int i;
  for (i = 0; i <= num_fors; ++i)
    iterators[i] = NULL;

  past_visitor (node, traverse_collect_iterators, iterators, NULL, NULL);

  return iterators;
}

static
void metainfoprint (s_past_node_t* node, FILE* out)
{
  if (node->metainfo)
    fprintf (out, "%s", (char*) node->metainfo);
}

static
void traverse_expr_for_tile (s_past_node_t* node, void* data)
{
  if (past_node_is_a (node, past_mul))
    {
      PAST_DECLARE_TYPED (binary, pb, node);
      if (past_node_is_a (pb->lhs, past_value) &&
	  past_node_is_a (pb->rhs, past_varref))
	{
	  PAST_DECLARE_TYPED(varref, pv, pb->rhs);
	  PAST_DECLARE_TYPED(value, pu, pb->rhs);
	  void** args = (void**)data;
	  int i;
	  s_symbol_t** outer_iters = (s_symbol_t**) args[0];
	  for (i = 0; outer_iters[i] && outer_iters[i] != pv->symbol; ++i)
	    ;
	  if (outer_iters && pu->type == e_past_value_int &&
	      pu->value.intval > 4)
	    {
	      int* ret = (int*) args[1];
	      *ret = 1;
	      args[2] = pv->symbol;
	    }
	}

    }
}


static
void traverse_mark_loop_type (s_past_node_t* node, void* data)
{
  if (past_node_is_a (node, past_for))
    {
      // If we can find an expression of the form 'x * outer_iterator' in
      // the loop bound, x > some value (say, 4) then 'outer_iterator'
      // is a tile loop and the loop is a point loop.

      // a- Collect surrounding loops.
      int count = 0;
      s_past_node_t* parent;
      for (parent = node->parent; parent; parent = parent->parent)
	if (past_node_is_a (parent, past_for))
	  ++count;
      s_symbol_t* outer_iters[count + 1];
      for (count = 0, parent = node->parent; parent; parent = parent->parent)
	if (past_node_is_a (parent, past_for))
	  {
	    PAST_DECLARE_TYPED(for, pf, parent);
	    outer_iters[count++] = pf->iterator;
	  }
      outer_iters[count] = NULL;
      int is_pt_loop = 0;
      void* data[3];
      data[0] = outer_iters;
      data[1] = &is_pt_loop;
      data[2] = NULL;
      past_visitor (node, traverse_expr_for_tile, (void*)data,
		    NULL, NULL);
      PAST_DECLARE_TYPED(for, pf, node);
      if (is_pt_loop)
	{
	  if (pf->type != e_past_fulltile_loop)
	    pf->type = e_past_point_loop;
	  for (parent = node->parent; parent; parent = parent->parent)
	    if (past_node_is_a (parent, past_for))
	      {
		PAST_DECLARE_TYPED(for, pf2, parent);
		int i;
		s_symbol_t** syms = (s_symbol_t**)data[0];
		for (i = 0; syms && syms[i]; ++i)
		  if (symbol_equal (pf2->iterator, syms[i]))
		    {
		      pf2->type = e_past_tile_loop;
		      break;
		    }
	      }
	}
    }
}

static
void traverse_collect_ploop (s_past_node_t* node, void* data)
{
  if (past_node_is_a (node, past_for))
    {
      PAST_DECLARE_TYPED(for, pf, node);
      // Collect both point loop nests and untiled loop nests.
      if (pf->type == e_past_point_loop ||
	  pf->type == e_past_fulltile_loop ||
	  pf->type == e_past_unknown_loop)
	{
	  int i;
	  s_past_node_t** args = (s_past_node_t**)data;
	  for (i = 0; args[i]; ++i)
	    ;
	  args[i] = node;
	}
    }
}

static
void traverse_mark_loop (s_past_node_t* node, void* data)
{
  if (past_node_is_a (node, past_for))
    {
      s_past_node_t** marked = (s_past_node_t**)data;
      int i;
      for (i = 0; marked[i]; ++i)
	;
      marked[i] = node;
    }
}

static
s_past_node_t**
collect_point_loop_components (s_past_node_t* root)
{
  // Collect all point loops.
  int num_loops = past_count_for_loops (root);
  s_past_node_t* ploops[num_loops + 1];
  s_past_node_t* marked_ploops[num_loops + 1];
  s_past_node_t* nests[num_loops + 1];
  int i, j;
  int num_nest;
  for (i = 0; i <= num_loops; ++i)
    ploops[i] = marked_ploops[i] = nests[i] = NULL;
  past_visitor (root, traverse_collect_ploop, ploops, NULL, NULL);

  // Inspect the point loops, extract all loop nests.
  for (i = 0; ploops[i]; ++i)
    {
      // Ensure the loop was not already marked as processed.
      for (j = 0; marked_ploops[j] && marked_ploops[j] != ploops[i]; ++j)
	;
      if (marked_ploops[j])
	continue;

      // See if it is an outer point loop.
      s_past_node_t* parent;
      for (parent = ploops[i]->parent; parent &&
	     !past_node_is_a (parent, past_for); parent = parent->parent)
	;
      if (parent)
	{
	  PAST_DECLARE_TYPED(for, pf, parent);
	  if (pf->type == e_past_point_loop)
	    continue;
	}
      for (num_nest = 0; nests[num_nest]; ++num_nest)
	;
      nests[num_nest] = ploops[i];
      // Mark all loops in the nest.
      s_past_node_t* next = ploops[i]->next;
      ploops[i]->next = NULL;
      past_visitor (ploops[i], traverse_mark_loop, marked_ploops,
		    NULL, NULL);
      ploops[i]->next = next;
    }

  s_past_node_t** ret = XMALLOC(s_past_node_t*, num_nest + 2);
  for (i = 0; i <= num_nest; ++i)
    ret[i] = nests[i];
  ret[i] = NULL;

  return ret;
}

static
void traverse_rename_expr (s_past_node_t* node, void* data)
{
  if (past_node_is_a (node, past_mul))
    {
      PAST_DECLARE_TYPED(binary, pb, node);
      if (past_node_is_a (pb->lhs, past_varref) &&
	  past_node_is_a (pb->rhs, past_varref))
	{
	  PAST_DECLARE_TYPED(varref, pv1, pb->lhs);
	  PAST_DECLARE_TYPED(varref, pv2, pb->rhs);
	  s_symbol_t* s1 = pv1->symbol;
	  s_symbol_t* s2 = pv2->symbol;
	  void** args = (void**)data;
	  s_symbol_t** iterators = args[0];
	  s_past_node_t*** map = args[1];
	  int i, j;
	  for (i = 0; iterators[i]; ++i)
	    if (symbol_equal (iterators[i], s1) ||
		symbol_equal (iterators[i], s2))
	      break;
	  if (! iterators[i])
	    {
	      char buffer[1024];
	      // Build a fake symbol.
	      if (s1->name_str)
		strcpy (buffer, (char*)s1->name_str);
	      else
		sprintf (buffer, "%p", s1->data);
	      if (s2->name_str)
		strcat (buffer, (char*)s2->name_str);
	      else
		sprintf (buffer, "%s%p", buffer, s1->data);
	      s_symbol_t* fake_symbol = symbol_add_from_char (NULL, buffer);
	      s_past_node_t* fakevar = past_node_varref_create (fake_symbol);
	      // Insert into the translation map.
	      for (j = 0; map[j][0]; ++j)
		;
	      map[j][0] = fakevar;
	      map[j][1] = node;
	    }
	}
    }
}

static
void
convert_non_affine_expr (s_past_node_t* node, s_past_node_t*** map)
{
  // Collect loop iterators in the nest.
  s_symbol_t** iterators = collect_all_loop_iterators (node);

  // All multiplications of variable expressions that do not involve a
  // loop iterator can be safely promoted to an affine expression, via
  // a fake parameter.
  // Fill-in the replacement map.
  void* args[2]; args[0] = iterators; args[1] = map;
  past_visitor (node, traverse_rename_expr, (void*)args, NULL, NULL);

  // Replace expressions.
  int i;
  for (i = 0; map[i][0]; ++i)
    past_replace_node (map[i][1], map[i][0]);

  XFREE(iterators);
}

static
scoplib_matrix_p extend_matrix (scoplib_matrix_p mat, int nb_par)
{
  scoplib_matrix_p ret =
    scoplib_matrix_malloc (mat->NbRows, mat->NbColumns + nb_par);
  int i, j;
  for (i = 0; i < mat->NbRows; ++i)
    {
      for (j = 0; j < mat->NbColumns - 1; ++j)
	SCOPVAL_assign(ret->p[i][j], mat->p[i][j]);
      SCOPVAL_assign(ret->p[i][ret->NbColumns - 1], mat->p[i][j]);
    }
  scoplib_matrix_free (mat);

  return ret;
}

static
scoplib_matrix_list_p extend_matrix_list (scoplib_matrix_list_p list,
					  int nb_par)
{
  scoplib_matrix_list_p ret = list;
  for (; list; list = list->next)
    list->elt = extend_matrix (list->elt, nb_par);

  return ret;
}

static
scoplib_scop_p create_mapped_scop (scoplib_scop_p program, s_past_node_t*** map)
{
  scoplib_scop_p newscop = scoplib_scop_dup (program);

  // Create unique list of fake parameters.
  int i, j, k;
  for (i = 0; map[i][0]; ++i)
    ;
  char* uniquefparam[i + 1];
  for (i = 0; map[i][0]; ++i)
    uniquefparam[i] = NULL;
  uniquefparam[i] = NULL;

  for (i = 0; map[i][0]; ++i)
    {
      PAST_DECLARE_TYPED(varref, pv, map[i][0]);
      for (j = 0; uniquefparam[j]; ++j)
	if (! strcmp ((char*)pv->symbol->name_str, uniquefparam[j]))
	  break;
      if (! uniquefparam[j])
	uniquefparam[j] = (char*)pv->symbol->name_str;
    }

  // Insert the parameters in the scop.
  for (j = 0; uniquefparam[j]; ++j)
    ;
  int count = j;
  char** newparameters = XMALLOC(char*, newscop->nb_parameters + count + 1);
  for (j = 0; j < newscop->nb_parameters; ++j)
    newparameters[j] = newscop->parameters[j];
  for (k = 0; uniquefparam[k]; ++k)
    newparameters[j + k] = uniquefparam[k];
  newparameters[newscop->nb_parameters + count] = NULL;
  newscop->nb_parameters += count;
  XFREE(newscop->parameters);
  newscop->parameters = newparameters;

  // Extend all the scop matrices to contain the new parameters.
  newscop->context = extend_matrix (newscop->context, count);
  scoplib_statement_p stm;
  for (stm = newscop->statement; stm; stm = stm->next)
    {
      stm->domain = extend_matrix_list (stm->domain, count);
      stm->read = extend_matrix (stm->read, count);
      stm->write = extend_matrix (stm->write, count);
      stm->schedule = extend_matrix (stm->schedule, count);
    }

  return newscop;
}


static
void restore_non_affine_expr (s_past_node_t*** map)
{
  // Replace expressions.
  int i;
  for (i = 0; map[i][0]; ++i)
    {
      past_replace_node (map[i][0], map[i][1]);
      past_deep_free (map[i][0]);
    }
}

static
void post_vectorize (s_past_node_t* root, scoplib_scop_p program,
		     s_pocc_options_t* poptions)
{
  // Set parent, just in case.
  past_set_parent (root);

  // Collect all point-loop nests.
  s_past_node_t** nests = collect_point_loop_components (root);

  // For each of them, apply vectorization.
  // Note: pvectorizer will ignore otl loops as candidates for
  // sinking.
  int i, j, k;
  for (i = 0; nests[i]; ++i)
    {
      s_past_node_t* next = nests[i]->next;
      nests[i]->next = NULL;

      // Convert non-affine expression into a fake affine one.
      int nb_mult = past_count_nodetype (nests[i], past_mul);
      s_past_node_t*** map = XMALLOC(s_past_node_t**, nb_mult + 1);
      for (j = 0; j <= nb_mult; ++j)
	{
	  map[j] = XMALLOC(s_past_node_t*, 2);
	  map[j][0] = map[j][1] = NULL;
	}
      convert_non_affine_expr (nests[i], map);

      // Update the scop with the new fake parameters.
      scoplib_scop_p newscop = create_mapped_scop (program, map);
      // Detect parallel loops and replace nodes in the loop nest.
      translate_past_for (newscop, &(nests[i]), 1, poptions);

      // Vectorize loops.
      pvectorizer_vectorize (newscop, nests[i],
			     poptions->vectorizer_keep_outer_par_loops,
			     poptions->verbose);

      // Restore non-affine expressions. Destroy the map entries.
      restore_non_affine_expr (map);

      // Be clean.
      for (j = 0; j <= nb_mult; ++j)
	XFREE(map[j]);
      XFREE(map);
      scoplib_scop_free (newscop);
      nests[i]->next = next;
    }
}

static
void
traverse_get_fulltiles (s_past_node_t* node, void* data)
{
  if (past_node_is_a (node, past_for))
    {
      PAST_DECLARE_TYPED(for, pf, node);
      if (pf->type == e_past_fulltile_loop)
	{
	  s_past_node_t*** fts = data;
	  int i = 0;
	  if (*fts == NULL)
	    {
	      *fts = XMALLOC(s_past_node_t*, 64);
	      int k;
	      for (k = 0; k < 64; ++k)
		(*fts)[k] = NULL;
	    }
	  else
	    {
	      for (; (*fts)[i]; ++i)
		;
	      if ((i + 2) % 64 == 0)
		{
		  *fts = XREALLOC(s_past_node_t*, *fts, i + 2 + 64);
		  int k;
		  for (k = i; k < i + 2 + 64; ++k)
		    (*fts)[k] = NULL;
		}
	    }
	  (*fts)[i] = node;
	}
    }
}

static
s_past_node_t**
collect_all_full_tiles (s_past_node_t* root)
{
  s_past_node_t** ret = NULL;
  past_visitor (root, traverse_get_fulltiles, &ret, NULL, NULL);

  return ret;
}


static
s_past_node_t**
get_ua_jammable_loops (scoplib_scop_p scop,
		       s_past_node_t* root)
{
  // 1. Count the number of loops.
  int num_loop = past_count_nodetype (root, past_for);
  s_past_node_t** ret = XMALLOC(s_past_node_t*, num_loop + 2);
  int i;
  for (i = 0; i < num_loop + 2; ++i)
    ret[i] = NULL;

  // 2. Collect all inner loops.
  s_past_node_t** inner = past_inner_loops (root);

  // 3. Eliminate all inner-loops that are not part of a perfect nest.
  int count = 0;
  for (i = 0; inner && inner[i]; ++i)
    {
      s_past_node_t* parent_loop = inner[i]->parent;
      while (parent_loop && ! past_node_is_a (parent_loop, past_for))
  	parent_loop = parent_loop->parent;
      if (parent_loop == NULL)
  	continue;
      s_past_node_t** tmp_in = past_inner_loops (parent_loop);
      if (!tmp_in || !tmp_in[0] || tmp_in[0] != inner[i] || tmp_in[1] != NULL)
  	continue;
      XFREE(tmp_in);
      int num_stmts_out = past_count_statements (parent_loop);
      int num_stmts_in = past_count_statements (inner[i]);
      if (num_stmts_out != num_stmts_in)
	continue;
      // Found a perfect nest. Check if it is permutable.
      PAST_DECLARE_TYPED(for, pfouter, parent_loop);
      PAST_DECLARE_TYPED(for, pfinner, inner[i]);
      if (pfouter->property == e_past_parallel_loop ||
  	  (pfouter->property == e_past_fco_loop &&
  	   (pfinner->property == e_past_fco_loop ||
	    pfinner->property == e_past_parallel_loop)))
  	{
  	  ret[count++] = parent_loop;
  	  ret[count++] = inner[i];
  	}
    }
  ret[count] = NULL;
  XFREE(inner);

  return ret;
}

#include <scoplib/scop.h>
int compute_fusion_score (scoplib_scop_p program,
		     s_past_node_t* root,
		     s_pocc_options_t* poptions)
{
  int score = 1;
  scoplib_statement_p stmt;
  int nstmt = 0;
  for (stmt = program->statement, nstmt = 0; stmt; stmt = stmt->next, nstmt++);
  
  if (nstmt == 1)
    return score;
  if (nstmt > 2)
    assert (0 && 
      "driver-pastops.c: Unexpected number of statements (compute_fusion_score)");
 
  scoplib_statement_p src = program->statement;
  scoplib_statement_p tgt = src;
  if (nstmt == 2)
    tgt = src->next;

  scoplib_matrix_p src_sched = src->schedule;
  scoplib_matrix_p tgt_sched = tgt->schedule;

  int src_n_iter = src->nb_iterators;
  int tgt_n_iter = tgt->nb_iterators;
  int max_penalty = src_n_iter;
  if (tgt_n_iter < max_penalty)
    max_penalty = tgt_n_iter;
  max_penalty = (1 << max_penalty);

  int src_n_row = src_sched->NbRows;
  int src_n_col = src_sched->NbColumns;
  int tgt_n_row = tgt_sched->NbRows;
  int tgt_n_col = tgt_sched->NbColumns;
  int ii;
  for (ii = 0; ii < src_n_row-1 && ii < tgt_n_row-1; ii+=2)
  {
    int coef_src = SCOPVAL_get_si(src_sched->p[ii][src_n_col-1]);
    int coef_tgt = SCOPVAL_get_si(tgt_sched->p[ii][tgt_n_col-1]);
    if (coef_src != coef_tgt)
      return max_penalty;
    max_penalty = max_penalty >> 1;
  }
  return score;
}

#define STRIDE_PENALTY 5
int compute_array_stride_penalty (scoplib_statement_p stmt,
  scoplib_matrix_p acc, int start_row)
{
  int ii;
  int fvd;
  int niter = stmt->nb_iterators;
  int nrow = acc->NbRows;
  int ncol = acc->NbColumns;
  scoplib_matrix_p sched = stmt->schedule;
  int nrow_sched = sched->NbRows;
  int hsi[niter]; // high stride 

  // Only consider the penalty when the statement is max-dimensional
  if (niter * 2 + 1 < nrow_sched)
    return 0;

  for (ii = 0; ii < niter; ii++)
    hsi[ii] = 0;

  // Fin
  for (fvd = start_row; 
    fvd + 1 < acc->NbRows && SCOPVAL_zero_p (acc->p[fvd+1][0]); fvd++);
  // fvd is pointing to the row storing the iterators used in the ref's FVD

  int idc = 0;
  for (ii = start_row; ii <= fvd - 1; ii++)
  {
    int cc;
    for (cc = 1; cc <= niter; cc++)
      hsi[cc-1] += SCOPVAL_get_si (acc->p[ii][cc]);
  }

  int penalty = 0;
  for (ii = 1; ii <= niter; ii++)
    penalty += STRIDE_PENALTY * hsi[ii-1] * 
      SCOPVAL_get_si(sched->p[nrow_sched-2][ii]);
  return penalty;
}

int next_array_row (scoplib_matrix_p acc, int curr_row)
{
  int ii;
  if (curr_row  + 1 >= acc->NbRows)
    return -1;
  for (ii = curr_row + 1; 
    SCOPVAL_zero_p (acc->p[ii][0]);
    ii++);
  return ii;
}

#define STRIDE_PENALTY 5
int compute_stride_score (scoplib_scop_p program,
		     s_past_node_t* root,
		     s_pocc_options_t* poptions)
{
  scoplib_statement_p stmt;
  int nstmt = 0;
  for (stmt = program->statement, nstmt = 0; stmt; stmt = stmt->next, nstmt++);
  
  if (nstmt > 2)
    assert (0 && 
      "driver-pastops.c: Unexpected number of statements (compute_stride_score)");
 
  scoplib_statement_p src = program->statement;
  scoplib_statement_p tgt = src;
  if (nstmt == 2)
    tgt = src->next;

  // TODO: Need to find the access matrices

  scoplib_matrix_p src_sched = src->schedule;
  scoplib_matrix_p tgt_sched = tgt->schedule;

  int ret = 1;
  int penalty = 0;
  int arrid;
  // Think about the cases
  if (nstmt == 2)
  {
    arrid = SCOPVAL_get_si (src->write->p[0][0]);
    if (strcmp (program->arrays[arrid-1], "_ftgt") != 0 &&
        strcmp (program->arrays[arrid-1], "_fsrc") != 0)
    {
      penalty += compute_array_stride_penalty (src, src->write, 0);
    }
    arrid = SCOPVAL_get_si (src->read->p[0][0]);
    if (strcmp (program->arrays[arrid-1], "_ftgt") != 0 &&
        strcmp (program->arrays[arrid-1], "_fsrc") != 0)
    {
      penalty += compute_array_stride_penalty (src, src->read, 0);
    }
    arrid = SCOPVAL_get_si (tgt->write->p[0][0]);
    if (strcmp (program->arrays[arrid-1], "_ftgt") != 0 &&
        strcmp (program->arrays[arrid-1], "_fsrc") != 0)
    {
      penalty += compute_array_stride_penalty (tgt, tgt->write, 0);
    }
    arrid = SCOPVAL_get_si (tgt->read->p[0][0]);
    if (strcmp (program->arrays[arrid-1], "_fsrc") != 0 &&
        strcmp (program->arrays[arrid-1], "_ftgt") != 0)
    {
      penalty += compute_array_stride_penalty (tgt, tgt->read, 0);
    }
  }
  if (nstmt == 1)
  {
    arrid = SCOPVAL_get_si (src->write->p[0][0]);
    if (strcmp (program->arrays[arrid-1], "_ftgt") != 0 &&
        strcmp (program->arrays[arrid-1], "_fsrc") != 0)
    {
      penalty += compute_array_stride_penalty (src, src->write, 0);
    }
    arrid = SCOPVAL_get_si (src->read->p[0][0]);
    if (strcmp (program->arrays[arrid-1], "_ftgt") != 0 &&
        strcmp (program->arrays[arrid-1], "_fsrc") != 0)
    {
      penalty += compute_array_stride_penalty (src, src->read, 0);
    }
    int next_row = next_array_row (src->read, 0);
    if (next_row != -1)
    {
      arrid = SCOPVAL_get_si (src->read->p[next_row][0]);
      if (strcmp (program->arrays[arrid-1], "_ftgt") != 0 &&
          strcmp (program->arrays[arrid-1], "_fsrc") != 0)
      {
        penalty += compute_array_stride_penalty (src, src->read, next_row);
      }
    }
  }

  if (penalty > 0)
    ret = penalty;
  return ret;
}

struct key_t {
  const char * filename;
  int n_param;
  int n_stmt;
  int dim_src_stmt;
  int dim_tgt_stmt;
  int rect_src_dom;  
  int rect_tgt_dom;  
  int dim_src_ref;
  int dim_tgt_ref;
  int n_shift_src_ref;  // No. of source array dimensions using a constant shift
  int n_shift_tgt_ref;  // No. of target array dimensions using a constant shift
  int skew_src_ref;// pending
  int skew_tgt_ref;// pending
  int has_dep;
  int self_dep; // 
  int dep_type; // none, flow, anti, write, read    // pending
  int dep_card; // dependence cardinality    // pending
  // Fusion scores: 1.0 = ideal, doubles for each non fused level
  int old_f_score; // pre-transform fusion score
  int new_f_score; // post-transform fusion score
  // Stride scores: 1 for stride 0 and 1; 5 for high-stride
  int old_s_score;
  int new_s_score;
  // Number of OMP parallel fors
  int old_n_parloop;
  int new_n_parloop;
  // Number of SIMD loops
  int old_n_vecloop;
  int new_n_vecloop;
  // Number of lines of code ==> code explosion
  int old_n_lines;
  int new_n_lines;
  // Unrolled ? 
  int old_b_unrollable;
  int new_b_unrollable;
  int par_benefit;
};
typedef struct key_t dep_key;

dep_key * dependence_key_alloc ()
{
  dep_key * key = NULL;
  key = (dep_key*)malloc (sizeof(dep_key));
  key->n_param = -1;
  key->n_stmt = -1;
  key->dim_src_stmt = -1;
  key->dim_tgt_stmt = -1;
  key->rect_src_dom = -1;  
  key->rect_tgt_dom = -1;  
  key->dim_src_ref = -1;
  key->dim_tgt_ref = -1;
  key->n_shift_src_ref = -1;  
  key->n_shift_tgt_ref = -1;  
  key->skew_src_ref = -1;
  key->skew_tgt_ref = -1;
  key->has_dep = -1;
  key->self_dep = -1; 
  key->dep_type = -1; 
  key->dep_card = -1; 
  key->old_f_score = 0;
  key->new_f_score = 0;
  key->old_s_score = 0;
  key->new_s_score = 0;
  key->old_n_parloop = 0;
  key->new_n_parloop = 0;
  key->old_n_vecloop = 0;
  key->new_n_vecloop = 0;
  key->old_n_lines = 0;
  key->new_n_lines = 0;
  key->old_b_unrollable = 0;
  key->new_b_unrollable = 0;
  key->par_benefit = 0;
  return key;
}

void collect_stats_from_source_code (dep_key * key)
{
  char cmd[1000];
  sprintf (cmd, "../gen-nano-tests/compute-source-score.py %s 0", key->filename);

  FILE * ff = popen (cmd, "r");

  char buf[100];
  int nf, nv, nl, bu, pb;

  fscanf (ff,"%d %d %d %d %d",&nf, &nv, &nl, &bu, &pb);

  key->old_n_parloop = nf;
  key->old_n_vecloop = nv;
  key->old_n_lines = nl;
  key->old_b_unrollable = bu;
  key->par_benefit = pb;

  pclose (ff);
  //printf ("Read : [%d,%d,%d]\n", x, y, z);
}

void store_dependence_key (FILE * fout, dep_key * key)
{
  // format of key:
  // n_param:n_stmt:
  // n_dim_src_stmt:n_dim_tgt_stmt:
  // rect_src_dom:rect_tgt_dom:          // 1 if domains are rectangular, 0 if not
  // n_dim_src_ref:n_dim_tgt_ref:
  // has_dep:self_dep:dep_type
  assert (key);
  fprintf (fout,"//key");
  fprintf (fout,"[");
  fprintf (fout, ":%d",key->n_param);
  fprintf (fout, ":%d",key->n_stmt);
  fprintf (fout,"][");
  fprintf (fout, ":%d",key->dim_src_stmt);
  fprintf (fout, ":%d",key->dim_tgt_stmt);
  fprintf (fout, ":%d",key->rect_src_dom);
  fprintf (fout, ":%d",key->rect_tgt_dom);
  fprintf (fout,"][");
  fprintf (fout, ":%d",key->dim_src_ref);
  fprintf (fout, ":%d",key->dim_tgt_ref);
  fprintf (fout, ":%d",key->n_shift_src_ref);
  fprintf (fout, ":%d",key->n_shift_tgt_ref);
  fprintf (fout, ":%d",key->skew_src_ref);
  fprintf (fout, ":%d",key->skew_tgt_ref);
  fprintf (fout,"][");
  fprintf (fout, ":%d",key->has_dep);
  fprintf (fout, ":%d",key->self_dep);
  fprintf (fout, ":%d",key->dep_type);
  fprintf (fout, ":%d",key->dep_card);
  fprintf (fout,"][");
  fprintf (fout, ":%d",key->old_f_score);
  //fprintf (fout, ":%d",key->new_f_score);
  fprintf (fout, ":%d",key->old_s_score);
  //fprintf (fout, ":%d",key->new_s_score);
  fprintf (fout, ":%d",key->old_n_parloop);
  //fprintf (fout, ":%d",key->new_n_parloop);
  fprintf (fout, ":%d",key->old_n_vecloop);
  //fprintf (fout, ":%d",key->new_n_vecloop);
  fprintf (fout, ":%d",key->old_n_lines);
  //fprintf (fout, ":%d",key->new_n_lines);
  fprintf (fout, ":%d",key->old_b_unrollable);
  //fprintf (fout, ":%d",key->new_b_unrollable);
  fprintf (fout, ":%d",key->par_benefit);
  fprintf (fout,"]");
  fprintf (fout, "\n");
}

//static
void
pocc_ponos_chunked_post_transform (scoplib_scop_p program,
		     s_past_node_t* root,
		     s_pocc_options_t* poptions)
{
  FILE * f;
  f = fopen (".chunked-unroll.txt", "r");
  int num_reg = 0;
  int outer_unroll;
  fscanf (f,"%d %d",&num_reg, &outer_unroll);
  fclose (f);

  if (num_reg > 4 && 
      poptions->ponos_chunked_unroll && !poptions->ponos_chunked_annotate_unroll)
  {
    if (! poptions->quiet)
	    printf ("[PoCC] Performing UAJ for Ponos/Chunked ...\n");
    punroll_unroll_in_place (program, root, num_reg, outer_unroll);
    printf ("Showing post-transformed chunked scop \n");
    past_pprint (stdout, root);
  }
  if (poptions->ponos_chunked_unroll && poptions->ponos_chunked_annotate_unroll)
  {
	  //printf ("[PoCC/Chunked] UNROLLING PROFITABLE; PASS DISABLED ...\n");
    dep_key * key = dependence_key_alloc ();
    const char * tempfile = "ast.c";
    FILE * fout = fopen (tempfile, "w");
    fprintf (fout,"#pragma scop\n");
    past_pprint (fout, root);
    fprintf (fout,"#pragma endscop\n");
    if (num_reg > 4)
    {
      fprintf (fout,"//[PoCC/Chunked] UNROLLING PROFITABLE; PASS DISABLED ...\n");
    }
    fclose (fout);
    key->filename = tempfile;  
    collect_stats_from_source_code (key);
    key->old_f_score = compute_fusion_score (program, root, poptions);
    key->old_s_score = compute_stride_score (program, root, poptions);
    //printf ("Fusion Score: %d\n", fs);
    //printf ("Stride Score: %d\n", ss);
    store_dependence_key (stdout, key);
    free (key);
    scoplib_scop_print (stdout, program);
  }
  //punroll_unswitch_inner_for_if (program, root);
}


/**
 * PAST post-processing and pretty-printing.
 *
 */
void
pocc_driver_pastops (scoplib_scop_p program,
		     s_past_node_t* root,
		     s_pocc_options_t* poptions,
		     s_pocc_utils_options_t* puoptions)
{
  if (! poptions->quiet)
    printf ("[PoCC] Using the PAST back-end\n");

  // Set parent, just in case.
  past_set_parent (root);

  // Mark the loop type.
  past_visitor (root, traverse_mark_loop_type, NULL, NULL, NULL);
  // Mark the loop dependences.
#ifdef POCC_DEVEL_MODE
  if (! poptions->ddg_analyze)
#endif    
  annotate_past_for (program, &root, 1, poptions);

  // Translate parallel for loops into parfor loops.
  if (poptions->pragmatizer || poptions->vectorizer ||
      poptions->vectorizer_vectorize_loops)
    {
      if (! poptions->quiet)
  	printf ("[PAST] Insert OpenMP/ICC pragmas\n");
      translate_past_for (program, &root, 1, poptions);
    }

  // Pre-vectorize.
  if (poptions->vectorizer)
    {
      if (! poptions->quiet)
	printf ("[PoCC] Move vectorizable loop(s) inward\n");
      pvectorizer_vectorize (program, root, poptions->ptile,
			     (!poptions->ptile && poptions->verbose));
    }

  // Run Polyhedral Feature Extraction.
  if (poptions->polyfeat)
    pocc_driver_polyfeat (program, root, poptions, puoptions);

  // Use PTILE, if asked.
  if (poptions->ptile)
    {
      pocc_driver_ptile (program, root, poptions, puoptions);
      // Post-vectorize.
      if (poptions->vectorizer)
	{
	  if (! poptions->quiet)
	    printf ("[PoCC] Post-vectorization: move vectorizable loop(s) inward\n");
	  post_vectorize (root, program, poptions);
	}
    }

  // Use Punroller, if asked.
  if (poptions->punroll && ! poptions->punroll_and_jam)
    {
      if (! poptions->quiet)
	printf ("[PoCC] Perform inner loop unrolling (factor=%d)\n",
		poptions->punroll_size);
      punroll (program, root, poptions->punroll_size,
	       poptions->nb_registers);
    }
  if (poptions->punroll_and_jam)
    {
      if (! poptions->quiet)
	printf ("[PoCC] Attempting unroll-and-jam\n");
      int uajfactors[] = {poptions->punroll_size, poptions->punroll_size};

      s_past_node_t** loops = get_ua_jammable_loops (program, root);
      int i;
      for (i = 0; loops && loops[i]; i += 2)
      	{
      	  if (! poptions->quiet)
      	    printf
      	      ("[PoCC] Perform unroll-and-jam (factor = %d x %d) on nest %d\n",
      	       uajfactors[0], uajfactors[1], i / 2);

      	  punroll_and_jam_loops (program, root, loops[i], loops[i+1],
      	  			 uajfactors, poptions->nb_registers);
      	}
      XFREE(loops);
    }

  // Systematically optimize the loop bounds (hoisting).
  if (poptions->past_optimize_loop_bounds)
    {
      if (! poptions->quiet)
	printf ("[PoCC] Perform aggressive loop bound optimization\n");
      past_optimize_loop_bounds (root);
    }
  else if (poptions->past_super_optimize_loop_bounds)
    {
      if (! poptions->quiet)
	printf ("[PoCC] Perform aggressive loop bound optimization\n");
      past_super_optimize_loop_bounds (root);
    }

  /* pocc_driver_tilescheduler (root, program, poptions, puoptions); */

  // Consistency test.
  if (! poptions->quiet)
    printf ("[PoCC] Perform AST consistency checks\n");
  // MK: commented this out
  //past_consistency_check (root, 1);


  // Perform PSimdKzer.
  if (poptions->psimdkzer)
    pocc_driver_psimdkzer (root, program, poptions, puoptions);


  if (poptions->ponos && poptions->ponos_chunked)
    {
      pocc_ponos_chunked_post_transform (program, root, poptions);
    }

  if (poptions->vectorizer_vectorize_loops)
    {
      if (! poptions->quiet)
	printf ("[PoCC] Perform SIMDization of inner vectorizable loops\n");
      pvectorizer_simdize (program, root,
			   poptions->element_size_in_bytes,
			   poptions->SIMD_vector_size_in_bytes,
			   poptions->SIMD_supports_fma);
    }


#ifdef POCC_DEVEL_MODE
  if (poptions->cdscgr)
    pocc_driver_cdscgr_pastops (root, program, poptions, puoptions);
#endif

  if (poptions->dump_approx_scop_from_outputast)
    {
      scoplib_scop_p appscop =
	past2scop_control_only (root, program, 1);
      char buffer[2048];
      sprintf (buffer, "%s.outputast.scop", poptions->output_file_name);
      FILE* of = fopen(buffer, "w");
      scoplib_scop_print_dot_scop (of, appscop);
      fclose (of);
      scoplib_scop_shallow_free (appscop);
    }

  if (poptions->past_pretty_print)
    {
      printf ("Past pretty print invoked ...\n");
      // Insert iterators declaration.
      s_symbol_t** iterators = collect_all_loop_iterators (root);
      int i;
      FILE* body_file = poptions->output_file;
      if (iterators[0])
	{
	  if (iterators[0]->name_str)
	    fprintf (body_file,"\t register int %s",
		     (char*) iterators[0]->name_str);
	  for (i = 1; iterators[i]; ++i)
	    if (iterators[i]->name_str)
	      fprintf (body_file,", %s", (char*)iterators[i]->name_str);
	  fprintf (body_file, ";\n\n");
	  fflush (body_file);
	}
      fprintf (body_file, "#pragma scop\n");

      // Pretty-print
      past_pprint_extended_metainfo (body_file, root, metainfoprint, NULL);

      fprintf (body_file, "#pragma endscop\n");

      // Be clean.
      past_deep_free (root);
      XFREE(iterators);
    }
}
