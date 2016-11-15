#include "Device.h"
#include "Singleton.h"
#include "Commands.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "json.hpp"

const char* Device::connectionString = "[connection string]";

Device::Device()
{
	_state = &Singleton<ReadyState>::Instance();
	settings = new DeviceSettings(this->getDeviceId(), _state->getStateName(), 10, "", 
								  0.0025, 8.5, 3.75, 4, 16,
								  0, 0, 
								  0, 0, 0, 0);
	platform_init();
	iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString, AMQP_Protocol);
	
}

void Device::ChangeState(DeviceState* s)
{
	_state = s;
}

void Device::UpdateSettings(std::string msgbody)
{
	std::cout << "\nold settings:" << std::endl;
	settings->Report();
	settings->Deserialize(msgbody);
	std::cout << "\nnew settings:" << std::endl;
	settings->Report();
}

// DeviceId is part of the connection string
std::string Device::getDeviceId()
{
	std::string connStr = std::string{connectionString};
	std::string searchPattern = "DeviceId=";
	std::size_t pos_begin = connStr.find(searchPattern) + searchPattern.length();
	std::size_t pos_end = connStr.find(";", pos_begin+1);
	return connStr.substr(pos_begin, pos_end - pos_begin);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::Start()
{
	return _state->Start(this);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::Stop()
{
	return _state->Stop(this);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::StartPreview()
{
	return _state->StartPreview(this);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::StopPreview()
{
	return _state->StopPreview(this);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::SetDeviceSettings(std::string msgbody)
{
	return _state->SetDeviceSettings(this, msgbody);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::GetDeviceSettings()
{
	return _state->GetDeviceSettings(this);
}



IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::Start(Device*){}
IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::Stop(Device*){}
IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::StartPreview(Device*){}
IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::StopPreview(Device*){}
IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::SetDeviceSettings(Device*, std::string){}
IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::GetDeviceSettings(Device*){}
void DeviceState::DoWork(Device*){}



//  
void DeviceState::ChangeState(Device* d, DeviceState* s)
{
	d->ChangeState(s);
}

void DeviceState::UpdateSettings(Device* d, std::string msgbody)
{
	d->UpdateSettings(msgbody);	
}

void DeviceState::SendD2C_DeviceSettings(Device* d)
{
	d->SendD2C_DeviceSettings();
}



void Device::ReceiveC2D()
{
	IoTHubClient_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, this);
	
	while(1)
    {   
        ThreadAPI_Sleep(1000);
    }
}

void Device::SendD2C_DeviceSettings()
{
    std::thread t([&]{
	
		IOTHUB_MESSAGE_HANDLE messageHandle; 	
		char sendBuffer[MAX_SEND_BUFFER_SIZE];
	
		// fill send buffer	
		sprintf_s(sendBuffer, MAX_SEND_BUFFER_SIZE, settings->Serialize().c_str());
		messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)sendBuffer, strlen(sendBuffer));
		
		// set event properties	
		MAP_HANDLE propMap = IoTHubMessage_Properties(messageHandle);
		Map_AddOrUpdate(propMap, EventType::D2C_COMMAND.c_str(), CommandType::UPDATE_DASHBOARD_CONTROLS.c_str());
	
		std::cout << "send message, size=" << strlen(sendBuffer) << std::endl;
	
		// send the message
		IoTHubClient_SendEventAsync(iotHubClientHandle, messageHandle, SendConfirmationCallback, this);
	});
	
	t.detach();
}


void Device::SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
	//Device* d = (Device*)userContextCallback;
	
	std::cout << "send confirmed." << std::endl;
}



// TODO: free memory of e.g buffer?
IOTHUBMESSAGE_DISPOSITION_RESULT Device::ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
	Device* d = (Device*)userContextCallback;
    // Message Properties
    //
    const char* buffer;
    MAP_HANDLE mapProperties = IoTHubMessage_Properties(message);
    if(mapProperties != nullptr)
    {
        const char*const* keys;
        const char*const* values;
        size_t propertyCount = 0;
        if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
        {
            if (propertyCount > 0)
            {
                for(size_t index = 0; index < propertyCount; index++)
                {
					// Filter all iot hub events on a high level. 
					// Handle only events with key EventType::C2D_COMMAND
                    if(std::string{keys[index]} == EventType::C2D_COMMAND)
                    {
						// Message Data
						//
    					size_t size;
    					if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) == IOTHUB_MESSAGE_OK)
    					{
							// !!! provide the exact size to std::string ctor 
        					std::string msgbody{buffer, 0, size};
	
							// Switched by CommandType::<command> delegate to _state how to handle the command
                        	std::string cmd{values[index]};
                        	if(cmd == CommandType::START)
                        	{
								return d->Start();
                        	}
                       		else if(cmd == CommandType::STOP)
                        	{
								return d->Stop();
                        	}
                        	else if(cmd == CommandType::START_PREVIEW)
                        	{
								return d->StartPreview();
                        	}
                        	else if(cmd == CommandType::STOP_PREVIEW)
                        	{
								return d->StopPreview();
                        	}
                        	else if(cmd == CommandType::SET_DEVICE_SETTINGS)
                        	{
								return d->SetDeviceSettings(msgbody);
                        	}
                        	else if(cmd == CommandType::GET_DEVICE_SETTINGS)
                    	    {
              	              	return d->GetDeviceSettings(); 
            	            }
        	                else
    	                    {
								std::cout << "- Unknown CommandType" << std::endl;	
                        	}
						}
                    }
                }
            }
        }
    }

	std::cout << "ReceiveMessageCallback returns with default IOTHUBMESSAGE_REJECTED." << std::endl;	
	return IOTHUBMESSAGE_REJECTED;
}



// ReadyState
//
ReadyState::ReadyState(){}
ReadyState::~ReadyState(){}
ReadyState::ReadyState(ReadyState const& s){}
ReadyState& ReadyState::operator=(ReadyState const& s){}

std::string ReadyState::getStateName() {return "ReadyState";}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::Start(Device* d)
{
    // ACK msg
    // start timer
    std::cout << "+ Starting to run device!" << std::endl;
	ChangeState(d, &Singleton<RunState>::Instance());

	return IOTHUBMESSAGE_ACCEPTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::Stop(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::StartPreview(Device* d)
{
    // ACK msg
    // start timer
    std::cout << "+ Starting to run device in preview mode!" << std::endl;
	ChangeState(d, &Singleton<PreviewState>::Instance());

	return IOTHUBMESSAGE_ACCEPTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::StopPreview(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running!" << std::endl;
	
	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::SetDeviceSettings(Device* d, std::string msgbody)
{
    // deserialize msg
    // set new DeviceSettings values
    // ACK msg
    std::cout << "+ READY_STATE: Set new DeviceSettings values!" << std::endl;
	
	UpdateSettings(d, msgbody);

	return IOTHUBMESSAGE_ACCEPTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::GetDeviceSettings(Device* d)
{
	// ACK msg
	std::cout << "+ Send DeviceSettings D2C message" << std::endl;

	//std::thread t([&]{SendD2C_DeviceSettings(d);});
	//t.join();
	SendD2C_DeviceSettings(d);
	return IOTHUBMESSAGE_ACCEPTED;
}

void ReadyState::DoWork(Device* d)
{

}



// RunState
//
RunState::RunState(){}
RunState::~RunState(){}
RunState::RunState(RunState const& s){}
RunState& RunState::operator=(RunState const&){}

std::string RunState::getStateName() {return "RunState";}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::Start(Device* d)
{
    // NAK msg
    std::cout << "- Device is already running!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::Stop(Device* d)
{
    // ACK msg
    // stop timer
    std::cout << "+ Stop running the device and go back to ready!" << std::endl;
	ChangeState(d, &Singleton<ReadyState>::Instance());

	return IOTHUBMESSAGE_ACCEPTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::StartPreview(Device* d)
{
    // NAK msg
    std::cout << "- Can't go to preview mode from running device, stop it first!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::StopPreview(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running in preview mode!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::SetDeviceSettings(Device* d, std::string msgbody)
{
    // NAK msg
    std::cout << "- RUN_STATE: Reject setting DeviceSettings values in RunState!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::GetDeviceSettings(Device* d)
{
	// ACK msg
	std::cout << "+ Send DeviceSettings D2C message" << std::endl;

	return IOTHUBMESSAGE_ACCEPTED;
}

void RunState::DoWork(Device* d)
{
    // TODO
}



// PreviewState
//
PreviewState::PreviewState(){}
PreviewState::~PreviewState(){}
PreviewState::PreviewState(PreviewState const& s){}
PreviewState& PreviewState::operator=(PreviewState const& s){}

std::string PreviewState::getStateName() {return "PreviewState";}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::Start(Device* d)
{
    // NAK msg
    std::cout << "- Can't go to run mode from preview mode device, stop preview first!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::Stop(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running in run mode!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::StartPreview(Device* d)
{
    // NAK msg
    std::cout << "- Device is already running in preview mode!" << std::endl;
	
	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::StopPreview(Device* d)
{
    // ACK msg
    // stop timer
    std::cout << "+ Stop running the preview and go back to ready!" << std::endl;
	ChangeState(d, &Singleton<ReadyState>::Instance());

	return IOTHUBMESSAGE_ACCEPTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::SetDeviceSettings(Device* d, std::string msgbody)
{
    // deserialize msg
    // set new DeviceSettings values
    // ACK msg
    std::cout << "+ PREVIEW_STATE: Set new DeviceSettings values!" << std::endl;

	UpdateSettings(d, msgbody);

	return IOTHUBMESSAGE_ACCEPTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::GetDeviceSettings(Device* d)
{
	// ACK msg
	std::cout << "+ Send DeviceSettings D2C message" << std::endl;

	return IOTHUBMESSAGE_ACCEPTED;
}

void PreviewState::DoWork(Device* d)
{
    // TODO
}



// DeviceSettings
//
DeviceSettings::DeviceSettings()
{

}

DeviceSettings::DeviceSettings(std::string DeviceId, std::string StateName, int CapturePeriod, std::string CurrentCaptureUri, // general settings
							   double VarianceThreshold, double DistanceMapThreshold, double RGThreshold, double RestrictedFillingThreshold, double DilateValue,     // Matlab Algorithm Settings
							   int Brightness, int Exposure, 								   // camera settings
							   int PulseWidth, int Current, int Predelay, bool IsOn)        				   // pulse settings 
{
	this->DeviceId = DeviceId;
	this->StateName = StateName;
	this->CapturePeriod = CapturePeriod;
	this->CurrentCaptureUri = CurrentCaptureUri;
	this->VarianceThreshold = VarianceThreshold;
	this->DistanceMapThreshold = DistanceMapThreshold;
	this->RGThreshold = RGThreshold;
	this->RestrictedFillingThreshold = RestrictedFillingThreshold;
	this->DilateValue = DilateValue;
	this->Brightness = Brightness;
	this->Exposure = Exposure;
	this->PulseWidth = PulseWidth;
	this->Current = Current;
	this->Predelay = Predelay;
	this->IsOn = IsOn;
}

DeviceSettings::~DeviceSettings()
{
}

std::string DeviceSettings::Serialize()
{
	nlohmann::json obj = {
		{"DeviceId", DeviceId},
		{"StateName", StateName},
		{"CapturePeriod", CapturePeriod},
		{"CurrentCaptureUri", CurrentCaptureUri},
		{"VarianceThreshold", VarianceThreshold},
		{"DistanceMapThreshold", DistanceMapThreshold},
		{"RGThreshold", RGThreshold},
		{"RestrictedFillingThreshold", RestrictedFillingThreshold},
		{"DilateValue", DilateValue},
		{"Brightness", Brightness},	
		{"Exposure", Exposure},
		{"PulseWidth", PulseWidth},
		{"Current", Current},
		{"Predelay", Predelay},
		{"IsOn", IsOn}
	};
	
	return obj.dump();
}

void DeviceSettings::Deserialize(std::string jsonStr)
{
	nlohmann::json values  = nlohmann::json::parse(jsonStr);
	DeviceId = values["DeviceId"];
    StateName = values["StateName"];
    CapturePeriod = values["CapturePeriod"];
    CurrentCaptureUri = values["CurrentCaptureUri"];
    VarianceThreshold = values["VarianceThreshold"];
    DistanceMapThreshold = values["DistanceMapThreshold"];
    RGThreshold = values["RGThreshold"];
    RestrictedFillingThreshold = values["RestrictedFillingThreshold"];
    DilateValue = values["DilateValue"];
    Brightness = values["Brightness"];
    Exposure = values["Exposure"];
    PulseWidth = values["PulseWidth"];
    Current = values["Current"];
    Predelay = values["Predelay"];
    IsOn = values["IsOn"];	
}


void DeviceSettings::Report()
{
	std::cout << "DeviceId: " <<  DeviceId << std::endl;
	std::cout << "StateName: " << StateName << std::endl;
	std::cout << "CapturePeriod: " << CapturePeriod << std::endl;
	std::cout << "CurrentCaptureUri: " << CurrentCaptureUri << std::endl;
	std::cout << "VarianceThreshold: " << VarianceThreshold << std::endl;
	std::cout << "DistanceMapThreshold: " << DistanceMapThreshold << std::endl;
	std::cout << "RGThreshold: " << RGThreshold << std::endl;
	std::cout << "RestrictedFillingThreshold: " << RestrictedFillingThreshold << std::endl;
	std::cout << "DilateValue: " << DilateValue << std::endl;
	std::cout << "Brightness: " << Brightness << std::endl;
	std::cout << "Exposure: " << Exposure << std::endl;
	std::cout << "PulseWidth: " << PulseWidth << std::endl;
	std::cout << "Current: " << Current << std::endl;
	std::cout << "Predelay: " << Predelay << std::endl;
	std::cout << "IsOn: " << IsOn << std::endl;
}

