/***********************************************************************************************************************************
Protocol Parallel Job
***********************************************************************************************************************************/
#ifndef PROTOCOL_PARALLEL_JOB_H
#define PROTOCOL_PARALLEL_JOB_H

#include "common/type/stringId.h"

/***********************************************************************************************************************************
Object type
***********************************************************************************************************************************/
typedef struct ProtocolParallelJob ProtocolParallelJob;

/***********************************************************************************************************************************
Job state enum
***********************************************************************************************************************************/
typedef enum
{
    protocolParallelJobStatePending = STRID5("pending", 0x1dc9238b00),
    protocolParallelJobStateRunning = STRID5("running", 0x1dc973ab20),
    protocolParallelJobStateDone = STRID5("done", 0x2b9e40),
} ProtocolParallelJobState;

#include "common/time.h"
#include "common/type/object.h"
#include "common/type/pack.h"
#include "protocol/client.h"

/***********************************************************************************************************************************
Constructors
***********************************************************************************************************************************/
ProtocolParallelJob *protocolParallelJobNew(const Variant *key, ProtocolCommand *command);

/***********************************************************************************************************************************
Getters/Setters
***********************************************************************************************************************************/
typedef struct ProtocolParallelJobPub
{
    const Variant *key;                                             // Unique key used to identify the job
    ProtocolCommand *command;                                       // Command to be executed
    unsigned int processId;                                         // Process that executed this job
    ProtocolParallelJobState state;                                 // Current state of the job
    int code;                                                       // Non-zero result indicates an error
    String *message;                                                // Message if there was a error
    PackRead *result;                                               // Result if job was successful
} ProtocolParallelJobPub;

// Job command
FN_INLINE_ALWAYS ProtocolCommand *
protocolParallelJobCommand(const ProtocolParallelJob *const this)
{
    return THIS_PUB(ProtocolParallelJob)->command;
}

// Job error
FN_INLINE_ALWAYS int
protocolParallelJobErrorCode(const ProtocolParallelJob *const this)
{
    return THIS_PUB(ProtocolParallelJob)->code;
}

FN_INLINE_ALWAYS const String *
protocolParallelJobErrorMessage(const ProtocolParallelJob *const this)
{
    return THIS_PUB(ProtocolParallelJob)->message;
}

void protocolParallelJobErrorSet(ProtocolParallelJob *this, int code, const String *message);

// Job key
FN_INLINE_ALWAYS const Variant *
protocolParallelJobKey(const ProtocolParallelJob *const this)
{
    return THIS_PUB(ProtocolParallelJob)->key;
}

// Process Id
FN_INLINE_ALWAYS unsigned int
protocolParallelJobProcessId(const ProtocolParallelJob *const this)
{
    return THIS_PUB(ProtocolParallelJob)->processId;
}

void protocolParallelJobProcessIdSet(ProtocolParallelJob *this, unsigned int processId);

// Job result
FN_INLINE_ALWAYS PackRead *
protocolParallelJobResult(const ProtocolParallelJob *const this)
{
    return THIS_PUB(ProtocolParallelJob)->result;
}

void protocolParallelJobResultSet(ProtocolParallelJob *const this, PackRead *const result);

// Job state
FN_INLINE_ALWAYS ProtocolParallelJobState
protocolParallelJobState(const ProtocolParallelJob *const this)
{
    return THIS_PUB(ProtocolParallelJob)->state;
}

void protocolParallelJobStateSet(ProtocolParallelJob *this, ProtocolParallelJobState state);

/***********************************************************************************************************************************
Functions
***********************************************************************************************************************************/
// Move to new parent mem context
FN_INLINE_ALWAYS ProtocolParallelJob *
protocolParallelJobMove(ProtocolParallelJob *const this, MemContext *const parentNew)
{
    return objMove(this, parentNew);
}

/***********************************************************************************************************************************
Destructor
***********************************************************************************************************************************/
FN_INLINE_ALWAYS void
protocolParallelJobFree(ProtocolParallelJob *const this)
{
    objFree(this);
}

/***********************************************************************************************************************************
Macros for function logging
***********************************************************************************************************************************/
String *protocolParallelJobToLog(const ProtocolParallelJob *this);

#define FUNCTION_LOG_PROTOCOL_PARALLEL_JOB_TYPE                                                                                    \
    ProtocolParallelJob *
#define FUNCTION_LOG_PROTOCOL_PARALLEL_JOB_FORMAT(value, buffer, bufferSize)                                                       \
    FUNCTION_LOG_STRING_OBJECT_FORMAT(value, protocolParallelJobToLog, buffer, bufferSize)

#endif
