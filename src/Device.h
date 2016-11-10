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
class DeviceSettings;
template<class T> class Singleton;

class Device
{
public:
	Device();
	IOTHUBMESSAGE_DISPOSITION_RESULT Start();
	IOTHUBMESSAGE_DISPOSITION_RESULT Stop();
	IOTHUBMESSAGE_DISPOSITION_RESULT StartPreview();
	IOTHUBMESSAGE_DISPOSITION_RESULT StopPreview();
	IOTHUBMESSAGE_DISPOSITION_RESULT SetDeviceSettings(std::string);
	IOTHUBMESSAGE_DISPOSITION_RESULT GetDeviceSettings();
	
void ReceiveC2D();
	

private:
	friend class DeviceState;

	void ChangeState(DeviceState*);
	void UpdateSettings(std::string);
	std::string getDeviceId();

	static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE, void*);

private:
	DeviceState* _state;
	static const char* connectionString;
	DeviceSettings* settings;
};



class DeviceState
{
public:
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT Start(Device*)=0;
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT Stop(Device*)=0;
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT StartPreview(Device*)=0;
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT StopPreview(Device*)=0;
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT SetDeviceSettings(Device*, std::string)=0;
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT GetDeviceSettings(Device*)=0;
	virtual void DoWork(Device*)=0;
	virtual std::string getStateName()=0;

protected:
	void ChangeState(Device*, DeviceState*);
	void UpdateSettings(Device*, std::string);
};



class ReadyState : public DeviceState
{
public:
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT Start(Device*);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT Stop(Device*);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT StartPreview(Device*);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT StopPreview(Device*);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT SetDeviceSettings(Device*, std::string);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT GetDeviceSettings(Device*);
	virtual void DoWork(Device*);
	virtual std::string getStateName();

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
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT Start(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT Stop(Device*);                                     
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT StartPreview(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT StopPreview(Device*);                                
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT SetDeviceSettings(Device*, std::string);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT GetDeviceSettings(Device*);	
    virtual void DoWork(Device*);
	virtual std::string getStateName();

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
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT Start(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT Stop(Device*);                                       
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT StartPreview(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT StopPreview(Device*);                             
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT SetDeviceSettings(Device*, std::string);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT GetDeviceSettings(Device*);
    virtual void DoWork(Device*);
	virtual std::string getStateName();

private:
	template<class T> friend class Singleton;
	PreviewState(); // ctor hidden
	~PreviewState(); // dtor hidden
	PreviewState(PreviewState const&); // cp ctor hidden
	PreviewState& operator=(PreviewState const&); // assign op hidden
};



class  DeviceSettings
{
public:
	DeviceSettings();
	DeviceSettings(std::string, std::string, int, std::string, // general settings
				   double, double, double, double, double,	   // Matlab Algorithm settings
				   int, int,								   // camera settings
				   int, int, int, bool);					   // pulse settings
	DeviceSettings(DeviceSettings const&);
	DeviceSettings& operator=(DeviceSettings const&);
	~DeviceSettings();

// Serialization / Deserialization
	std::string Serialize();
	void Deserialize(std::string);
	void Report();

// get/set methods
public:
	inline std::string getDeviceId() {return DeviceId;}
	inline std::string getStateName() {return StateName;}
	inline int getCapturePeriod() {return CapturePeriod;}
	inline std::string getCurrentCaptureUri() {return CurrentCaptureUri;}
	inline double getVarianceThreshold() {return VarianceThreshold;}
	inline double getDistanceMapThreshold() {return DistanceMapThreshold;}
	inline double getRGThreshold() {return RGThreshold;}
	inline double getRestrictedFillingThreshold() {return RestrictedFillingThreshold;}
	inline double getDilateValue() {return DilateValue;}
	inline int getBrightness() {return Brightness;}
	inline int getExposure() {return Exposure;}
	inline int getPulseWidth() {return PulseWidth;}
	inline int getCurrent() {return Current;}
	inline int getPredelay() {return Predelay;}
	inline bool getIsOn() {return IsOn;}

	inline void setDeviceId(std::string v) {DeviceId = v;}
	inline void setStateName(std::string v) {StateName = v;}
	inline void setCapturePeriod(int v) {CapturePeriod = v;}
	inline void setCurrentCaptureUri(std::string v) {CurrentCaptureUri = v;}
	inline void setVarianceThreshold(double v) {VarianceThreshold = v;}
	inline void setDistanceMapThreshold(double v) {DistanceMapThreshold = v;}
	inline void setRGThreshold(double v) {RGThreshold = v;}
	inline void setRestrictedFillingThreshold(double v) {RestrictedFillingThreshold = v;}
	inline void setDilateValue(double v) {DilateValue = v;}
	inline void setBrightness(int v) {Brightness = v;}
	inline void setExposure(int v) {Exposure = v;}
	inline void setPulseWidth(int v) {PulseWidth = v;}
	inline void setCurrent(int v) {Current = v;}
	inline void setPredelay(int v) {Predelay = v;}
	inline void setIsOn(int v) {IsOn = v;}

private:
	std::string DeviceId;
	std::string StateName;
	int CapturePeriod;
	std::string CurrentCaptureUri;
	
	// Matlab Filament-Algorithm params
	double VarianceThreshold;
	double DistanceMapThreshold;
	double RGThreshold;
	double RestrictedFillingThreshold;
	double DilateValue;

	// Camera Settings
	int Brightness;
	int Exposure;

	// Pulse Settings
	int PulseWidth;
	int Current;
	int Predelay;
	bool IsOn;
};

#endif
