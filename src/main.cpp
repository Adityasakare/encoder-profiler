#include <gst/gst.h>
#include <memory>
#include <iostream>
#include <stdlib>

/*
===========================================
Deleters - one per Gstreamer resource type
===========================================
*/

struct GstElementDeleter
{
    void operator()(GstElement* e)
    {
        if(e)
            gst_object_unref(e);
    }
};


struct GstBusDeleter
{
    void operator()(GstBus* b)
    {
        if(b)
            gst_object_unref(b);
    }
};

struct GstCapsDeleter
{
    void operator()(GstCaps* c)
    {
        if(c)
            gst_caps_unref(c);
    }
};

struct GstLoopDeleter
{
    void operator()(GMainLoop* m)
    {
        if(m)
            g_main_loop_unref(m);
    }
};


/*
===========================================
Typedef - for smart pointer
===========================================
*/

typedef std::unique_ptr<GstElement, GstElementDeleter> GstElementPtr;
typedef std::unique_ptr<GstBus, GstBusDeleter> GstBusPtr;
typedef std::unique_ptr<GstCaps, GstCapsDeleter> GstCapsPtr;
typedef std::unique_ptr<GMainLoop, GstLoopDeleter> GMainLoopPtr;

/*
===========================================
Application State - passed into bus callback
===========================================
*/

struct AppData
{
    GMainLoop* loop;        // raw ptr
    gboolean had_error;

    AppData() : loop(nullptr), had_error(false) {}
};


/*
===========================================
Helper - create elements
===========================================
*/

static GstElement* make_element(const char* factory, const char* name)
{
    GstElement* e = gst_element_factory_make(factory, name);
    if(!e)
    {
        std::cerr << "[ERROR]: failed to create: " << factory << std::endl;
        std::Exit(1);
    }
    return e;
}


/*
===========================================
main - Driver code
===========================================
*/

int main(int argc, char* argv[])
{
    
}