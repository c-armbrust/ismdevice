#include <stdio.h>
#include <stdlib.h>
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "iothub_client.h"
#include "iothub_message.h"
#include "iothubtransportamqp.h"
#include "Commands.h"
#include <iostream>
#include <thread>
 
static const char* connectionString = "HostName=bbbiothub.azure-devices.net;DeviceId=test;SharedAccessKey=9Aei+0u4AgxH/7T73Ld5mjINk1MsuWaow9E6vB0HjG4=";

class Device
{
public:
static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
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
                    if(std::string{keys[index]} == "command")
                    {
                        std::string cmd{values[index]};
                        if(cmd == CommandType::START)
                        {
                            std::cout << "Start()\n";
                        }
                        else if(cmd == CommandType::STOP)
                        {
                            std::cout << "Stop()\n";
                        }
                        else if(cmd == CommandType::START_PREVIEW)
                        {
                            std::cout << "StartPreview()\n";
                        }
                        else if(cmd == CommandType::STOP_PREVIEW)
                        {
                            std::cout << "StopPreview()\n";
                        }
                        else if(cmd == CommandType::SET_DEVICE_SETTINGS)
                        {
                            std::cout << "SetDeviceSettings()\n";
                        }
                        else if(cmd == CommandType::GET_DEVICE_SETTINGS)
                        {
                            std::cout << "GetDeviceSettings()\n";
                        }
                        else
                        {

                        }
                    }
                }
            }

        }
    }

	((Device*)userContextCallback)->Test();
    
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

void Test()
{
	std::cout << "call Test\n";
}


void ReceiveC2D()
{
	platform_init();
    IOTHUB_CLIENT_HANDLE iotHubClientHandle;
    iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString, AMQP_Protocol);    IoTHubClient_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, this);
	
	while(1)
    {   
        ThreadAPI_Sleep(1000);
    }
}

}; // class

int main()
{
	Device d;
	std::thread receivec2d([&]{ d.ReceiveC2D(); });
	receivec2d.join();

return 0;
}
