/**
@page lomse-callbacks How Lomse callbacks work

It is very important to understand how a callback works and how to use a class method as callback.

A callback function is just a function that is called through a function pointer, as in this example:

@code
void say_hello()
{
    printf("Hi there!");
}
 
int main ()
{
    void (*callback)(void);         //declare 'callback' as a ptr to a func. with no args
    callback = (void*)say_hello;    //instantiate it pointing to say_hello() function
    callback();                     //invoke say_hello()
    return 0;
}
@endcode

The problem with callbacks when using C++ is that a callback is just a C function, but <b>not a C++ class method</b>. As you know, to invoke an object's method you need a reference or pointer to the object, as in this example:

@code
 pObject->method();
@endcode

The only exception to this is when the method is static. In this case you will directly invoke the method as in:

@code
 object::method()
@endcode

Therefore, to use a method as callback it is necessary to pass two parameters, a pointer to the object instance, and a pointer to the method. But as the Lomse callback is a C callback, only one parameter is passed: the pointer to the method. As a consequence, only static methods can be invoked. But this is a very strong limitation as static members can only access static variables.

A simple solution used in Lomse, that works for both C and C++ programs, is to add another parameter to the function that sets up the callback. This parameter is a pointer to the object instance (for C++ programs) or NULL (for C programs). The callback is still an static method but it will receive the object instance pointer as parameter and, thus, non-static members can be invoked from inside the static method.

Example:

@code
class MyApp
{
public:
    //requests wrapper
    static void wrapper_for_lomse_requests(void* pThis, Request* pRequests)
    {
        static_cast<MyApp*>(pThis)->on_lomse_request(pRequest);
    }
    ...

protected:
    void on_lomse_request(Request* pRequest);
    ...
};
@endcode

And inform lomse about it. We do it at lomse initialization:

@code
void MyApp::initialize_lomse()
{
    //initialize the library
    ...
    m_lomse.init_library(pixel_format, resolution, reverse_y_axis);

    //set callbacks
    m_lomse.set_request_callback(this, wrapper_for_lomse_requests);

    ...
}
@endcode

*/


