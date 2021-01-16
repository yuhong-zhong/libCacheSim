#include "../include/libCacheSim.h"
#include "../include/libCacheSim/evictionAlgo.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s <file path> <cache size> <number of lines>\n", argv[0]);
    exit(1);
  }

  char *file_path = argv[1];
  long cache_size;
  long num_lines;
  sscanf(argv[2], "%ld", &cache_size);
  sscanf(argv[3], "%ld", &num_lines);

  common_cache_params_t cc_params = {.cache_size=cache_size, .default_ttl=0};
  cache_t* lru = LRU_init(cc_params, NULL);

  request_t *req = new_request();

  FILE *fp;
  char *buf = NULL;
  ssize_t read_size;
  size_t buf_size;

  fp = fopen(file_path, "r");
  if (fp == NULL) {
    printf("cannot open file\n");
    exit(1);
  }

  long cur_line = 0;
  long req_count = 0;
  long miss_count = 0;
  while ((read_size = getline(&buf, &buf_size, fp)) != -1) {
    req->valid = TRUE;
    req->obj_size = 0;

    /* read request from trace */
    char *token = strtok(buf, ",");
    int token_index = 0;
    while (token != NULL) {
      if (token_index == 0) {
        sscanf(token, "%ld", &req->real_time);
      } else if (token_index == 1) {
        req->obj_id_int = g_quark_from_string(token);
      } else if (token_index == 2) {
        size_t key_size;
        sscanf(token, "%ld", &key_size);
        req->obj_size += key_size;
      } else if (token_index == 3) {
        size_t value_size;
        sscanf(token, "%ld", &value_size);
        req->obj_size += value_size;
      }

      token = strtok(NULL, ",");
      ++token_index;
    }

    /* apply request on cache */
    if (req->valid) {
      ++req_count;
      if (lru->get(lru, req) != cache_ck_hit) {
        ++miss_count;
      }
    }

    ++cur_line;
    if (cur_line >= num_lines) {
      break;
    }
  }

  printf("miss ratio: %f\n", (float)(miss_count) / (float)(req_count));
}
