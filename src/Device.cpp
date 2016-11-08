#include "Device.h"
#include "Singleton.h"
#include "Commands.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

const char* Device::connectionString = "[connection string]";

Device::Device()
{
	_state = &Singleton<ReadyState>::Instance();
}

void Device::ChangeState(DeviceState* s)
{
	_state = s;
}

void Device::Start()
{
	_state->Start(this);
}

void Device::Stop()
{
	_state->Stop(this);
}

void Device::StartPreview()
{
	_state->StartPreview(this);
}

void Device::StopPreview()
{
	_state->StopPreview(this);
}

void Device::SetDeviceSettings()
{
	_state->SetDeviceSettings(this);
}



void DeviceState::Start(Device*){}
void DeviceState::Stop(Device*){}
void DeviceState::StartPreview(Device*){}
void DeviceState::StopPreview(Device*){}
void DeviceState::SetDeviceSettings(Device*){}
void DeviceState::ChangeState(Device* d, DeviceState* s)
{
	d->ChangeState(s);
}



void Device::ReceiveC2D()
{
	platform_init();
    IOTHUB_CLIENT_HANDLE iotHubClientHandle;
    iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString, AMQP_Protocol);    IoTHubClient_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, this);
	
	while(1)
    {   
        ThreadAPI_Sleep(1000);
    }
}



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
					// Handle only events with key EventType::COMMAND
                    if(std::string{keys[index]} == EventType::COMMAND)
                    {
						// Switched by CommandType::<command> delegate to _state how to handle the command
                        std::string cmd{values[index]};
                        if(cmd == CommandType::START)
                        {
							d->_state->Start(d);
                        }
                        else if(cmd == CommandType::STOP)
                        {
							d->_state->Stop(d);
                        }
                        else if(cmd == CommandType::START_PREVIEW)
                        {
                            d->_state->StartPreview(d);
                        }
                        else if(cmd == CommandType::STOP_PREVIEW)
                        {
                            d->_state->StopPreview(d);
                        }
                        else if(cmd == CommandType::SET_DEVICE_SETTINGS)
                        {
                            d->_state->SetDeviceSettings(d);
                        }
                        else if(cmd == CommandType::GET_DEVICE_SETTINGS)
                        {
//                            d->_state->GetDeviceSettings(d); 
                        }
                        else
                        {

                        }
                    }
                }
            }

        }
    }

 
	// Message Data
	//
    size_t size;
    if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) == IOTHUB_MESSAGE_OK)
    {
        std::string msg{buffer};
        std::cout << "Received Message with Data: " << msg << " & Size=" << size << std::endl;
    }

    return IOTHUBMESSAGE_ACCEPTED;
}



ReadyState::ReadyState(){}
ReadyState::~ReadyState(){}
ReadyState::ReadyState(ReadyState const& s){}
ReadyState& ReadyState::operator=(ReadyState const& s){}

void ReadyState::Start(Device* d)
{
    // ACK msg
    // start timer
    std::cout << "+ Starting to run device!" << std::endl;
	ChangeState(d, &Singleton<RunState>::Instance());
}

void ReadyState::Stop(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running!" << std::endl;
}

void ReadyState::StartPreview(Device* d)
{
    // ACK msg
    // start timer
    std::cout << "+ Starting to run device in preview mode!" << std::endl;
	ChangeState(d, &Singleton<PreviewState>::Instance());
}

void ReadyState::StopPreview(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running!" << std::endl;
}

void ReadyState::SetDeviceSettings(Device* d)
{
    // deserialize msg
    // set new DeviceSettings values
    // ACK msg
    std::cout << "Set new DeviceSettings values!" << std::endl;
}

void ReadyState::DoWork(Device* d)
{

}



RunState::RunState(){}
RunState::~RunState(){}
RunState::RunState(RunState const& s){}
RunState& RunState::operator=(RunState const&){}

void RunState::Start(Device* d)
{
    // NAK msg
    std::cout << "- Device is already running!" << std::endl;
}

void RunState::Stop(Device* d)
{
    // ACK msg
    // stop timer
    std::cout << "+ Stop running the device and go back to ready!" << std::endl;
	ChangeState(d, &Singleton<ReadyState>::Instance());
}

void RunState::StartPreview(Device* d)
{
    // NAK msg
    std::cout << "- Can't go to preview mode from running device, stop it first!" << std::endl;
}

void RunState::StopPreview(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running in preview mode!" << std::endl;
}

void RunState::SetDeviceSettings(Device* d)
{
    // NAK msg
    std::cout << "Reject setting DeviceSettings values in RunState!" << std::endl;
}

void RunState::DoWork(Device* d)
{
    // TODO
}



PreviewState::PreviewState(){}
PreviewState::~PreviewState(){}
PreviewState::PreviewState(PreviewState const& s){}
PreviewState& PreviewState::operator=(PreviewState const& s){}

void PreviewState::Start(Device* d)
{
    // NAK msg
    std::cout << "- Can't go to run mode from preview mode device, stop preview first!" << std::endl;
}

void PreviewState::Stop(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running in run mode!" << std::endl;
}

void PreviewState::StartPreview(Device* d)
{
    // NAK msg
    std::cout << "- Device is already running in preview mode!" << std::endl;
}

void PreviewState::StopPreview(Device* d)
{
    // ACK msg
    // stop timer
    std::cout << "+ Stop running the preview and go back to ready!" << std::endl;
	ChangeState(d, &Singleton<ReadyState>::Instance());
}

void PreviewState::SetDeviceSettings(Device* d)
{
    // deserialize msg
    // set new DeviceSettings values
    // ACK msg
    std::cout << "Set new DeviceSettings values!" << std::endl;
}

void PreviewState::DoWork(Device* d)
{
    // TODO
}
