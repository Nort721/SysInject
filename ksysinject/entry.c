#include <ntddk.h>
#include "ntifs.h"
#include <windef.h>

// Definitions
#define IOCTL_KILL_PID    CTL_CODE(0x8000, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

DRIVER_DISPATCH ProtectorDeviceControl;

VOID KeKillProcessSimple(DWORD pid) {
	__try {
		HANDLE hProcess = NULL;
		CLIENT_ID ClientId = { 0 };
		OBJECT_ATTRIBUTES oa = { 0 };
		ClientId.UniqueProcess = (HANDLE)pid;
		ClientId.UniqueThread = 0;
		oa.Length = sizeof(oa);
		oa.RootDirectory = 0;
		oa.ObjectName = 0;
		oa.Attributes = 0;
		oa.SecurityDescriptor = 0;
		oa.SecurityQualityOfService = 0;
		ZwOpenProcess(&hProcess, 1, &oa, &ClientId);
		if (hProcess)
		{
			ZwTerminateProcess(hProcess, 0);
			ZwClose(hProcess);
		}
		KdPrint(("[KSysInject] attempted to kill processes from kernel"));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("[KSysInject] elevated kill process request failed"));
	}
}

BOOLEAN KeKillProcessZeroMemory(DWORD pid) {
	NTSTATUS ntStatus = STATUS_SUCCESS;
	int i = 0;
	PVOID handle;
	PEPROCESS Eprocess;
	ntStatus = PsLookupProcessByProcessId(pid, &Eprocess);
	if (NT_SUCCESS(ntStatus))
	{
		PKAPC_STATE pKs = (PKAPC_STATE)ExAllocatePool(NonPagedPool, sizeof(PKAPC_STATE));
		KeStackAttachProcess(Eprocess, pKs);
		for (i = 0; i <= 0x7fffffff; i += 0x1000)
		{
			if (MmIsAddressValid((PVOID)i))
			{
				_try
				{
					ProbeForWrite((PVOID)i,0x1000,sizeof(ULONG));
					memset((PVOID)i,0xcc,0x1000);
				}
				_except(1) { continue; }
			}
			else {
				if (i > 0x1000000)  
					break;
			}
		}
		KeUnstackDetachProcess(pKs);
		if (ObOpenObjectByPointer((PVOID)Eprocess, 0, NULL, 0, NULL, KernelMode, &handle) != STATUS_SUCCESS)
			return FALSE;
		ZwTerminateProcess((HANDLE)handle, STATUS_SUCCESS);
		ZwClose((HANDLE)handle);
		return TRUE;
	}
	return FALSE;

}

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD drvUnload;

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING registry) {
	UNREFERENCED_PARAMETER(registry);

	NTSTATUS status = STATUS_SUCCESS;

	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\KSysInject");
	UNICODE_STRING symName = RTL_CONSTANT_STRING(L"\\??\\KSysInject");
	PDEVICE_OBJECT DeviceObject = NULL;

	status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	status = IoCreateSymbolicLink(&symName, &deviceName);

	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(DeviceObject);
		return status;
	}

	DriverObject->DriverUnload = drvUnload;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProtectorDeviceControl;

	return status;
}

NTSTATUS ProtectorDeviceControl(PDEVICE_OBJECT pob, PIRP Irp) {
	UNREFERENCED_PARAMETER(pob);

	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_KILL_PID:
	{
		ULONG size = stack->Parameters.DeviceIoControl.InputBufferLength;

		if (size != sizeof(ULONG)) {
			status = STATUS_INVALID_BUFFER_SIZE;
			break;
		}

		ULONG* data = (ULONG*)Irp->AssociatedIrp.SystemBuffer;
		DWORD targetPid = *data;
		KeKillProcessSimple(targetPid);
		break;
	}
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

VOID drvUnload(PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symName = RTL_CONSTANT_STRING(L"\\??\\Protector");

	// Delete the symbolic link and device object
	IoDeleteSymbolicLink(&symName);
	if (DriverObject->DeviceObject)
		IoDeleteDevice(DriverObject->DeviceObject);

	KdPrint(("ProtectorUnload: Unloaded\n"));
}