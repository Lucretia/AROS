#ifndef WORKER_H
#define WORKER_H

#define WM_START    1
#define WM_STOP     2
#define WM_RESUME   3

#define WA_SCANNER  1

#define SCAN_BUFFER 400

struct WorkerContext
{
	ULONG workerAction;
	APTR (*start)(struct WorkerContext *con);
	APTR (*resume)(struct WorkerContext *con);
	APTR (*stop)(struct WorkerContext *con);
};

struct ScannerWorkerContext
{
	struct WorkerContext swc_Context;
	struct ExAllControl *swc_EAC;
	BPTR swc_DirLock;
	BOOL swc_More;
	STRPTR swc_Buffer;
	struct WorkerMessage *swc_CurrentRequest;
};

struct WorkerMessage
{
	struct Message w_Message;
	ULONG w_Command;
	ULONG w_Action;
	ULONG w_ID;
};

struct SingleResult
{
	UBYTE *sr_Name;
};

struct WorkerScanRequest
{
	/* The requester must complete this part of the message */
	struct WorkerMessage wsr_WMessage;
	BPTR   wsr_DirLock;
	/* This part is filled in by the worker */
	BOOL   wsr_More;
	ULONG wsr_Results;
	struct SingleResult *wsr_ResultsArray;
};

#endif
