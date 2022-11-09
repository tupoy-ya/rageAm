#pragma once

template<class T>
class atSingleton
{
private:
	static T _instance;
public:
	atSingleton() {} // Shouldn't be public...

	atSingleton(atSingleton const&) = delete;
	void operator=(atSingleton const&) = delete;

	inline static T* GetInstance() 
	{
		return (T*)&_instance;
	}

	~atSingleton()
	{
		//
	}
};

template<class T>
T atSingleton<T>::_instance = T();