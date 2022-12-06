# Rage Hook

This is the place where most of 'memory editing' happening.

The base class template as follows:

```cpp
namespace rh
{
    class Module
    {
    public:
        Module()
        {
            // Hack stuff...
            // Everything you need is available in rageAm/memory namespace
        }
        
        // This static instance allows us to use C++ dynamic initialization
        // to execute hack code in constructor without any additional calls.
        // The only downside of this is that it can't be really chosen
        // when to execute it, because dynamic initialization happens
        // right after start of the application, (DLL Injection in our case)
        // so all rage hook classes are imported in the beginning in main.cpp file.
        inline Module g_Module;
    }
}
```
