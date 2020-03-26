#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include "IronStorage.h"

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif // _PREFAST_

#pragma prefast( disable: 28719, "this warning only applies to drivers not applications" )

void PrintLastError(char* Prefix)
{
    LPVOID lpMsgBuf;

    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        0,
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
        );

    fprintf(stderr, "%s %s", Prefix, (LPTSTR) lpMsgBuf);

    LocalFree(lpMsgBuf);
}

int IronStorageDiskMount(int DeviceNumber, POPEN_FILE_INFORMATION  OpenFileInformation)
{
    char    VolumeName[] = "\\\\.\\ :";
    char    DriveName[] = " :\\";
    char    DeviceName[255];
    HANDLE  Device;
    DWORD   BytesReturned;

    VolumeName[4] = OpenFileInformation->DriveLetter;
    DriveName[0] = OpenFileInformation->DriveLetter;

    Device = CreateFile(
        VolumeName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING,
        NULL
        );

    if (Device != INVALID_HANDLE_VALUE)
    {
        CloseHandle(Device);
        SetLastError(ERROR_BUSY);
        PrintLastError(&VolumeName[4]);
        return -1;
    }

    sprintf(DeviceName, DEVICE_NAME_PREFIX "%u", DeviceNumber);

    if (!DefineDosDevice(
        DDD_RAW_TARGET_PATH,
        &VolumeName[4],
        DeviceName
        ))
    {
        PrintLastError(&VolumeName[4]);
        return -1;
    }

    Device = CreateFile(
        VolumeName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING,
        NULL
        );

    if (Device == INVALID_HANDLE_VALUE)
    {
        PrintLastError(&VolumeName[4]);
        DefineDosDevice(DDD_REMOVE_DEFINITION, &VolumeName[4], NULL);
        return -1;
    }

    if (!DeviceIoControl(
        Device,
		IOCTL_IRON_STORAGE_OPEN_FILE,
        OpenFileInformation,
        sizeof(OPEN_FILE_INFORMATION) + OpenFileInformation->FileNameLength - 1,
        NULL,
        0,
        &BytesReturned,
        NULL
        ))
    {
        PrintLastError("IronStorage:");
        DefineDosDevice(DDD_REMOVE_DEFINITION, &VolumeName[4], NULL);
        CloseHandle(Device);
        return -1;
    }

    CloseHandle(Device);

    SHChangeNotify(SHCNE_DRIVEADD, SHCNF_PATH, DriveName, NULL);

    return 0;
}

int IronStorageDiskUnMount(char DriveLetter)
{
    char    VolumeName[] = "\\\\.\\ :";
    char    DriveName[] = " :\\";
    HANDLE  Device;
    DWORD   BytesReturned;

    VolumeName[4] = DriveLetter;
    DriveName[0] = DriveLetter;

    Device = CreateFile(
        VolumeName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING,
        NULL
        );

    if (Device == INVALID_HANDLE_VALUE)
    {
        PrintLastError(&VolumeName[4]);
        return -1;
    }

    if (!DeviceIoControl(
        Device,
        FSCTL_LOCK_VOLUME,
        NULL,
        0,
        NULL,
        0,
        &BytesReturned,
        NULL
        ))
    {
        PrintLastError(&VolumeName[4]);
        CloseHandle(Device);
        return -1;
    }

    if (!DeviceIoControl(
        Device,
		IOCTL_IRON_STORAGE_CLOSE_FILE,
        NULL,
        0,
        NULL,
        0,
        &BytesReturned,
        NULL
        ))
    {
        PrintLastError("IronStorage:");
        CloseHandle(Device);
        return -1;
    }

    if (!DeviceIoControl(
        Device,
        FSCTL_DISMOUNT_VOLUME,
        NULL,
        0,
        NULL,
        0,
        &BytesReturned,
        NULL
        ))
    {
        PrintLastError(&VolumeName[4]);
        CloseHandle(Device);
        return -1;
    }

    if (!DeviceIoControl(
        Device,
        FSCTL_UNLOCK_VOLUME,
        NULL,
        0,
        NULL,
        0,
        &BytesReturned,
        NULL
        ))
    {
        PrintLastError(&VolumeName[4]);
        CloseHandle(Device);
        return -1;
    }

    CloseHandle(Device);

    if (!DefineDosDevice(
        DDD_REMOVE_DEFINITION,
        &VolumeName[4],
        NULL
        ))
    {
        PrintLastError(&VolumeName[4]);
        return -1;
    }

    SHChangeNotify(SHCNE_DRIVEREMOVED, SHCNF_PATH, DriveName, NULL);

    return 0;
}

POPEN_FILE_INFORMATION IronStorageDiskStatus(char DriveLetter)
{
    char                    VolumeName[] = "\\\\.\\ :";
    HANDLE                  Device;
    POPEN_FILE_INFORMATION  OpenFileInformation;
    DWORD                   BytesReturned;

    VolumeName[4] = DriveLetter;

    Device = CreateFile(
        VolumeName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING,
        NULL
        );

    if (Device == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    OpenFileInformation = (OPEN_FILE_INFORMATION*)malloc(sizeof(OPEN_FILE_INFORMATION) + MAX_PATH);

	if (OpenFileInformation == NULL)
	{
		return NULL;
	}

    if (!DeviceIoControl(
        Device,
		IOCTL_IRON_STORAGE_QUERY_FILE,
        NULL,
        0,
        OpenFileInformation,
        sizeof(OPEN_FILE_INFORMATION) + MAX_PATH,
        &BytesReturned,
        NULL
        ))
    {
        PrintLastError(&VolumeName[4]);
        CloseHandle(Device);
        free(OpenFileInformation);
        return NULL;
    }

    if (BytesReturned < sizeof(OPEN_FILE_INFORMATION))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        PrintLastError(&VolumeName[4]);
        CloseHandle(Device);
        free(OpenFileInformation);
        return NULL;
    }

    CloseHandle(Device);

    return OpenFileInformation;
}
