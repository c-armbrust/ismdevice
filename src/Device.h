#ifndef DEVICE_H
#define DEVICE_H

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "iothub_client.h"
#include "iothub_message.h"
#include "iothubtransportamqp.h"
#include <thread>

class DeviceState;
template<class T> class Singleton;

class Device
{
public:
	Device();
	void Start();
	void Stop();
	void StartPreview();
	void StopPreview();
	void SetDeviceSettings();
	void GetDeviceSettings();
	static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE, void*);	 void ReceiveC2D();

private:
	friend class DeviceState;
	void ChangeState(DeviceState*);
private:
	DeviceState* _state;
	static const char* connectionString;
};



class DeviceState
{
public:
	virtual void Start(Device*)=0;
	virtual void Stop(Device*)=0;
	virtual void StartPreview(Device*)=0;
	virtual void StopPreview(Device*)=0;
	virtual void SetDeviceSettings(Device*)=0;
	virtual void DoWork(Device*)=0;

protected:
	void ChangeState(Device*, DeviceState*);
};



class ReadyState : public DeviceState
{
public:
	virtual void Start(Device*);
	virtual void Stop(Device*);
	virtual void StartPreview(Device*);
	virtual void StopPreview(Device*);
	virtual void SetDeviceSettings(Device*);
	virtual void DoWork(Device*);

private:
	template<class T> friend class Singleton;	
	ReadyState(); // ctor hidden
	~ReadyState(); // dtor hidden
	ReadyState(ReadyState const&); // cp ctor hidden
	ReadyState& operator=(ReadyState const&); // assign op hidden
};

class RunState : public DeviceState
{ 
public:  
    virtual void Start(Device*);
    virtual void Stop(Device*);                                                  
    virtual void StartPreview(Device*);
    virtual void StopPreview(Device*);                                                         
    virtual void SetDeviceSettings(Device*);
    virtual void DoWork(Device*);

private:
	template<class T> friend class Singleton;
	RunState(); // ctor hidden
	~RunState(); // dtor hidden
	RunState(RunState const&); // cp ctor hidden
	RunState& operator=(RunState const&); // assign op hidden
};

class PreviewState : public DeviceState
{ 
public:  
    virtual void Start(Device*);
    virtual void Stop(Device*);                                                  
    virtual void StartPreview(Device*);
    virtual void StopPreview(Device*);                                                         
    virtual void SetDeviceSettings(Device*);
    virtual void DoWork(Device*);

private:
	template<class T> friend class Singleton;
	PreviewState(); // ctor hidden
	~PreviewState(); // dtor hidden
	PreviewState(PreviewState const&); // cp ctor hidden
	PreviewState& operator=(PreviewState const&); // assign op hidden
};

#endif
