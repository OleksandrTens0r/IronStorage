#pragma once

#include "Windows.h"
#include "exception"

class DriverNotLoadedException : public std::exception { };

class DriverLoader
{
public:
	DriverLoader();
	DriverLoader(LPTSTR file_path, LPTSTR service_name, LPTSTR display_name, DWORD start_type);

	DriverLoader(DriverLoader const& other) = delete;
	DriverLoader& operator=(DriverLoader const& other) = delete;

	bool is_init() const;
	bool is_loaded() const;
	bool is_started() const;

	void create_svc();
	void start_svc();
	void stop_svc();
	void unload_svc();

	~DriverLoader();
private:
	bool is_init_;
	bool is_loaded_;
	bool is_started_;

	LPTSTR file_path_;
	LPTSTR service_name_;
	LPTSTR display_name_;

	DWORD start_type_;

	SC_HANDLE service_;
};

