#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>

struct CommandType
{
public:
	// Device identity registry Commands
	static const  std::string UNPROVISION;
	static const std::string PROVISION;
	
	// C2D Commands
	static const std::string START;
	static const std::string STOP;
	static const std::string START_PREVIEW;
	static const std::string STOP_PREVIEW;
	
	// C2D Dashboard Commands
	static const std::string GET_DEVICE_SETTINGS;
	static const std::string SET_DEVICE_SETTINGS;
	
	// D2C Commands
	static const std::string DAT;
	static const std::string PRV;

	// D2C Dashboard Commands 
	static const std::string UPDATE_DASHBOARD_CONTROLS;
};


struct CommandStatus
{
public:
	static const std::string SUCCESS;
	static const std::string FAILURE;
	static const std::string PENDING;
};

#endif