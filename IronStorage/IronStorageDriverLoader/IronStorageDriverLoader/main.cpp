#include <iostream>
#include "DriverLoader.h"

int main(int argc, char* argv[])
{
	DriverLoader* driver_loader;

	std::string file_path = "IronStorageDriver.sys";
	std::string service_name = "IronStorageDriver";
	std::string display_name = "IronStorageDriver";

	try
	{
		driver_loader = new DriverLoader(const_cast<char*>(file_path.c_str()),
										const_cast<char*>(service_name.c_str()),
										const_cast<char*>(display_name.c_str()),
													SERVICE_DEMAND_START);
		driver_loader->create_svc();
		driver_loader->start_svc();
	}
	catch (std::exception const& ex)
	{	
		std::cerr << ex.what() << std::endl;
		return -1;
	}

	return 0;
}
