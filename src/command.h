#ifndef EUCLID__COMMAND_H
#define EUCLID__COMMAND_H 1

#include <semaphore.h>

typedef unsigned short intent;

typedef struct euclid_command
{
	char *bytes;
	struct euclid_command *result;
	sem_t sem;
} EuclidCommand;

/**
 * (terminal client) -> (euclid node)
 */
#define INTENT__TERMINAL_CONTROL 0

/**
 * (child node) -> (euclid node)
 */
#define INTENT__CHILD_NODE_JOIN 1

/**
 * (euclid node) -> (terminal client)
 * (euclid node) -> (child node)
 */
#define INTENT__ALLOW 2

/**
 * (terminal client) -> (euclid master node)
 *
 * 4 bytes - data package capacity
 * 2 bytes - intention
 * 4 bytes - The number of bytes occupied by the MDX statement
 * N bytes - MDX statement bytes
 *
 */
#define INTENT__MDX 3
EuclidCommand *build_intent_command_mdx(char *mdx);

/**
 * (euclid node) -> (child node)
 *
 * 4 bytes - data package capacity
 * 2 bytes - intention
 * 8 bytes - cube g_id
 * 4 bytes - {DimRoles amount} - the number of coordinate axes, which is the number of dimension-roles.
 * 4 bytes - {MeasureMbrs amount} - the number of measure-dimension members.
 * (
 *     (4 bytes + 8 bytes * N) * {DimRoles amount}
 *         - The coordinate on the axis, which is the full path of the dimension member md_gid. (N > 0)
 *     (8 + 1 bytes) * {MeasureMbrs amount} - Measures values, and null-flag.
 * ) * V - 'V' represents the number of vectors inserted at one time. (V > 0)
 */
#define INTENT__INSERT_CUBE_MEARSURE_VALS 4

/**
 * (*) <-> (*)
 * 
 * 4 bytes - data package capacity
 * 2 bytes - intention
 * N bytes - A descriptive paragraph, include valid text and a trailing 0 character.
 */
#define INTENT__SUCCESSFUL 5

/**
 * (*) <-> (*)
 * 
 * 4 bytes - data package capacity
 * 2 bytes - intention
 * N bytes - A descriptive paragraph, include valid text and a trailing 0 character.
 */
#define INTENT__FAILURE 6

#define INTENT__UNKNOWN 65535

int init_command_module();

EuclidCommand *create_command(char *bytes);

EuclidCommand *get_const_command_intent(intent inte);

intent ec_get_intent(EuclidCommand *ec);

// int ec_release(EuclidCommand *ec);

int ec_get_capacity(EuclidCommand *ec);

int submit_command(EuclidCommand *ec);

EuclidCommand *EuclidCommand_failure(char *desc);

#endif
