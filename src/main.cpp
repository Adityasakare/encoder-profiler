#include <gst/gst.h>
#include <memory>
#include <iostream>
#include <stdlib.h>

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
        std::exit(1);
    }
    return e;
}

/*
===========================================
Bus callback
===========================================
*/

static gboolean on_bus_message(GstBus*, GstMessage* msg, gpointer user_data)
{
    AppData* app = static_cast<AppData*>(user_data);

    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_EOS:
        std::cout << "[bus] EOS - all frame encoded\n";
        g_main_loop_quit(app->loop);
        break;

    case GST_MESSAGE_ERROR:
    {
        GError* err = nullptr;
        gchar* debug = nullptr;
        gst_message_parse_error(msg, &err, &debug);
        std::cerr <<"[bus] ERROR: " << err->message << "\n";
        std::cerr <<"[bus] Debug: " << (debug ? debug : "-") << "\n";
        g_error_free(err);
        g_free(debug);
        app->had_error = TRUE;
        g_main_loop_quit(app->loop);
        break;
    }
    default:
        break;
    }

    return TRUE;
}





/*
===========================================
main - Driver code
===========================================
*/

int main(int argc, char* argv[])
{
    gst_init(&argc, &argv);

    AppData app;

    /* 1. Main Loop */
    GMainLoopPtr loop(g_main_loop_new(nullptr, FALSE));
    app.loop = loop.get();          // give raw pointer to callback

    /* 2. Pipeline */
    GstElementPtr pipeline(gst_pipeline_new("profiler"));
    if(!pipeline.get())
    {
        std::cerr << "[ERROR]: failed to create a pipeline\n";
        return 1;
    }

    /* 3. Create a child element for piepline */
    GstElement* src = make_element("videotestsrc", "src");
    GstElement* capsfilter = make_element("capsfilter", "caps");
    GstElement* encoder = make_element("x264enc", "enc");
    GstElement* sink = make_element("fakesink", "sink");

    /* 4. Set element properties */
    
    // src: strops after 300 frames 
    g_object_set(src, "num-buffers", 300, nullptr);

    // capsfilter : force 1980x1080 @ 30 fps
    GstCapsPtr caps(gst_caps_from_string(
        "video/x-raw,"
        "width=1920,"
        "height=1080,"
        "framerate=30/1"));
        g_object_set(capsfilter, "caps", caps.get(), nullptr);


        // encoder: ultrafast for now so test quikly
        g_object_set(encoder, "speed-preset", 1, nullptr);  // 1 = ultrafast
        g_object_set(encoder, "tune", 4, nullptr);          // 4 = zerolatency

        // sink: sync false for clock - run as fast as possible
        g_object_set(sink, "sync", false, nullptr); 

        /* 5. Add all elements into pipeline bin */
        gst_bin_add_many(GST_BIN(pipeline.get()),
                                src, capsfilter, encoder, sink,
                                nullptr);

        /* 6. Link element in data-flow order */
        gboolean linked = gst_element_link_many(
                                src, capsfilter, encoder, sink,
                                nullptr);      
        if(!linked)
        {
            std::cerr << "[ERROR] failed to link the pipeline\n";
            return 1;
        }

        /* 7. Attcahed bus watch */
        GstBusPtr bus(gst_element_get_bus(pipeline.get()));
        gst_bus_add_watch(bus.get(), on_bus_message, &app);

        /* 8. start pipeline */
        GstStateChangeReturn ret = gst_element_set_state(pipeline.get(), GST_STATE_PLAYING);
        if(ret == GST_STATE_CHANGE_FAILURE)
        {
            std::cerr << "[ERROR] failed to set PLAYING\n";
            return 1;
        }

        std::cout << "[main] pipeline PLAYING - encoding 300 frames\n";

        /* 9. Block until EOS */
        g_main_loop_run(loop.get());

        std::cout << "[main] done\n";

        /* 10. Exit */
        gst_element_set_state(pipeline.get(), GST_STATE_NULL);

        return app.had_error ? 1 : 0;
}

