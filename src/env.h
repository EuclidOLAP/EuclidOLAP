#ifndef EUCLIDOLAP_ENV_H
#define EUCLIDOLAP_ENV_H 1

#define ENV_ATTR_LEN_LIMIT 256

typedef struct _olap_env_
{
    char OLAP_HOME[ENV_ATTR_LEN_LIMIT + 1];
} OLAPEnv;

void olap_env_init(void);

#endif