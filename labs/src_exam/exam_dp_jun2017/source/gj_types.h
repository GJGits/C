#include <inttypes.h>

typedef struct cli_params {
    char **requests;
    int len;
} cli_params;

typedef struct run_params {
    const char *wait;
} run_params;

typedef struct client_req {
    int status;
    uint8_t message;
    uint16_t fnlen;
    char filename[256];
} client_req;

typedef struct server_res {
    /* data */
} server_res;

