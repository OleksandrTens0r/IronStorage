#include "DriverLoader.h"



DriverLoader::DriverLoader() :	
	is_init_(false),
	is_loaded_(false),
	is_started_(false),
	file_path_(nullptr),
	service_name_(nullptr),
	display_name_(nullptr),
	start_type_(0),
	service_(nullptr)
{
	
}

DriverLoader::DriverLoader(LPTSTR file_path, LPTSTR service_name, LPTSTR display_name, DWORD start_type) :	
	is_init_(false),																							
	is_loaded_(false),																							
	is_started_(false),
	file_path_(file_path),
	service_name_(service_name),
	display_name_(display_name),
	start_type_(start_type),
	service_(nullptr)
{

}

bool DriverLoader::is_init() const
{
	return is_init_;
}

bool DriverLoader::is_loaded() const
{
	return is_loaded_;
}

bool DriverLoader::is_started() const
{
	return is_started_;
}

void DriverLoader::create_svc()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

	if (hSCManager == nullptr)
	{
		throw std::exception(&"OpenSCManager Failed with error code: "[GetLastError()]);
	}
		

	service_ = CreateService(hSCManager,
		service_name_,
		display_name_,
		SC_MANAGER_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		start_type_,
		SERVICE_ERROR_NORMAL,
		file_path_,
		nullptr, nullptr, nullptr, nullptr, nullptr);

	if (service_ == nullptr)
	{
		service_ = OpenService(hSCManager, service_name_, SERVICE_ALL_ACCESS);

		if (service_ == nullptr)
		{
			CloseServiceHandle(hSCManager);
			throw std::exception(&"CreateService Failed with error code: "[GetLastError()]);
		}
	}

	is_loaded_ = true;
	CloseServiceHandle(hSCManager);
}

void DriverLoader::start_svc()
{
	if (!is_loaded())
	{
		throw DriverNotLoadedException();
	}

	if (is_started()) { return; }
		
	SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);

	if (hSCManager == nullptr)
		throw std::exception(&"OpenSCManager Failed with error code: "[GetLastError()]);

	service_ = OpenService(hSCManager, service_name_, SERVICE_ALL_ACCESS);

	if (service_ == nullptr)
	{
		CloseServiceHandle(hSCManager);
		throw std::exception(&"OpenService Failed with error code: "[GetLastError()]);
	}

	if (StartService(service_, 0, nullptr) == NULL)
	{
		CloseServiceHandle(hSCManager);
		CloseServiceHandle(service_);
		throw std::exception(&"StartService Failed with error code: "[GetLastError()]);
	}

	CloseServiceHandle(hSCManager);
	is_started_ = true;
}

void DriverLoader::stop_svc()
{
	SERVICE_STATUS service_status;

	if (!is_started()) { return; }
		

	SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);

	if (hSCManager == nullptr)
	{
		throw std::exception(&"OpenSCManager Failed with error code: "[GetLastError()]);
	}
		
	service_ = OpenService(hSCManager, service_name_, SERVICE_ALL_ACCESS);

	if (service_ == nullptr)
	{
		CloseServiceHandle(hSCManager);
		throw std::exception(&"OpenService Failed with error code: "[GetLastError()]);
	}

	if (ControlService(service_, SERVICE_CONTROL_STOP, &service_status) == NULL)
	{
		CloseServiceHandle(hSCManager);
		CloseServiceHandle(service_);
		throw std::exception(&"ControlService Failed with error code: "[GetLastError()]);
	}

	CloseServiceHandle(hSCManager);
	CloseServiceHandle(service_);
	is_started_ = false;
}

void DriverLoader::unload_svc()
{
	if (!is_loaded()) { return;}

	if (is_started())
	{
		stop_svc();
	}

	SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);

	if (hSCManager == nullptr)
	{
		throw std::exception(&"OpenSCManager Failed with error code: "[GetLastError()]);
	}

	service_ = OpenService(hSCManager, service_name_, SERVICE_ALL_ACCESS);

	if (service_ == nullptr)
	{
		CloseServiceHandle(hSCManager);
		throw std::exception(&"OpenService Failed with error code: "[GetLastError()]);
	}

	DeleteService(service_);
	CloseServiceHandle(hSCManager);

	is_loaded_ = false;
}


DriverLoader::~DriverLoader() = default;
