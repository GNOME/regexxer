/* $Id$
 *
 * Copyright (c) 2004  Daniel Elstner  <daniel.elstner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License VERSION 2 as
 * published by the Free Software Foundation.  You are not allowed to
 * use any other version of the license; unless you got the explicit
 * permission from the author to do so.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "globalstrings.h"
#include "mainwindow.h"
#include "miscutils.h"
#include "translation.h"

#include <glib.h>
#include <glibmm.h>
#include <gconfmm.h>
#include <gtkmm/iconfactory.h>
#include <gtkmm/iconset.h>
#include <gtkmm/iconsource.h>
#include <gtkmm/main.h>
#include <gtkmm/stock.h>
#include <gtkmm/stockitem.h>
#include <gtkmm/window.h>

#include <exception>
#include <list>

#include <config.h>


namespace
{

/*
 * Include inlined raw pixbuf data generated by gdk-pixbuf-csource.
 */
#include <pixmaps/stockimages.h>


struct StockIconData
{
  const guint8*         data;
  int                   length;
  Gtk::BuiltinIconSize  size;
};

struct StockItemData
{
  const char*           id;
  const StockIconData*  icons;
  int                   n_icons;
  const char*           label;
};


const StockIconData stock_icon_about[] =
{
  { stock_menu_about, sizeof(stock_menu_about), Gtk::ICON_SIZE_MENU }
};

const StockIconData stock_icon_save_all[] =
{
  { stock_save_all_16, sizeof(stock_save_all_16), Gtk::ICON_SIZE_MENU          },
  { stock_save_all_24, sizeof(stock_save_all_24), Gtk::ICON_SIZE_SMALL_TOOLBAR }
};

const StockItemData regexxer_stock_items[] =
{
  { "regexxer-about",    stock_icon_about,    G_N_ELEMENTS(stock_icon_about),    N_("_About")    },
  { "regexxer-save-all", stock_icon_save_all, G_N_ELEMENTS(stock_icon_save_all), N_("Save _all") }
};


void register_stock_items()
{
  const Glib::RefPtr<Gtk::IconFactory> factory = Gtk::IconFactory::create();
  const Glib::ustring domain = PACKAGE_TARNAME;

  for (unsigned int item = 0; item < G_N_ELEMENTS(regexxer_stock_items); ++item)
  {
    const StockItemData& stock = regexxer_stock_items[item];
    Gtk::IconSet icon_set;

    for (int icon = 0; icon < stock.n_icons; ++icon)
    {
      const StockIconData& icon_data = stock.icons[icon];
      Gtk::IconSource source;

      source.set_pixbuf(Gdk::Pixbuf::create_from_inline(icon_data.length, icon_data.data));
      source.set_size(icon_data.size);

      // Unset wildcarded for all but the the last icon.
      source.set_size_wildcarded(icon == stock.n_icons - 1);

      icon_set.add_source(source);
    }

    const Gtk::StockID stock_id (stock.id);

    factory->add(stock_id, icon_set);
    Gtk::Stock::add(Gtk::StockItem(stock_id, stock.label, Gdk::ModifierType(0), 0, domain));
  }

  factory->add_default();
}

void set_default_window_icon()
{
  try
  {
    Gtk::Window::set_default_icon_from_file(Regexxer::application_icon_filename);
  }
  catch (const Glib::Error& error)
  {
    const Glib::ustring what = error.what();
    g_warning("%s", what.c_str());
  }
}

void trap_gconf_exceptions()
{
  try
  {
    throw; // re-throw current exception
  }
  catch (const Gnome::Conf::Error&)
  {
    // Ignore GConf exceptions thrown from GObject signal handlers.
    // GConf itself is going print the warning message for us
    // since we set the error handling mode to CLIENT_HANDLE_ALL.
  }
}

void initialize_configuration()
{
  using namespace Gnome::Conf;

  Glib::add_exception_handler(&trap_gconf_exceptions);

  const Glib::RefPtr<Client> client = Client::get_default_client();

  client->set_error_handling(CLIENT_HANDLE_ALL);
  client->add_dir(REGEXXER_GCONF_DIRECTORY, CLIENT_PRELOAD_ONELEVEL);

  const std::list<Entry> entries (client->all_entries(REGEXXER_GCONF_DIRECTORY));

  // Issue an artificial value_changed() signal for each entry in /apps/regexxer.
  // Reusing the signal handlers this way neatly avoids the need for separate
  // startup-initialization routines.

  for (std::list<Entry>::const_iterator p = entries.begin(); p != entries.end(); ++p)
  {
    client->value_changed(p->get_key(), p->get_value());
  }
}

} // anonymous namespace


int main(int argc, char** argv)
{
  try
  {
    Gnome::Conf::init();
    Gtk::Main main_instance (&argc, &argv);

    Util::initialize_gettext(PACKAGE_TARNAME, REGEXXER_LOCALEDIR);

    Glib::set_application_name(PACKAGE_NAME);
    register_stock_items();
    set_default_window_icon();

    Regexxer::MainWindow window;

    initialize_configuration();
    Gtk::Main::run(*window.get_window());
  }
  catch (const Glib::Error& error)
  {
    const Glib::ustring what = error.what();
    g_error("unhandled exception: %s", what.c_str());
  }
  catch (const std::exception& ex)
  {
    g_error("unhandled exception: %s", ex.what());
  }
  catch (...)
  {
    g_error("unhandled exception: (type unknown)");
  }

  return 0;
}

