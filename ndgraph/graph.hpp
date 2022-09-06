/*
 * ============================================================================
 *
 *       Filename:  graph.h
 *
 *         Author:  zt
 *   Organization:
 *
 * ============================================================================
 */

#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

#include "SkipList.h"
#include "containers/pma_dynamic_graph.hpp"
#include "pma.hpp"
// #include "PMA.hpp"
// #include "partitioned_counter.h"
// #include "btree.h"
// #include "BitArray.h"
//#include "cpp-btree/btree_set.h"

#define PREFETCH 1

#if WEIGHTED
#define NUM_IN_PLACE_NEIGHBORS 14
#else
#define NUM_IN_PLACE_NEIGHBORS 11
#endif
#define MEDIUM_DEGREE (1ULL << 10)  // 2^10 ~ 1024 即为阈值m

#define LOCK_MASK (1ULL << 31)
#define UNLOCK_MASK ~(1ULL << 31)//1ULL其中 unsigned long long

// A 0 bit means the lock is free
// A 1 bit means the lock is currently acquired
static inline void lock(uint32_t *data) {
  while ((__sync_fetch_and_or(data, LOCK_MASK) & (1ULL << 31)) != 0) {
  }
}

static inline void unlock(uint32_t *data) {
  __sync_fetch_and_and(data, UNLOCK_MASK);
}

// Nvm based dynamic graph store 基于Nvm的动态图形存储
// Version1 is unweight graph
class Graph {
 public:
  typedef uint32_t vertex;
  // typedef uint32_t weight;
  typedef std::pair<vertex, vertex> edge;
  // TODO: add pma and skiplist container // already add 添加pma和skiplist容器
  typedef pma_dynamic_graph pma_container;
  typedef SkipList<uint32_t> skiplist_container;
  // Construction
  Graph(uint32_t size);
  ~Graph();

  // Add edge and delete edge

  int add_edge(const vertex s, const vertex d);
  int remove_edge(const vertex s, const vertex d);
  // add edge for a batch 添加批次的边，加入多条边
  void add_edge_batch(vertex *srcs, vertex *dests, uint32_t edge_count,
                      std::vector<uint32_t> &perm);
  // check the existence of the edge
  uint32_t is_edge(const vertex s, const vertex d);
  // get out degree of vertex v
  uint32_t degree(const vertex v) const;
  uint32_t get_num_edges(void);
  uint32_t get_num_vertices(void) const;
  // void add_pma(vertex* srcs)
  // one source vertex to one vertex_block
  // to show where to store
  typedef struct __attribute__((__packed__)) {
    uint32_t degree;  // 4bytes
    // void *pma_level_neighbors{nullptr};        // 8bytes
    void *skip_list_level_neighbors{nullptr};  // 8bytes
    vertex neighbors[NUM_IN_PLACE_NEIGHBORS];
  } vertex_block;//顶点块

  vertex_block *vertices;
  uint32_t num_vertices{0};
  pma_container *pma_container_;
  void add_inplace(vertex *srcs, vertex *dests, uint32_t idx,
                   std::vector<uint32_t> &parts, uint8_t *array_pma,
                   uint8_t *array_skip_list, uint8_t *array_skip_list_node);
  // void add_pma(vertex *srcs, vertex *dests, uint32_t idx,
  //              std::vector<uint32_t> &parts, uint8_t *array_pma);
  void add_skip_list(vertex *srcs, vertex *dests, uint32_t idx,
                     std::vector<uint32_t> &parts, uint8_t *array_skip_list);
};

Graph::Graph(uint32_t size) : num_vertices(size) {
  pma_container_ = new pma_container(size);
  vertices = (vertex_block *)calloc(size, sizeof(vertex_block));
}

Graph::~Graph() {
  // for (int i = 0; i < num_vertices; i++) {
  //   // free(vertices[i].pma_level_neighbors);
  //   free(vertices[i].skip_list_level_neighbors);
  // }
  free(vertices);
}
void Graph::add_inplace(vertex *srcs, vertex *dests, uint32_t idx,
                        std::vector<uint32_t> &parts, uint8_t *array_pma,
                        uint8_t *array_skip_list,
                        uint8_t *array_skip_list_node) {
  // add edge
  vertex s = srcs[parts[idx]];
  std::pair<uint32_t, uint32_t> range = {parts[idx], parts[idx + 1]};
  uint32_t size = range.second - range.first;
#if ENABLE_LOCK
  lock(&vertices[s].degree);
#endif
  uint32_t degree = this->degree(s);
  if (degree == 0) {  // empty vertex
    uint32_t cnt =
        size < NUM_IN_PLACE_NEIGHBORS ? size : NUM_IN_PLACE_NEIGHBORS;
    memcpy(vertices[s].neighbors, &dests[range.first], cnt * sizeof(vertex));
    if (vertices[s].skip_list_level_neighbors == nullptr &&
        degree + size <= MEDIUM_DEGREE) {  // going to pma
      for (uint32_t i = range.first + cnt; i < range.second; i++) {
        array_pma[i] = 1;
      }
      // array_pma_node[idx] = 1;
    } else {  // go to skip_list
      for (uint32_t i = range.first + cnt; i < range.second; i++) {
        array_skip_list[i] = 1;
      }
      array_skip_list_node[idx] = 1;
    }
    vertices[s].degree += cnt;
  } else {  // merge sort two list
    uint32_t in_place_limit =
        degree < NUM_IN_PLACE_NEIGHBORS ? degree : NUM_IN_PLACE_NEIGHBORS;
    vertex *new_in_place = (vertex *)calloc(degree + size, sizeof(vertex));
    uint32_t i = 0, j = 0, k = 0;
    for (i = 0, j = range.first; i < in_place_limit && j < range.second; k++) {
      if (vertices[s].neighbors[i] < dests[j]) {
        new_in_place[k] = vertices[s].neighbors[i];
        i++;
      } else if (vertices[s].neighbors[i] > dests[j]) {
        new_in_place[k] = dests[j];
        j++;
      } else {
        new_in_place[k] = vertices[s].neighbors[i];
        i++;
        j++;
      }
    }
    if (i < in_place_limit) {
      memcpy(new_in_place + k, &vertices[s].neighbors[i],
             (in_place_limit - i) * sizeof(vertex));
      k += (in_place_limit - i);
    } else if (j < range.second) {
      memcpy(new_in_place + k, &dests[j], (range.second - j) * sizeof(vertex));
      k += (range.second - j);
    }
    uint32_t cnt = k < NUM_IN_PLACE_NEIGHBORS ? k : NUM_IN_PLACE_NEIGHBORS;
    memcpy(vertices[s].neighbors, new_in_place, cnt * sizeof(vertex));
    if (k > NUM_IN_PLACE_NEIGHBORS) {
      uint32_t sec_cnt = k - NUM_IN_PLACE_NEIGHBORS;
      memcpy(dests + range.first, new_in_place + cnt, sec_cnt * sizeof(vertex));
      if (vertices[s].skip_list_level_neighbors == nullptr &&
          degree + size <= MEDIUM_DEGREE) {
        for (uint32_t i = range.first; i < range.first + sec_cnt; i++) {
          array_pma[i] = 1;
        }
        // array_pma_node[idx] = 1;
      } else {
        for (uint32_t i = range.first; i < range.first + sec_cnt; i++) {
          array_skip_list[i] = 1;
        }
        array_skip_list_node[idx] = 1;
      }
    }

    if (degree < NUM_IN_PLACE_NEIGHBORS) {
      vertices[s].degree = LOCK_MASK;
      vertices[s].degree = cnt;
    }
  }
#if ENABLE_LOCK
  unlock(&vertices[s].degree);
#endif
  return;
}

void Graph::add_edge_batch(vertex *srcs, vertex *dests, uint32_t edge_count,
                           std::vector<uint32_t> &perm) {
  // bitarray to show where the edge store 显示边缘存储位置的位数组
  uint8_t *array_pma = (uint8_t *)calloc(edge_count, sizeof(uint8_t));//分配元素个数跟元素大小
  uint8_t *array_skip_list = (uint8_t *)calloc(edge_count, sizeof(uint8_t));

  // generate edge array 生成边缘阵列
  std::vector<uint32_t> parts;
  vertex cur_src = srcs[0];
  parts.emplace_back(0);
  for (uint32_t i = 1; i < edge_count; i++) {
    if (cur_src != srcs[i]) {
      parts.emplace_back(i);
      cur_src = srcs[i];
    }
  }
  parts.emplace_back(edge_count);
  // uint8_t *array_pma_node = (uint8_t *)calloc(parts.size(), sizeof(uint8_t));
  uint8_t *array_skip_list_node =
      (uint8_t *)calloc(parts.size(), sizeof(uint8_t));
  // TODO: add muti-thread to parallel excute
  for (uint32_t i = 0; i < parts.size() - 1; i++) {
    add_inplace(srcs, dests, i, parts, array_pma, array_skip_list,
                array_skip_list_node);
  }
  printf("starting to insert items to pma\n");
  for (uint32_t i = 0; i < edge_count; i++) {
    auto idx = perm[i];
    if (array_pma[idx] == 1) {
      vertex s = srcs[idx];
#if ENABLE_LOCK
      lock(&vertices[s].degree);
#endif
      pma_container_->insert_pma({srcs[idx], dests[idx]});
      vertices[s].degree++;
    }
  }
  printf("starting to insert items to skiplist\n");
  for (uint32_t i = 0; i < parts.size() - 1; i++) {
    if (array_skip_list_node[i] == 1) {
      add_skip_list(srcs, dests, i, parts, array_skip_list);
    }
  }
}

// void Graph::add_pma(vertex *srcs, vertex *dests, uint32_t idx,
//                     std::vector<uint32_t> &parts, uint8_t *array_pma) {
//   vertex s = srcs[parts[idx]];
//   std::pair<uint32_t, uint32_t> range = {parts[idx], parts[idx + 1]};
// #if ENABLE_LOCK
//   lock(&vertices[s].degree);
// #endif
//   if (vertices[s].pma_level_neighbors == nullptr) {
//     pma_container *container;
//     vertices[s].pma_level_neighbors = container;
//   }
//   pma_container *container = (pma_container
//   *)(vertices[s].pma_level_neighbors); for (uint32_t i = range.first; i <
//   range.second; i++) {
//     if (array_pma[i] == 1) {
//       container->insert_element(dests[i]);
//       vertices[s].degree++;
//     }
//   }
// #if ENABLE_LOCK
//   unlock(&vertices[s].degree);
// #endif
// }

void Graph::add_skip_list(vertex *srcs, vertex *dests, uint32_t idx,
                          std::vector<uint32_t> &parts,
                          uint8_t *array_skip_list) {
  vertex s = srcs[parts[idx]];
  std::pair<uint32_t, uint32_t> range = {parts[idx], parts[idx + 1]};
#if ENABLE_LOCK
  lock(&vertices[s].degree);
#endif
  if (vertices[s].skip_list_level_neighbors == nullptr) {
    skiplist_container *container = new skiplist_container(0x7fffffff);
    vertices[s].skip_list_level_neighbors = container;
    // TODO: move the pma elements to skip list
  }
  skiplist_container *container =
      (skiplist_container *)(vertices[s].skip_list_level_neighbors);
  for (uint32_t i = range.first; i < range.second; i++) {
    if (array_skip_list[i] == 1) {
      container->insert(dests[i]);
      vertices[s].degree++;
    }
  }
#if ENABLE_LOCK
  unlock(&vertices[s].degree);
#endif
}

inline uint32_t Graph::degree(const vertex s) const {
#if ENABLE_LOCK
  return vertices[s].degree & UNLOCK_MASK;
#else
  return vertices[s].degree;
#endif
}

uint32_t Graph::get_num_edges(void) {
  uint64_t num_edges{0};
  for (uint32_t i = 0; i < num_vertices; i++) num_edges += vertices[i].degree;
  return num_edges;
}

uint32_t Graph::get_num_vertices(void) const { return num_vertices; }
#endif  // _GRAPH_H_
