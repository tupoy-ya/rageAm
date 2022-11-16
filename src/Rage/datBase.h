#pragma once

class datBase
{
	datBase()
	{
		
	}
public:
	virtual datBase Create()
	{
		return datBase();
	}
};
