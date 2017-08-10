#define global

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <vector>
#include <stack>

#include "graph.h"

extern "C" {
  #include "func.h"
}
#include "barrier.h"

using namespace std;

void
print_paths(int src, vector<int> paths);

int printed_lock = 0;
bool printed = false;

int main(int argc, char ** argv)
{
  if (argc != 3) {
    printf("usage: shortest_path_parallel thread_number input_file\n");
    return 1;
  }

  FILE * fp = fopen(argv[2], "r");
  if (fp == NULL) {
    printf("can't open \'%s\'\n", argv[2]);
    return 1;
  }

  int tnum;
  tnum = atoi(argv[1]);

  graph g;
  
  int vnum;
  if (fscanf(fp, "%d\n", &vnum) < 1) {
    printf("no vertex number read\n");
    return 1;
  }
  
  for (int i = 0; i < vnum; i++)
    g.add_vertex(vertex());
  
  int src, dst;
  double weight;
  while (fscanf(fp, "%d %d %lg\n", &src, &dst, &weight) == 3)
    for (int i = 0; i < 9; i++) {
      g.add_edge(src, dst, weight);
      g.add_edge(dst, src, weight);
    }

  timeval start, end;
  gettimeofday(&start, 0);
  
  vector<int> prevs = graph::single_src_sp(g, 0, tnum);
  
  mutex_lock(&printed_lock);
  if (!printed) {
    printed = true;
    mutex_unlock(&printed_lock);

    gettimeofday(&end, 0);
    double timeused =
      1000000 * (end.tv_sec - start.tv_sec)
      + end.tv_usec - start.tv_usec;
    timeused /= 1000000;

    printf("execution time: %f seconds\n", timeused);
    fflush(stdout);
    print_paths(0, prevs);
    } else {
    mutex_unlock(&printed_lock);
  }

  barrier_wait(MTA_getthreadID(), tnum);
  
  MTA_Stats(0);

  while (MTA_wait() == -1)
    ;

  return 0;
}

void
print_paths(int src, vector<int> prevs)
{
  stack<int> path;

  for (size_t i = 0; i < prevs.size(); i++) {
    for (int j = prevs[i]; j != -1; j = prevs[j])
      path.push(j);
    while (!path.empty()) {
      printf("%d -> ", path.top());
      path.pop();
    }
    printf("%ld", i);
    if ((int) i == src)
      printf(" : source node");
    else if (prevs[i] == -1) 
      printf(" : unreachable");
    printf("\n");
    fflush(stdout);
  }
}
