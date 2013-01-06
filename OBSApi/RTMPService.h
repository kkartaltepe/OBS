#pragma once

class BASE_EXPORT RTMPService
{
public:
    virtual ~RTMPService() {};

	virtual void init(XElement* data){};

	virtual void onSelected(){};
	virtual void onDeselected(){};

	virtual String getRTMPURL();
	virtual String getPlayPath();
};