#define ENABLE_LOCK 0
#define WEIGHTED 0
#define VERIFY 0
#define OPENMP 1
#include <assert.h>
#include <stdlib.h>


#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include "graph.hpp"
#include "io_util.h"
#include "parallel.h"
#include "rmat_util.h"
#include "sys/time.h"

std::vector<uint32_t> get_random_permutation_new(uint32_t num) {
  std::vector<uint32_t> perm(num);
  std::vector<uint32_t> vec(num);

  for (uint32_t i = 0; i < num; i++) vec[i] = i;

  uint32_t cnt{0};
  while (vec.size()) {
    uint32_t n = vec.size();
    srand(time(NULL));
    uint32_t idx = rand() % n;
    uint32_t val = vec[idx];
    std::swap(vec[idx], vec[n - 1]);
    vec.pop_back();
    perm[cnt++] = val;
  }
  return perm;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Invalid arguments.\n");
    return -1;
  }

  string file_name = argv[1];
  //string file_name = "/home/zpw/HME-Quartz-broadwell-master/dataset/soc-LiveJournal1.txt";
  std::string src, dest;
  // read edges as source and destination
  // int cnt = 0;
  struct timeval start, end;
  struct timezone tzp;

  // initialize graph
  uint32_t num_nodes=4847571;
  uint64_t num_edges=68993760;
  //uint32_t num_nodes=9;
  //uint64_t num_edges=30;

  pair_uint *edges =
      get_edges_from_file(file_name.c_str(), &num_edges, &num_nodes);

  Graph graph(num_nodes);
  std::vector<uint32_t> new_srcs(num_edges);
  std::vector<uint32_t> new_dests(num_edges);
  for (uint32_t i = 0; i < num_edges; i++) {
    new_srcs[i] = edges[i].x;
    new_dests[i] = edges[i].y;
    //cout<<new_srcs[i]<<" "<<new_dests[i]<<endl;
  }
  auto perm = get_random_permutation_new(num_edges);  //
  fstream output("/home/zpw/HME-Quartz-broadwell-master/XXX.txt");
  output<< "Insert edges"<<endl;
  std::cout << "Insert edges" << std::endl;
  gettimeofday(&start, &tzp);
  ofstream outfile;
  outfile.open("/home/zpw/HME-Quartz-broadwell-master/ndgraph/data.txt", ios::binary | ios::app | ios::in | ios::out);
  outfile<<tzp.tz_dsttime<<"  minute:   "<<tzp.tz_minuteswest<<"\n";
  outfile<<"miao: "<<start.tv_sec<<"  umiao:   "<<start.tv_usec<<"\n";
  std::cout << start.tv_sec << std::endl;
  std::cout << start.tv_usec << std::endl;
  std::cout<<tzp.tz_minuteswest<<std::endl;
  std::cout<<tzp.tz_dsttime<<std::endl;
  graph.add_edge_batch(new_srcs.data(), new_dests.data(), num_edges, perm);
  gettimeofday(&end, &tzp);
  outfile<<tzp.tz_dsttime<<"  minute:   "<<tzp.tz_minuteswest<<"\n";
  outfile<<"miao: "<<end.tv_sec<<"  umiao:   "<<end.tv_usec<<"\n";
  std::cout << end.tv_sec << std::endl;
  std::cout << end.tv_usec << std::endl;
  std::cout<<tzp.tz_minuteswest<<std::endl;
  std::cout<<tzp.tz_dsttime<<std::endl;
}