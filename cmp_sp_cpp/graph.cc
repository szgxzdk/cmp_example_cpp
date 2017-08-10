#define global

#include <limits.h>

#include <vector>
#include <list>

#include "graph.h"

extern "C" {
  #include "func.h"
}
#include "barrier.h"

using namespace std;

const double INF = __DBL_MAX__;

bool all_done = false;

static
void
do_sp(int id);

static
bool
merge();


int tnum;
const vector<vertex> * vs;

//global result
vector<int> prevs;
vector<double> minws;
vector<bool> s;

//local result
vector<double> lminw;
vector<int> lprev, lminwi;

vector<int>
graph::single_src_sp(const graph & g, int src, int tn)
{
  tnum = tn;
  
  vs = &(g.vertices);
  
  prevs.resize(vs->size());
  minws.resize(vs->size());
  s.resize(vs->size());

  //initialization
  for (size_t i = 0; i < vs->size(); i++) {
    prevs[i] = -1;
    if ((int)i == src) {
      minws[i] = 0;
      s[i] = true;
    } else {
      minws[i] = INF;
      s[i] = false;
    }
  }

  //global iterations
  lminw.resize(tnum);
  lminwi.resize(tnum);
  lprev.resize(tnum);

  barrier_init(tnum);
  int tid = 0;
  for (long i = 1; i < tnum; i++)
    if ((tid = MTA_fork()) != 0) {
      break;
    }

  barrier_wait(tid, tnum);
  MTA_Stats(1);
  
  do_sp(tid);

  return prevs;
}

static
void
do_sp(int id)
{
  int tmp = tnum;
  if (tnum > (int)vs->size())
    tmp = vs->size();
    
  size_t left = (double)id * vs->size() / tmp;
  size_t right = (double)(id + 1) * vs->size() / tmp;
  
  if (id == tmp - 1)
    right = vs->size();
  
  //local iterations
  bool done = false;
  if (id >= tmp) {
    done = true;
  }
  while (!done) {
    double minw = INF;
    int prev, minwi;
    bool all_in_s = true;
    for (size_t j = left; j < right; j++) {
      if (s[j] == true) {
	const list<edge> & nbs = (*vs)[j].get_edges();
	for (list<edge>::const_iterator it = nbs.begin();
	     it != nbs.end();
	     it++) {
	  if (s[it->dst] == false) {
	    double w = minws[j] + it->weight;
	    if (w < minw) {
	      minw = w;
	      minwi = it->dst;
	      prev = j;
	    }
	  }
	}
      } else {
	all_in_s = false;
      }
    }

    lminw[id] = minw;
    lminwi[id] = minwi;
    lprev[id] = prev;

    if (all_in_s)
      done = true;

    barrier_wait(id, tnum);
    if (id == 0 && !merge()) {
      all_done = true;
      barrier_wait(id, tnum);
      return;
    }
    barrier_wait(id, tnum);
  }

  lminw[id] = INF;

  while (!all_done) {
    barrier_wait(id, tnum);
    if (id == 0 && !merge()) {
      all_done = true;
      barrier_wait(id, tnum);
      return;
    }
    barrier_wait(id, tnum);
  }
}

static 
bool
merge()
{
  double gminw = lminw[0];
  int gminwi = lminwi[0], gprev = lprev[0];
  for (int j = 1; j < tnum; j++)
    if (lminw[j] < gminw) {
      gminw = lminw[j];
      gminwi = lminwi[j];
      gprev = lprev[j];
    }
    
  if (gminw == INF)
    return false;

  s[gminwi] = true;
  prevs[gminwi] = gprev;
  minws[gminwi] = gminw;

  return true;
}
