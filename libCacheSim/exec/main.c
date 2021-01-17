#include "../include/libCacheSim.h"
#include "../include/libCacheSim/evictionAlgo.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: %s <cache size> <files>\n", argv[0]);
    exit(1);
  }
  long cache_size;
  sscanf(argv[2], "%ld", &cache_size);

  common_cache_params_t cc_params = {.cache_size=cache_size, .default_ttl=0};
  cache_t* lru = LRU_init(cc_params, NULL);
  request_t *req = new_request();

  FILE *fp;
  char *buf = NULL;
  ssize_t read_size;
  size_t buf_size;

  int file_index;
  long cur_line = 0;
  long req_count = 0;
  long miss_count = 0;

  for (file_index = 0; file_index < argc - 2; ++file_index) {
    fp = fopen(argv[file_index + 2], "r");
    if (fp == NULL) {
      printf("cannot open file\n");
      exit(1);
    }
    fprintf(stderr, "start reading %s\n", argv[file_index + 2]);

    while ((read_size = getline(&buf, &buf_size, fp)) != -1) {
      req->valid = TRUE;
      req->obj_size = 0;

      /* read request from trace */
      char *token = strtok(buf, ",");
      int token_index = 0;
      while (token != NULL) {
        if (token_index == 0) {
          req->real_time = atol(token);
        } else if (token_index == 1) {
          req->obj_id_int = atol(token);
        } else if (token_index == 2) {
          req->obj_size += atol(token);
        } else if (token_index == 3) {
          req->obj_size += atol(token);
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
      if (cur_line % 1000000 == 0) {
        fprintf(stderr, "cur_line: %ld\r", cur_line);
      }
    }

    fclose(fp);
  }
  fprintf(stderr, "\n");

  printf("%f\n", (float)(miss_count) / (float)(req_count));
}
