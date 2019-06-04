/// \file
/// Wrappers for MPI functions.

#ifndef _PARALLEL_H_
#define _PARALLEL_H_

#include "mytype.h"

/// Structure for use with MPI_MINLOC and MPI_MAXLOC operations.
typedef struct RankReduceDataSt
{
   double val;
   int rank;
} RankReduceData;

/// Return total number of processors.
int getNRanks(void);

/// Return local rank.
int getMyRank(void);

/// Return non-zero if printing occurs from this rank.
int printRank(void);

/// Print a timestamp and message when all tasks arrive.
int timestampBarrier(const char* msg);

/// Wrapper for MPI_Init.
void initParallel(int *argc, char ***argv);

///Wrapper for COMM_WORLD size and rank
void commParallel(void);

/// Wrapper for MPI_Finalize.
void destroyParallel(void);

/// Wrapper for MPI_Barrier(MPI_COMM_WORLD).
int barrierParallel(void);

/// Wrapper for MPI_Sendrecv.
int sendReceiveParallel(void* sendBuf, int sendLen, int dest,
                        void* recvBuf, int recvLen, int source);

/// Wrapper for MPI_Allreduce integer sum.
int addIntParallel(int* sendBuf, int* recvBuf, int count);

/// Wrapper for MPI_Allreduce real sum.
int addRealParallel(real_t* sendBuf, real_t* recvBuf, int count);

/// Wrapper for MPI_Allreduce double sum.
int addDoubleParallel(double* sendBuf, double* recvBuf, int count);

/// Wrapper for MPI_Allreduce integer max.
int maxIntParallel(int* sendBuf, int* recvBuf, int count);

/// Wrapper for MPI_Allreduce double min with rank.
int minRankDoubleParallel(RankReduceData* sendBuf, RankReduceData* recvBuf, int count);

/// Wrapper for MPI_Allreduce double max with rank.
int maxRankDoubleParallel(RankReduceData* sendBuf, RankReduceData* recvBuf, int count);

/// Wrapper for MPI_Bcast
int bcastParallel(void* buf, int len, int root);

/// Wrapper for MPI_Abort
void abortParallel(void);

///Wrapper for MPIX_Get_fault_epoch(&epoch);
int epochParallel(int *epoch);

///Wrapper for MPIX_Load_checkpoint();
void checkpointParallel(void);

///Wrapper for MPIX_checkpoint();
int checkpointwriteParallel(void);

///Wrapper for MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
void errhandlerParallel(void);

///Wrapper for MPI_SUCCESS
void successParallel(int *code);

///  Return non-zero if code was built with MPI active.
int builtWithMpi(void);

#endif

