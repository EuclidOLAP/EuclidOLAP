#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "env.h"

OLAPEnv olap_env;

void olap_env_init(void) {

    char *home = getenv("EUCLIDOLAP_HOME");
    char arr[ENV_ATTR_LEN_LIMIT];

    if (!home) {
        char *bindir = getenv("EUCLIDOLAP_BIN_DIR");

        if (!bindir) {
            fprintf(stderr, "There is no environment variable EUCLIDOLAP_BIN_DIR set.\n");
            exit(EXIT_FAILURE);
        }

        if (strlen(bindir) >= ENV_ATTR_LEN_LIMIT) {
            fprintf(stderr, "The length EUCLIDOLAP_BIN_DIR environment variables cannot exceed %d.\n", ENV_ATTR_LEN_LIMIT);
            exit(EXIT_FAILURE);
        }

        home = arr;
        memset(home, 0, ENV_ATTR_LEN_LIMIT);

        strcpy(home, bindir);

        for (int i=strlen(home)-1; i > 0; i--) {
            if (home[i] == '/')
                break;

            home[i] = 0;
        }
    }

    memset(&olap_env, 0, sizeof(OLAPEnv));

    strcpy(olap_env.OLAP_HOME, home);
}