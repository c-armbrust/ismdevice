#include "Commands.h"

const std::string CommandType::UNPROVISION{"Unprovision"};
const std::string CommandType::PROVISION{"Provision"};
const std::string CommandType::START{"Start"};
const std::string CommandType::STOP{"Stop"};
const std::string CommandType::START_PREVIEW{"StartPreview"};
const std::string CommandType::STOP_PREVIEW{"StopPreview"};
const std::string CommandType::GET_DEVICE_SETTINGS{"GetDeviceSettings"};
const std::string CommandType::SET_DEVICE_SETTINGS{"SetDeviceSettings"};
const std::string CommandType::DAT{"DAT"};
const std::string CommandType::PRV{"PRV"};
const std::string CommandType::UPDATE_DASHBOARD_CONTROLS{"UpdateDashboardControls"};

const std::string CommandStatus::SUCCESS{"Success"};
const std::string CommandStatus::FAILURE{"Failure"};
const std::string CommandStatus::PENDING{"Pending"};
