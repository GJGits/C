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
    char message[7];
    char offset[10]; // 9(4 miliardi) + 1(end srting)
    char filename[256];
} client_req;

typedef struct server_res {
    /* data */
} server_res;

