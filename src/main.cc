/* $Id$
 *
 * Copyright (c) 2002  Daniel Elstner  <daniel.elstner@gmx.net>
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

#include "mainwindow.h"

#include <glib.h>
#include <gtkmm/iconfactory.h>
#include <gtkmm/iconset.h>
#include <gtkmm/iconsource.h>
#include <gtkmm/main.h>
#include <gtkmm/stock.h>
#include <gtkmm/stockitem.h>

#include <exception>
#include <list>


namespace
{

#include <pixmaps/stockimages.h>

struct StockIconData
{
  const guint8*         data;
  unsigned int          length;
  Gtk::BuiltinIconSize  size;
};

struct StockItemData
{
  const char*           id;
  const StockIconData*  icons;
  unsigned int          n_icons;
  const char*           label;
};

const StockIconData stock_icon_info[] =
{
  { stock_menu_about, sizeof(stock_menu_about), Gtk::ICON_SIZE_INVALID }
};

const StockIconData stock_icon_save_all[] =
{
  { stock_save_all_16, sizeof(stock_save_all_16), Gtk::ICON_SIZE_MENU    },
  { stock_save_all_24, sizeof(stock_save_all_24), Gtk::ICON_SIZE_INVALID }
};

const StockItemData regexxer_stock_items[] =
{
  { "regexxer-info",     stock_icon_info,     G_N_ELEMENTS(stock_icon_info),     "_Info"     },
  { "regexxer-save-all", stock_icon_save_all, G_N_ELEMENTS(stock_icon_save_all), "Save _all" }
};


void regexxer_register_stock_items()
{
  const Glib::RefPtr<Gtk::IconFactory> factory = Gtk::IconFactory::create();

  for(unsigned item = 0; item < G_N_ELEMENTS(regexxer_stock_items); ++item)
  {
    const StockItemData& stock = regexxer_stock_items[item];
    Gtk::IconSet icon_set;

    for(unsigned icon = 0; icon < stock.n_icons; ++icon)
    {
      const StockIconData& icon_data = stock.icons[icon];

      Gtk::IconSource source;
      source.set_pixbuf(Gdk::Pixbuf::create_from_inline(icon_data.length, icon_data.data));

      if(icon_data.size != Gtk::ICON_SIZE_INVALID)
      {
        source.set_size(icon_data.size);
        source.set_size_wildcarded(false);
      }

      icon_set.add_source(source);
    }

    const Gtk::StockID stock_id (stock.id);
    factory->add(stock_id, icon_set);
    Gtk::Stock::add(Gtk::StockItem(stock_id, stock.label));
  }

  factory->add_default();
}

void regexxer_set_window_icon()
{
  const char *const regexxer_icon_filename =
      REGEXXER_DATADIR G_DIR_SEPARATOR_S "pixmaps" G_DIR_SEPARATOR_S "regexxer.png";

  try
  {
    std::list< Glib::RefPtr<Gdk::Pixbuf> > icons;
    icons.push_back(Gdk::Pixbuf::create_from_file(regexxer_icon_filename));
    Gtk::Window::set_default_icon_list(icons);
  }
  catch(const Glib::Error& error)
  {
    const Glib::ustring what = error.what();
    g_warning(what.c_str());
  }
}

} // anonymous namespace


int main(int argc, char** argv)
{
  try
  {
    Gtk::Main main_instance (&argc, &argv);

    regexxer_register_stock_items();
    regexxer_set_window_icon();

    Regexxer::MainWindow window;
    Gtk::Main::run(window);
  }
  catch(const Glib::Error& error)
  {
    const Glib::ustring what = error.what();
    g_error("unhandled exception: %s", what.c_str());
  }
  catch(const std::exception& except)
  {
    g_error("unhandled exception: %s", except.what());
  }
  catch(...)
  {
    g_error("unhandled exception: type unknown");
  }

  return 0;
}

