

#include <string>
#include <iostream>

#include <FL/x.H>

// include the URI and global data of this plugin
#include "../dsp/masha.hxx"

// this is our custom widget include
#include "masha_widget.h"

// core spec include
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

// GUI
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include "../../avtk/avtk.h"

using namespace std;

typedef struct {
  MashaUI* widget;
  
  float sidechainAmp;
  
  LV2UI_Write_Function write_function;
  LV2UI_Controller controller;
} MashaGUI;

static LV2UI_Handle instantiate(const struct _LV2UI_Descriptor * descriptor,
                const char * plugin_uri,
                const char * bundle_path,
                LV2UI_Write_Function write_function,
                LV2UI_Controller controller,
                LV2UI_Widget * widget,
                const LV2_Feature * const * features)
{
  if (strcmp(plugin_uri, MASHA_URI) != 0) {
      fprintf(stderr, "MASHA_URI error: this GUI does not support plugin with URI %s\n", plugin_uri);
      return NULL;
  }
  
  MashaGUI* self = (MashaGUI*)malloc(sizeof(MashaGUI));
  if (self == NULL) return NULL;
  
  self->controller     = controller;
  self->write_function = write_function;
  
  void* parentXwindow = 0;
  LV2UI_Resize* resize = NULL;
  
  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_UI__parent)) {
      parentXwindow = features[i]->data;
    } else if (!strcmp(features[i]->URI, LV2_UI__resize)) {
      resize = (LV2UI_Resize*)features[i]->data;
    }
  }
  
  // in case FLTK hasn't opened it yet
  fl_open_display();
  
  self->widget = new MashaUI();
  
  self->widget->window->border(0);
  
  
  // write functions into the widget
  self->widget->controller = controller;
  self->widget->write_function = write_function;
  
  // set host to change size of the window
  if (resize)
  {
    resize->ui_resize(resize->handle, self->widget->getWidth(), self->widget->getHeight());
  }
  else
  {
    cout << "MashaUI: Warning, host doesn't support resize extension.\n\
    Please ask the developers of the host to support this extension. "<< endl;
  }
  
  fl_embed( self->widget->window, (Window)parentXwindow );
  
  
  return (LV2UI_Handle)self;
}



static void cleanup(LV2UI_Handle ui) {
  MashaGUI *pluginGui = (MashaGUI *) ui;
  delete pluginGui->widget;
  free( pluginGui);
}

static void port_event(LV2UI_Handle ui,
               uint32_t port_index,
               uint32_t buffer_size,
               uint32_t format,
               const void * buffer)
{
  MashaGUI *self = (MashaGUI *) ui;
  
  
  if ( format == 0 )
  {
    float value =  *(float *)buffer;
    switch ( port_index )
    {
      case MASHA_TIME:
          {
            self->widget->graph->duration( value );
            self->widget->time->value( value );
          }
          break;
      case MASHA_AMP:
          {
            self->widget->graph->volume( value );
            self->widget->volume->value( value );
          }
          break;
      case MASHA_DRY_WET:
          {
            self->widget->graph->replace( value );
            self->widget->replace->value( value );
          }
          break;
      default:
          break;
    }
  }
  
  
  return;
}


static int
idle(LV2UI_Handle handle)
{
  MashaGUI* self = (MashaGUI*)handle;
  
  self->widget->idle();
  
  return 0;
}

static const LV2UI_Idle_Interface idle_iface = { idle };

static const void*
extension_data(const char* uri)
{
  if (!strcmp(uri, LV2_UI__idleInterface))
  {
    return &idle_iface;
  }
  return NULL;
}

static LV2UI_Descriptor descriptors[] = {
    {MASHA_UI_URI, instantiate, cleanup, port_event, extension_data}
};

const LV2UI_Descriptor * lv2ui_descriptor(uint32_t index)
{
  if (index >= sizeof(descriptors) / sizeof(descriptors[0]))
  {
      return NULL;
  }
  return descriptors + index;
}