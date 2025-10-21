// Protector.c
#include <ntddk.h>     // use only one kernel header (ntddk.h is fine for drivers)

// Definitions
#define IOCTL_KILL_PID CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PROCESS_TERMINATE                  (0x0001)  

DRIVER_DISPATCH ProtectorDeviceControl;
DRIVER_UNLOAD ProtectorUnload;

VOID KeKillProcessSimple(_In_ HANDLE ProcessId)
{
    __try
    {
        HANDLE hProcess = NULL;
        CLIENT_ID ClientId;
        OBJECT_ATTRIBUTES oa;

        RtlZeroMemory(&ClientId, sizeof(ClientId));
        RtlZeroMemory(&oa, sizeof(oa));

        ClientId.UniqueProcess = ProcessId;
        ClientId.UniqueThread = NULL;

        InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);

        // Request PROCESS_TERMINATE
        NTSTATUS st = ZwOpenProcess(&hProcess, PROCESS_TERMINATE, &oa, &ClientId);
        if (NT_SUCCESS(st) && hProcess)
        {
            ZwTerminateProcess(hProcess, STATUS_SUCCESS);
            ZwClose(hProcess);
        }
        KdPrint(("Protector: attempted to kill process\n"));
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        KdPrint(("Protector: kill request caused exception\n"));
    }
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS status = STATUS_SUCCESS;

    UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\KSysInject");
    UNICODE_STRING symName = RTL_CONSTANT_STRING(L"\\DosDevices\\KSysInject"); // more conventional
    PDEVICE_OBJECT DeviceObject = NULL;

    status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
    if (!NT_SUCCESS(status))
        return status;

    status = IoCreateSymbolicLink(&symName, &deviceName);
    if (!NT_SUCCESS(status))
    {
        IoDeleteDevice(DeviceObject);
        return status;
    }

    DriverObject->DriverUnload = ProtectorUnload;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProtectorDeviceControl;

    KdPrint(("KSysinject: DriverEntry success\n"));
    return STATUS_SUCCESS;
}

NTSTATUS ProtectorDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG_PTR information = 0;

    switch (stack->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_KILL_PID:
    {
        ULONG inlen = stack->Parameters.DeviceIoControl.InputBufferLength;
        if (inlen != sizeof(ULONG))
        {
            status = STATUS_INVALID_BUFFER_SIZE;
            break;
        }

        ULONG* data = (ULONG*)Irp->AssociatedIrp.SystemBuffer;
        if (!data)
        {
            status = STATUS_INVALID_USER_BUFFER;
            break;
        }

        HANDLE targetPid = (HANDLE)(ULONG_PTR)(*data);
        KeKillProcessSimple(targetPid);
        information = 0;
        break;
    }
    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

VOID ProtectorUnload(PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING symName = RTL_CONSTANT_STRING(L"\\DosDevices\\KSysInject");

    IoDeleteSymbolicLink(&symName);
    if (DriverObject->DeviceObject)
        IoDeleteDevice(DriverObject->DeviceObject);

    KdPrint(("KSysinject: Unloaded\n"));
}
