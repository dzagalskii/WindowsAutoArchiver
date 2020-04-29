#include <stdio.h>
#include <Windows.h>

#define zipCommand "C:\\VS\\BSIT_3\\7-Zip\\7z.exe u -tzip -ssw -mx1 -r0"
/*
u - обновить файл в архиве (если его нет, добавить)
ssw - даже если файл используется в данный момент, он будет добавлен в архив
r0 - включить в архив все каталоги
mx1 - компрессия архивации 1
*/

char serviceName[] = "BSIT_3";
char servicePath[] = "C:\\VS\\BSIT_3\\Release\\BSIT_3.exe";
SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE hStatus;

//установить службу
int InstallService(void)
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager) 
	{
		OutputDebugString("[myService]: Error: Cannot open Service Control Manager");
		return -1;
	}
	SC_HANDLE hService = CreateServiceA(hSCManager, serviceName, serviceName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, servicePath, NULL, NULL, NULL, NULL, NULL);
	if (!hService) 
	{
		int err = GetLastError();
		switch (err)
		{
		case ERROR_ACCESS_DENIED:
			OutputDebugString("[myService]: Error: ERROR_ACCESS_DENIED");
			break;
		case ERROR_CIRCULAR_DEPENDENCY:
			OutputDebugString("[myService]: Error: ERROR_CIRCULAR_DEPENDENCY");
			break;
		case ERROR_INVALID_HANDLE:
			OutputDebugString("[myService]: Error: ERROR_INVALID_HANDLE");
			break;
		case ERROR_INVALID_NAME: OutputDebugString("[myService]: Error: ERROR_INVALID_NAME");
			break;
		case ERROR_INVALID_SERVICE_ACCOUNT:
			OutputDebugString("[myService]: Error: ERROR_INVALID_SERVICE_ACCOUNT");
			break;
		case ERROR_SERVICE_EXISTS:
			OutputDebugString("[myService]: Error: ERROR_SERVICE_EXISTS");
			break;
		default: OutputDebugString("[myService]: Error: Undefined");
		}
		CloseServiceHandle(hSCManager);
		return -1;
	}
	OutputDebugString("[myService]: Success install service");
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	return 0;
}

//удалить службу
int RemoveService(void)
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (!hSCManager) 
	{
		OutputDebugString("[myService]: Error: Can't open Service Control Manager");
		return -1;
	}

	SC_HANDLE hService = OpenServiceA(hSCManager, serviceName, SERVICE_STOP | DELETE);

	if (!hService) 
	{
		OutputDebugString("[myService]: Error: Can't remove service");
		CloseServiceHandle(hSCManager);
		return -1;
	}

	DeleteService(hService);
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	OutputDebugString("Success remove service");

	return 0;
}

//запустить службу
int RunService(void)
{
	SC_HANDLE hSCManager = OpenSCManager(NULL,/*local computer*/NULL, /*servicesActive database */SC_MANAGER_ALL_ACCESS	/*full access rights*/);
	SC_HANDLE hService = OpenServiceA(hSCManager, /*SCM database*/serviceName, /*name of service*/SERVICE_ALL_ACCESS /*full access*/);

	if (!hService) 
	{
		CloseServiceHandle(hSCManager);
		OutputDebugString("[myService]: Error: incorrect handle in RunService()");
		return -1;
	}

	if (!StartService(hService, 0, NULL)) 
	{
		CloseServiceHandle(hSCManager);
		OutputDebugString("[myService]: Error: cant StartService");
		return -1;
	}
	OutputDebugString("[myService]: Success start service");
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	return 0;
}

//остановить службу
void StopService(void)
{
	SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
	if (serviceControlManager)
	{
		SC_HANDLE service = OpenService(serviceControlManager, serviceName, SERVICE_QUERY_STATUS | SERVICE_STOP);
		if (service)
		{
			SERVICE_STATUS serviceStatus;
			if (QueryServiceStatus(service, &serviceStatus))
			{
				if (serviceStatus.dwCurrentState == SERVICE_RUNNING)
					ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus);
			}
		}
	}
	else 
	{
		OutputDebugString("[myService]: Error: cant StopService");
		return;
	}
	OutputDebugString("[myService]: Success stop service");
	CloseServiceHandle(serviceControlManager);
	return;
}

//изменение состояния службы
void ControlHandler(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:

		OutputDebugString("[myService]: Stopped");

		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hStatus, &serviceStatus);
		return;
	case SERVICE_CONTROL_SHUTDOWN:
		OutputDebugString("[myService]: Shutdown");
		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hStatus, &serviceStatus);
		return;
	default:
		break;
	}
	SetServiceStatus(hStatus, &serviceStatus);
	return;
}

//главная функция сервиса
void ServiceMain(int argc, char** argv)
{
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode = 0;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;

	hStatus = RegisterServiceCtrlHandler(serviceName, (LPHANDLER_FUNCTION)ControlHandler);
	if (!hStatus)
	{
		OutputDebugString("[myService]: Error: Register ServiceControl handler failed");
		return;
	}

	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hStatus, &serviceStatus);

	while (serviceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		FILE* conf_file = NULL;
		errno_t err;
		if (err = fopen_s(&conf_file, "C:\\VS\\BSIT_3\\config.txt", "r")) 
		{
			OutputDebugString("[myService]: Error: cannot open configure.txt");
			serviceStatus.dwCurrentState = SERVICE_STOPPED;
			serviceStatus.dwWin32ExitCode = -1;
			SetServiceStatus(hStatus, &serviceStatus);
			return;
		}

		char folder_path[150], folder_archive_path[150];
		memset(folder_path, 0, sizeof(folder_path));
		memset(folder_archive_path, 0, sizeof(folder_archive_path));

		fgets(folder_path, sizeof(folder_path), conf_file);
		fgets(folder_archive_path, sizeof(folder_archive_path), conf_file);

		char command[500];
		memset(command, 0, sizeof(command));

		sprintf_s(command, sizeof(zipCommand) + sizeof(folder_archive_path) + sizeof(folder_path), "%s %s %s", zipCommand, folder_archive_path, folder_path);
		fclose(conf_file);
		system(command);

		OutputDebugString("[myService]: zip is changed");
		Sleep(10000);
	}
	return;
}

void main(int argc, TCHAR* argv[])
{
	if (argc - 1 == 0) 
	{
		//точка входа - ServiceMain
		SERVICE_TABLE_ENTRY ServiceTable[2];
		ServiceTable[0].lpServiceName = serviceName;
		ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
		ServiceTable[1].lpServiceName = NULL;
		ServiceTable[1].lpServiceProc = NULL;

		if (!StartServiceCtrlDispatcher(ServiceTable)) 
		{
			//связываем сервис с SCM
			OutputDebugString("[myService]: Error: StartServiceCtrlDispatcher");
		}
	}
	else if (!strcmp(argv[argc - 1], "install")) {
		InstallService();
	}
	else if (!strcmp(argv[argc - 1], "remove")) {
		RemoveService();
	}
	else if (!strcmp(argv[argc - 1], "start")) {
		RunService();
	}
	else if (!strcmp(argv[argc - 1], "stop")) {
		StopService();
	}
	exit(0);
}