///////////////////////////////////////////////////////////////////////////////
//
// File Communicator.cpp
//
// For more information, please see: http://www.nektar.info
//
// The MIT License
//
// Copyright (c) 2006 Division of Applied Mathematics, Brown University (USA),
// Department of Aeronautics, Imperial College London (UK), and Scientific
// Computing and Imaging Institute, University of Utah (USA).
//
// License for the specific language governing rights and limitations under
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// Description: GlobalLinSys definition
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <LibUtilities/Communication/CommMpi.h>

namespace Nektar
{
    namespace LibUtilities
    {
        string CommMpi::className
            = GetCommFactory().RegisterCreatorFunction(
                "ParallelMPI",
                CommMpi::create,
                "Parallel communication using MPI.");

        /**
         *
         */
        CommMpi::CommMpi(int narg, char* arg[])
                : Comm(narg,arg)
        {
            int retval = MPI_Init(&narg, &arg);
            if (retval != MPI_SUCCESS)
            {
                ASSERTL0(false, "Failed to initialise MPI");
            }

            MPI_Comm_size( MPI_COMM_WORLD, &m_size );
            MPI_Comm_rank( MPI_COMM_WORLD, &m_rank );
        }


        /**
         *
         */
        CommMpi::~CommMpi()
        {

        }


        /**
         *
         */
        MPI_Comm CommMpi::GetComm()
        {
            return MPI_COMM_WORLD;
        }


        /**
         *
         */
        void CommMpi::v_Finalise()
        {
            MPI_Finalize();
        }


        /**
         *
         */
        int CommMpi::v_GetRank()
        {
            return m_rank;
        }


        /**
         *
         */
        void CommMpi::v_Block()
        {
            MPI_Barrier(MPI_COMM_WORLD);
        }


        /**
         *
         */
        void CommMpi::v_Send(int pProc, Array<OneD, NekDouble>& pData)
        {
            if (MPISYNC)
            {
                MPI_Ssend( pData.get(),
                          (int) pData.num_elements(),
                          MPI_DOUBLE,
                          pProc,
                          0,
                          MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send( pData.get(),
                          (int) pData.num_elements(),
                          MPI_DOUBLE,
                          pProc,
                          0,
                          MPI_COMM_WORLD);
            }
        }


        /**
         *
         */
        void CommMpi::v_Recv(int pProc, Array<OneD, NekDouble>& pData)
        {
            MPI_Status status;
            MPI_Recv( pData.get(),
                      (int) pData.num_elements(),
                      MPI_DOUBLE,
                      pProc,
                      0,
                      MPI_COMM_WORLD,
                      &status);

            ASSERTL0(status.MPI_ERROR == MPI_SUCCESS,
                     "MPI error receiving data.");
        }


        /**
         *
         */
        void CommMpi::v_Send(int pProc, Array<OneD, int>& pData)
        {
            if (MPISYNC)
            {
                MPI_Ssend( pData.get(),
                          (int) pData.num_elements(),
                          MPI_INT,
                          pProc,
                          0,
                          MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send( pData.get(),
                          (int) pData.num_elements(),
                          MPI_INT,
                          pProc,
                          0,
                          MPI_COMM_WORLD);
            }
        }


        /**
         *
         */
        void CommMpi::v_Recv(int pProc, Array<OneD, int>& pData)
        {
            MPI_Status status;
            MPI_Recv( pData.get(),
                      (int) pData.num_elements(),
                      MPI_INT,
                      pProc,
                      0,
                      MPI_COMM_WORLD,
                      &status);

            ASSERTL0(status.MPI_ERROR == MPI_SUCCESS,
                     "MPI error receiving data.");
        }


        /**
         *
         */
        void CommMpi::v_SendRecv(int pSendProc,
                                Array<OneD, NekDouble>& pSendData,
                                int pRecvProc,
                                Array<OneD, NekDouble>& pRecvData)
        {
            MPI_Status status;
            int retval = MPI_Sendrecv(pSendData.get(),
                         (int) pSendData.num_elements(),
                         MPI_DOUBLE,
                         pSendProc,
                         0,
                         pRecvData.get(),
                         (int) pRecvData.num_elements(),
                         MPI_DOUBLE,
                         pRecvProc,
                         0,
                         MPI_COMM_WORLD,
                         &status);

            ASSERTL0(retval == MPI_SUCCESS,
                     "MPI error performing send-receive of data.");
        }


        /**
         *
         */
        void CommMpi::v_SendRecv(int pSendProc,
                                Array<OneD, int>& pSendData,
                                int pRecvProc,
                                Array<OneD, int>& pRecvData)
        {
            MPI_Status status;
            int retval = MPI_Sendrecv(pSendData.get(),
                         (int) pSendData.num_elements(),
                         MPI_INT,
                         pSendProc,
                         0,
                         pRecvData.get(),
                         (int) pRecvData.num_elements(),
                         MPI_INT,
                         pRecvProc,
                         0,
                         MPI_COMM_WORLD,
                         &status);

            ASSERTL0(retval == MPI_SUCCESS,
                     "MPI error performing send-receive of data.");
        }


        /**
         *
         */
        void CommMpi::v_AllReduce(NekDouble& pData, enum ReduceOperator pOp)
        {
            MPI_Op vOp;
            switch (pOp)
            {
            case ReduceSum: vOp = MPI_SUM; break;
            case ReduceMax: vOp = MPI_MAX; break;
            case ReduceMin: vOp = MPI_MIN; break;
            }
            int retval = MPI_Allreduce( MPI_IN_PLACE,
                                        &pData,
                                        1,
                                        MPI_DOUBLE,
                                        vOp,
                                        MPI_COMM_WORLD);

            ASSERTL0(retval == MPI_SUCCESS,
                     "MPI error performing All-reduce.");
        }


        /**
         *
         */
        void CommMpi::v_AllReduce(int& pData, enum ReduceOperator pOp)
        {
            MPI_Op vOp;
            switch (pOp)
            {
            case ReduceSum: vOp = MPI_SUM; break;
            case ReduceMax: vOp = MPI_MAX; break;
            case ReduceMin: vOp = MPI_MIN; break;
            }
            int retval = MPI_Allreduce( MPI_IN_PLACE,
                                        &pData,
                                        1,
                                        MPI_INT,
                                        vOp,
                                        MPI_COMM_WORLD);

            ASSERTL0(retval == MPI_SUCCESS,
                     "MPI error performing All-reduce.");
        }


        /**
         *
         */
        void CommMpi::v_AllReduce(Array<OneD, NekDouble>& pData, enum ReduceOperator pOp)
        {
            MPI_Op vOp;
            switch (pOp)
            {
            case ReduceSum: vOp = MPI_SUM; break;
            case ReduceMax: vOp = MPI_MAX; break;
            case ReduceMin: vOp = MPI_MIN; break;
            }
            int retval = MPI_Allreduce( MPI_IN_PLACE,
                                        pData.get(),
                                        (int) pData.num_elements(),
                                        MPI_DOUBLE,
                                        vOp,
                                        MPI_COMM_WORLD);

            ASSERTL0(retval == MPI_SUCCESS,
                     "MPI error performing All-reduce.");
        }


        /**
         *
         */
        void CommMpi::v_AllReduce(Array<OneD, int      >& pData, enum ReduceOperator pOp)
        {
            MPI_Op vOp;
            switch (pOp)
            {
            case ReduceSum: vOp = MPI_SUM; break;
            case ReduceMax: vOp = MPI_MAX; break;
            case ReduceMin: vOp = MPI_MIN; break;
            }
            int retval = MPI_Allreduce( MPI_IN_PLACE,
                                        pData.get(),
                                        (int) pData.num_elements(),
                                        MPI_INT,
                                        vOp,
                                        MPI_COMM_WORLD);

            ASSERTL0(retval == MPI_SUCCESS,
                     "MPI error performing All-reduce.");
        }

    }
}