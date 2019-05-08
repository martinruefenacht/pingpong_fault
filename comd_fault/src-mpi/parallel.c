/// \file
/// Wrappers for MPI functions.  This should be the only compilation 
/// unit in the code that directly calls MPI functions.  To build a pure
/// serial version of the code with no MPI, do not define DO_MPI.  If
/// DO_MPI is not defined then all MPI functionality is replaced with
/// equivalent single task behavior.

#include "parallel.h"

#ifdef DO_MPI
#include <mpi.h>
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

static int myRank = 0;
static int nRanks = 1;

#ifdef DO_MPI
#ifdef SINGLE
#define REAL_MPI_TYPE MPI_FLOAT
#else
#define REAL_MPI_TYPE MPI_DOUBLE
#endif

#endif

int getNRanks()
{
   return nRanks;
}

int getMyRank()   
{
   return myRank;
}

/// \details
/// For now this is just a check for rank 0 but in principle it could be
/// more complex.  It is also possible to suppress practically all
/// output by causing this function to return 0 for all ranks.
int printRank()
{
   if (myRank == 0) return 1;
   return 0;
}

int timestampBarrier(const char* msg)
{
   int rc = barrierParallel();
#ifdef DO_MPI
   if (rc != MPI_SUCCESS) {
	   return MPIX_TRY_RELOAD;
   }
#endif
   if (rc != )
   if (! printRank())
      return 0;
   time_t t= time(NULL);
   char* timeString = ctime(&t);
   timeString[24] = '\0'; // clobber newline
   fprintf(screenOut, "%s: %s\n", timeString, msg);
   fflush(screenOut);
   return 0;
}

void initParallel(int* argc, char*** argv)
{
#ifdef DO_MPI
   MPI_Init(argc, argv);
#endif
}

void commParallel() {
#ifdef DO_MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &nRanks);
#endif
}

void destroyParallel()
{
#ifdef DO_MPI
   MPI_Finalize();
#endif
}

int barrierParallel()
{
#ifdef DO_MPI
   int rc = MPI_Barrier(MPI_COMM_WORLD);
   if (rc != MPI_SUCCESS)
	   return MPIX_TRY_RELOAD;
#else
   return 0;
#endif
}

/// \param [in]  sendBuf Data to send.
/// \param [in]  sendLen Number of bytes to send.
/// \param [in]  dest    Rank in MPI_COMM_WORLD where data will be sent.
/// \param [out] recvBuf Received data.
/// \param [in]  recvLen Maximum number of bytes to receive.
/// \param [in]  source  Rank in MPI_COMM_WORLD from which to receive.
/// \return Number of bytes received.
int sendReceiveParallel(void* sendBuf, int sendLen, int dest,
                        void* recvBuf, int recvLen, int source)
{
#ifdef DO_MPI
   int bytesReceived;
   MPI_Status status;
   int rc = MPI_Sendrecv(sendBuf, sendLen, MPI_BYTE, dest,   0,
                recvBuf, recvLen, MPI_BYTE, source, 0,
                MPI_COMM_WORLD, &status);
   if (rc != MPI_SUCCESS) {
	   return -1;
   }
   rc = MPI_Get_count(&status, MPI_BYTE, &bytesReceived);
   if (rc != MPI_SUCCESS) {
   	   return -1;
   }
   return bytesReceived;
#else
   assert(source == dest);
   memcpy(recvBuf, sendBuf, sendLen);

   return sendLen;
#endif
}

int addIntParallel(int* sendBuf, int* recvBuf, int count)
{
#ifdef DO_MPI
   int rc = MPI_Allreduce(sendBuf, recvBuf, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
   if (rc != MPI_SUCCESS) {
   	   return MPIX_TRY_RELOAD;
      }
#else
   for (int ii=0; ii<count; ++ii)
      recvBuf[ii] = sendBuf[ii];
#endif
   return 0;
}

int addRealParallel(real_t* sendBuf, real_t* recvBuf, int count)
{
#ifdef DO_MPI
   int rc = MPI_Allreduce(sendBuf, recvBuf, count, REAL_MPI_TYPE, MPI_SUM, MPI_COMM_WORLD);
   if (rc != MPI_SUCCESS) {
   	   return MPIX_TRY_RELOAD;
      }
#else
   for (int ii=0; ii<count; ++ii)
      recvBuf[ii] = sendBuf[ii];
#endif
   return 0;
}

int addDoubleParallel(double* sendBuf, double* recvBuf, int count)
{
#ifdef DO_MPI
   int rc = MPI_Allreduce(sendBuf, recvBuf, count, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
   if (rc != MPI_SUCCESS) {
   	   return MPIX_TRY_RELOAD;
      }
#else
   for (int ii=0; ii<count; ++ii)
      recvBuf[ii] = sendBuf[ii];
#endif
   return 0;
}

int maxIntParallel(int* sendBuf, int* recvBuf, int count)
{
#ifdef DO_MPI
   int rc = MPI_Allreduce(sendBuf, recvBuf, count, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
   if (rc != MPI_SUCCESS) {
   	   return MPIX_TRY_RELOAD;
      }
#else
   for (int ii=0; ii<count; ++ii)
      recvBuf[ii] = sendBuf[ii];
#endif
   return 0;
}


int minRankDoubleParallel(RankReduceData* sendBuf, RankReduceData* recvBuf, int count)
{
#ifdef DO_MPI
   int rc = MPI_Allreduce(sendBuf, recvBuf, count, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
   if (rc != MPI_SUCCESS) {
   	   return MPIX_TRY_RELOAD;
      }
#else
   for (int ii=0; ii<count; ++ii)
   {
      recvBuf[ii].val = sendBuf[ii].val;
      recvBuf[ii].rank = sendBuf[ii].rank;
   }
#endif
   return 0;
}

int maxRankDoubleParallel(RankReduceData* sendBuf, RankReduceData* recvBuf, int count)
{
#ifdef DO_MPI
   int rc = MPI_Allreduce(sendBuf, recvBuf, count, MPI_DOUBLE_INT, MPI_MAXLOC, MPI_COMM_WORLD);
   if (rc != MPI_SUCCESS) {
   	   return MPIX_TRY_RELOAD;
      }
#else
   for (int ii=0; ii<count; ++ii)
   {
      recvBuf[ii].val = sendBuf[ii].val;
      recvBuf[ii].rank = sendBuf[ii].rank;
   }
#endif
   return 0;
}

/// \param [in] count Length of buf in bytes.
int bcastParallel(void* buf, int count, int root)
{
#ifdef DO_MPI
   int rc = MPI_Bcast(buf, count, MPI_BYTE, root, MPI_COMM_WORLD);
   if (rc != MPI_SUCCESS) {
   	   return MPIX_TRY_RELOAD;
      }
#endif
   return 0;
}

void abortParallel()
{
#ifdef DO_MPI
   MPI_Abort(MPI_COMM_WORLD, -1);
#endif
}

int epochParallel(int *epoch) {
#ifdef DO_MPI
	int rc = MPIX_Get_fault_epoch(epoch);
	if (rc != MPI_SUCCESS) {
		   return MPIX_TRY_RELOAD;
	   }
#endif
	return 0;
}

void checkpointParallel() {
#ifdef DO_MPI
	MPIX_Load_checkpoint();
#endif
}

int checkpointwriteParallel() {
#ifdef DO_MPI
	int rc = MPIX_Checkpoint_read();
	if (rc != MPI_SUCCESS) {
			   return MPIX_TRY_RELOAD;
		   }
#endif
	return 0;
}

void errhandlerParallel() {
#ifdef DO_MPI
	MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
#endif
}

void successParallel(int *code) {
#ifdef DO_MPI
	*code = MPI_SUCCESS;
#else
	*code = 0;
#endif
}

int builtWithMpi(void)
{
#ifdef DO_MPI
   return 1;
#else
   return 0;
#endif
}


