/* This entire file is licensed under GNU General Public License v3.0
 *
 * Copyright 2022 sfwbar maintainers
 */

#include <glib.h>
#include <gdk/gdkwayland.h>
#include "xdg-output-unstable-v1.h"

static struct zxdg_output_manager_v1 *xdg_output_manager;

static void xdg_output_noop ()
{
}

static void xdg_output_handle_name ( void *monitor,
    struct zxdg_output_v1 *xdg_output, const gchar *name )
{
  g_free(g_object_get_data(G_OBJECT(monitor),"xdg_name"));
  g_object_set_data(G_OBJECT(monitor),"xdg_name",strdup(name));
}

static const struct zxdg_output_v1_listener xdg_output_listener = {
  .logical_position = xdg_output_noop,
  .logical_size = xdg_output_noop,
  .done = xdg_output_noop,
  .name = xdg_output_handle_name,
  .description = xdg_output_noop,
};

void xdg_output_new ( GdkMonitor *monitor )
{
  struct wl_output *output;
  struct zxdg_output_v1 *xdg;

  if(!monitor || !xdg_output_manager)
    return;

  output = gdk_wayland_monitor_get_wl_output(monitor);

  if(!output)
    return;

  xdg = zxdg_output_manager_v1_get_xdg_output(xdg_output_manager, output);

  if(xdg)
  {
    zxdg_output_v1_add_listener(xdg,&xdg_output_listener,monitor);
    g_object_set_data(G_OBJECT(monitor),"xdg_output",xdg);
  }
}

void xdg_output_destroy ( GdkMonitor *gmon )
{
  struct zxdg_output_v1 *xdg;

  if(!gmon || !xdg_output_manager)
    return;

  xdg = g_object_get_data(G_OBJECT(gmon),"xdg_output");

  if(xdg)
    zxdg_output_v1_destroy(xdg);
}

void xdg_output_register (struct wl_registry *registry, uint32_t name)
{
  GdkDisplay *display;
  gint i,n;

  xdg_output_manager = wl_registry_bind(registry, name,
      &zxdg_output_manager_v1_interface, ZXDG_OUTPUT_V1_NAME_SINCE_VERSION);
  if(!xdg_output_manager)
    return;

  display = gdk_display_get_default();
  n = gdk_display_get_n_monitors(display);
  for(i=0;i<n;i++)
    xdg_output_new(gdk_display_get_monitor(display,i));
  wl_display_roundtrip(gdk_wayland_display_get_wl_display(display));
}
