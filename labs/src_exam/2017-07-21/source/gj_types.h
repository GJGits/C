#include <inttypes.h>

typedef struct cli_params {
    const char *request;
    const char *file_mod_name;
} cli_params;

typedef struct run_params {
    const char *string;
} run_params;

typedef struct client_req {
    int status;
    uint32_t fsize;
    char fbuf[10000];
} client_req;

typedef struct server_res {
    /* data */
} server_res;

