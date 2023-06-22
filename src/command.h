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

typedef struct euclid_command Action;

/**
 * (terminal client) -> (euclid node)
 */
#define INTENT__TERMINAL_CONTROL 0

/**
 * (worker node) -> (logical node)
 * 
 * 4 bytes - data package capacity
 * 2 bytes - intention
 * 8 bytes - unsigned long: worker id
 */
#define INTENT__WORKER_JOINS_CLUSTER 1

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
#define INTENT__INSERT_CUBE_MEASURE_VALS 4

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

/**
 * (*) <-> (*)
 *
 * 4 bytes - data package capacity
 * 2 bytes - intention
 * N bytes - A descriptive paragraph, include valid text and a trailing 0 character.
 */
#define INTENT__EXE_RESULT_DESC 7

/**
 * (euclid master node) -> (terminal client)
 *
 * 4 bytes - data package capacity
 * 2 bytes - intention
 * 4 bytes - AX_COUNT: the number of axes of multidimensional result
 * (
 *     4 bytes - S_LEN: lenght of set
 *     4 bytes - T_LEN: lenght of tuple
 *     (
 *         8 bytes - member id (it's zero when formula member or no member)
 *         N bytes - member name string that endwith '\0', can be an empty string containing only '\0'. (N > 0)
 *     ) * S_LEN * T_LEN
 * ) * AX_COUNT
 * 8 bytes - RS_LEN: Length of the result measure array.
 * 8 * RS_LEN bytes - measure values array
 * RS_LEN bytes - null flags array
 */
#define INTENT__MULTIDIM_RESULT_BIN 8

/**
 * Execute MDX and expect the result to be returned in text format.
 *
 * (terminal client) -> (euclid master node)
 *
 * 4 bytes - data package capacity
 * 2 bytes - intention
 * 4 bytes - The number of bytes occupied by the MDX statement
 * N bytes - MDX statement bytes
 *
 */
#define INTENT__MDX_EXPECT_RESULT_TXT 9

/**
 * The master node assigns the metric vector aggregation task to the worker node.
 *
 * (master node) -> (worker node)
 *
 * 4 bytes - data package capacity
 * 2 bytes - intention
 * 8 bytes - MeasureSpace::id (Cube::gid)
 * 8 bytes - task group code
 * 4 bytes - max task group number
 * 4 bytes - task group number (start with 0)
 * 8 bytes - QOV : quantity of vectors
 * (
 *     8 bytes * {cube dimension roles quantity} - member gid array
 *     4 bytes - measure index
 * ) * QOV
 */
#define INTENT__VECTOR_AGGREGATION 10

/**
 * The aggregation node reports the execution result of the aggregate task to the logic node.
 *
 * aggregation node(worker node) -> logic node(master node)
 *
 * 4 bytes - data package capacity
 * 2 bytes - intention
 * 8 bytes - MeasureSpace::id (Cube::gid)
 * 8 bytes - task group code
 * 4 bytes - max task group number
 * 4 bytes - task group number (start with 0)
 * 8 bytes - COG : a long value that count of grids, if it equales zero that mean there's no MeasureSpace object
 * (
 *     8 bytes - measure double value
 * ) * COG
 * (
 *     1 bytes - null flag
 * ) * COG
 */
#define INTENT__AGGREGATE_TASK_RESULT 11

#define INTENT__UNKNOWN 65535

int init_command_module();

EuclidCommand *create_command(char *bytes);

/**
 * Create a command object and allocate payload memory.
 */
EuclidCommand *ec_new(intent inte, size_t payload_sz);

EuclidCommand *get_const_command_intent(intent inte);

intent ec_get_intent(EuclidCommand *ec);

// int ec_release(EuclidCommand *ec);

int ec_get_capacity(EuclidCommand *ec);

int submit_command(EuclidCommand *ec);

EuclidCommand *EuclidCommand_failure(char *desc);

// Modifies the execution intent of the binary command to the specified value.
void ec_change_intent(EuclidCommand *ec, intent inte);

#endif
