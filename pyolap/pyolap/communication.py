"""
Communication protocol identifications
"""

"""
(terminal client) -> (euclid node)
"""
INTENT__TERMINAL_CONTROL = 0
INTENT__TERMINAL_CONTROL__BYTES = INTENT__TERMINAL_CONTROL.to_bytes(2, 'little')

"""
(worker node) -> (logical node)

4 bytes - data package capacity
2 bytes - intention
8 bytes - unsigned long: worker id
"""
INTENT__WORKER_JOINS_CLUSTER = 1
INTENT__WORKER_JOINS_CLUSTER__BYTES = INTENT__WORKER_JOINS_CLUSTER.to_bytes(2, 'little')

"""
(euclid node) -> (terminal client)
(euclid node) -> (child node)
"""
INTENT__ALLOW = 2
INTENT__ALLOW__BYTES = INTENT__ALLOW.to_bytes(2, 'little')

"""
(terminal client) -> (euclid master node)

4 bytes - data package capacity
2 bytes - intention
4 bytes - The number of bytes occupied by the MDX statement
N bytes - MDX statement bytes
"""
INTENT__MDX = 3
INTENT__MDX__BYTES = INTENT__MDX.to_bytes(2, 'little')

"""
(euclid node) -> (child node)

4 bytes - data package capacity
2 bytes - intention
N bytes - N = sizeof(InsertingMeasuresOptions), these bytes are a InsertingMeasuresOptions instance.
8 bytes - cube g_id
4 bytes - {DimRoles amount} - the number of coordinate axes, which is the number of dimension-roles.
4 bytes - {MeasureMbrs amount} - the number of measure-dimension members.
(
    (4 bytes + 8 bytes * N) * {DimRoles amount}
        - The coordinate on the axis, which is the full path of the dimension member md_gid. (N > 0)
    (8 + 1 bytes) * {MeasureMbrs amount} - Measures values, and null-flag.
) * V - 'V' represents the number of vectors inserted at one time. (V > 0)
"""
INTENT__INSERT_CUBE_MEASURE_VALS = 4
INTENT__INSERT_CUBE_MEASURE_VALS__BYTES = INTENT__INSERT_CUBE_MEASURE_VALS.to_bytes(2, 'little')

"""
(*) <-> (*)

4 bytes - data package capacity
2 bytes - intention
N bytes - A descriptive paragraph, include valid text and a trailing 0 character.
"""
INTENT__SUCCESSFUL = 5
INTENT__SUCCESSFUL__BYTES = INTENT__SUCCESSFUL.to_bytes(2, 'little')

"""
(*) <-> (*)

4 bytes - data package capacity
2 bytes - intention
N bytes - A descriptive paragraph, include valid text and a trailing 0 character.
"""
INTENT__FAILURE = 6
INTENT__FAILURE__BYTES = INTENT__FAILURE.to_bytes(2, 'little')

"""
(*) <-> (*)

4 bytes - data package capacity
2 bytes - intention
N bytes - A descriptive paragraph, include valid text and a trailing 0 character.
"""
INTENT__EXE_RESULT_DESC = 7
INTENT__EXE_RESULT_DESC__BYTES = INTENT__EXE_RESULT_DESC.to_bytes(2, 'little')

"""
(euclid master node) -> (terminal client)

4 bytes - data package capacity
2 bytes - intention
4 bytes - AX_COUNT: the number of axes of multidimensional result
(
    4 bytes - S_LEN: lenght of set
    4 bytes - T_LEN: lenght of tuple
    (
        8 bytes - member id (it's zero when formula member or no member)
        N bytes - member name string that endwith '\0', can be an empty string containing only '\0'. (N > 0)
    ) * S_LEN * T_LEN
) * AX_COUNT
8 bytes - RS_LEN: Length of the result measure array.
8 * RS_LEN bytes - measure values array
RS_LEN bytes - null flags array
? bytes - A compact list of strings(including empty strings),
          it containing the number of strings is RS_LEN, which holds
          the results of string expressions or string function evaluations.
"""
INTENT__MULTIDIM_RESULT_BIN = 8
INTENT__MULTIDIM_RESULT_BIN__BYTES = INTENT__MULTIDIM_RESULT_BIN.to_bytes(2, 'little')

"""
Execute MDX and expect the result to be returned in text format.

(terminal client) -> (euclid master node)

4 bytes - data package capacity
2 bytes - intention
4 bytes - The number of bytes occupied by the MDX statement
N bytes - MDX statement bytes
"""
INTENT__MDX_EXPECT_RESULT_TXT = 9
INTENT__MDX_EXPECT_RESULT_TXT__BYTES = INTENT__MDX_EXPECT_RESULT_TXT.to_bytes(2, 'little')

"""
The master node assigns the metric vector aggregation task to the worker node.

(master node) -> (worker node)

4 bytes - data package capacity
2 bytes - intention
8 bytes - MeasureSpace::id (Cube::gid)
8 bytes - task group code
4 bytes - max task group number
4 bytes - task group number (start with 0)
8 bytes - QOV : quantity of vectors
(
    8 bytes * {cube dimension roles quantity} - member gid array
    4 bytes - measure index
) * QOV
"""
INTENT__VECTOR_AGGREGATION = 10
INTENT__VECTOR_AGGREGATION__BYTES = INTENT__VECTOR_AGGREGATION.to_bytes(2, 'little')

"""
The aggregation node reports the execution result of the aggregate task to the logic node.

aggregation node(worker node) -> logic node(master node)

4 bytes - data package capacity
2 bytes - intention
8 bytes - MeasureSpace::id (Cube::gid)
8 bytes - task group code
4 bytes - max task group number
4 bytes - task group number (start with 0)
8 bytes - COG : a long value that count of grids, if it equales zero that mean there's no MeasureSpace object
(
    8 bytes - measure double value
) * COG
(
    1 bytes - null flag
) * COG
"""
INTENT__AGGREGATE_TASK_RESULT = 11
INTENT__AGGREGATE_TASK_RESULT__BYTES = INTENT__AGGREGATE_TASK_RESULT.to_bytes(2, 'little')

"""
logic node(master node) -> aggregation node(worker node)

4 bytes - data package capacity
2 bytes - intention
8 bytes - MeasureSpace::id (Cube::gid)
"""
INTENT__RELOAD_SPACE_OF_MEASURES = 12
INTENT__RELOAD_SPACE_OF_MEASURES__BYTES = INTENT__RELOAD_SPACE_OF_MEASURES.to_bytes(2, 'little')

"""
logic node(master node) -> aggregation node(worker node)

4 bytes - data package capacity
2 bytes - intention
8 bytes - MeasureSpace::id (Cube::gid)
"""
INTENT__SOLIDIFY_MIRROR_OF_SPACE = 13
INTENT__SOLIDIFY_MIRROR_OF_SPACE__BYTES = INTENT__SOLIDIFY_MIRROR_OF_SPACE.to_bytes(2, 'little')

INTENT__UNKNOWN = 65535
INTENT__UNKNOWN__BYTES = INTENT__UNKNOWN.to_bytes(2, 'little')
