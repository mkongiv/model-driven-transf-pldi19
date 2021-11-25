/*
 * iutil.c: This file is part of the PONOS project.
 *
 * PONOS: POlyhedral aNd Optimal Schedulers
 *
 * Copyright (C) 2012-2018 Louis-Noel Pouchet
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * The complete GNU Lesser General Public Licence Notice can be found
 * as the `COPYING.LESSER' file in the root directory.
 *
 * Authors:
 * Martin Kong <martin.richard.kong@gmail.com>
 *
 */
#if HAVE_CONFIG_H
# include <ponos/config.h>
#endif

#include <ponos/common.h>
#include <ponos/space.h>
#include <ponos/objectives.h>
#include <ponos/codelet.h>
#include <ponos/chunked.h>

#include <time.h>
#include <sys/time.h>


static
void show_collected_variables (s_ponos_space_t * space, int * ids)
{
  int ii;
  for (ii = 0; ids && ids[ii] != -1; ii++)
  {
    printf ("[%d] Variable: %s\n",ids[ii],space->vars[ids[ii]]->name);
  }
  printf ("\n");
}


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
void ponos_chunked_set_var_ub (s_ponos_space_t* space, 
  int var_idx, int weight, int bound)
{
  ponos_codelet_create_ge_cst (space, var_idx, - weight, - bound);
}

static
void ponos_chunked_set_var_lb (s_ponos_space_t* space, 
  int var_idx, int weight, int bound)
{
  ponos_codelet_create_ge_cst (space, var_idx, weight, bound);
}

static
void ponos_chunked_set_var_lb_ub (s_ponos_space_t* space, 
  int var_idx, int weight, int lb, int ub)
{
  ponos_chunked_set_var_lb (space, var_idx, weight, lb);
  ponos_chunked_set_var_ub (space, var_idx, weight, ub);
  ponos_space_var_set_bounds (space->vars[var_idx], lb, ub);
}


static
void ponos_chunked_set_var_eq (s_ponos_space_t* space, 
  int var_idx, int weight, int bound)
{
  ponos_chunked_set_var_lb_ub (space, var_idx, weight, bound, bound);
}



static
int count_ids (int * ids)
{
  int ii;
  if (!ids)
    return 0;
  for (ii = 0; ids[ii] != -1; ii++);
  return ii;
}

static 
int get_stmt_number_in_cluster (int stmtid, int * filter)
{
  assert (filter);
  int ii;
  int count = 0;
  for (ii = 0; filter && filter[ii] != -1 && ii < stmtid ; ii++)
    if (filter[ii])
      count++;
  return count;
}
