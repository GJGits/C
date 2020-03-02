#include <inttypes.h>

typedef struct run_params {
    const char *string;
} run_params;

typedef struct client_req {
    int status;
    uint32_t fsize;
    char fbuf[1024];
} client_req;

typedef struct server_res {
    /* data */
} server_res;

