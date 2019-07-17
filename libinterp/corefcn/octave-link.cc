/*

Copyright (C) 2013-2019 John W. Eaton
Copyright (C) 2011-2019 Jacob Dawid
Copyright (C) 2011-2019 John P. Swensen

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "builtin-defun-decls.h"
#include "cmd-edit.h"
#include "defun.h"
#include "interpreter.h"
#include "interpreter-private.h"
#include "octave-link.h"
#include "oct-env.h"
#include "oct-mutex.h"
#include "ovl.h"
#include "pager.h"
#include "syminfo.h"

static int
octave_readline_hook (void)
{
  octave_link& olnk = octave::__get_octave_link__ ("octave_readline_hook");

  olnk.process_events ();

  return 0;
}

octave_link::octave_link (void)
  : instance (nullptr), event_queue_mutex (new octave::mutex ()),
    gui_event_queue (), debugging (false), link_enabled (true)
{
  octave::command_editor::add_event_hook (octave_readline_hook);
}

octave_link::~octave_link (void)
{
  delete event_queue_mutex;
}

// OBJ should be an object of a class that is derived from the base
// class octave_link, or 0 to disconnect the link.  It is the
// responsibility of the caller to delete obj.

void
octave_link::connect_link (octave_link_events *obj)
{
  if (obj && instance)
    error ("octave_link is already linked!");

  instance = obj;
}

octave_link_events *
octave_link::disconnect_link (bool delete_instance)
{
  if (delete_instance)
    {
      delete instance;
      instance = nullptr;
      return nullptr;
    }
  else
    {
      octave_link_events *retval = instance;
      instance = nullptr;
      return retval;
    }
}

void
octave_link::process_events (bool disable_flag)
{
  if (enabled ())
    {
      if (disable_flag)
        disable ();

      event_queue_mutex->lock ();

      gui_event_queue.run ();

      event_queue_mutex->unlock ();
    }
}

void
octave_link::discard_events (void)
{
  if (enabled ())
    {
      event_queue_mutex->lock ();

      gui_event_queue.discard ();

      event_queue_mutex->unlock ();
    }
}

void
octave_link::set_workspace (void)
{
  if (enabled ())
    {
      octave::tree_evaluator& tw
        = octave::__get_evaluator__ ("octave_link::set_workspace");

      instance->do_set_workspace (tw.at_top_level (), debugging,
                                  tw.get_symbol_info (), true);
    }
}

DEFMETHOD (__octave_link_enabled__, interp, , ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_enabled__ ()
Undocumented internal function.
@end deftypefn */)
{
  octave_link& olnk = interp.get_octave_link ();

  return ovl (olnk.enabled ());
}

DEFMETHOD (__octave_link_edit_file__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_edit_file__ (@var{file})
Undocumented internal function.
@end deftypefn */)
{
  octave_value retval;

  octave_link& olnk = interp.get_octave_link ();

  if (args.length () == 1)
    {
      std::string file = args(0).xstring_value ("first argument must be filename");

      octave::flush_stdout ();

      retval = olnk.edit_file (file);
    }
  else if (args.length () == 2)
    {
      std::string file = args(0).xstring_value ("first argument must be filename");

      octave::flush_stdout ();

      retval = olnk.prompt_new_edit_file (file);
    }

  return retval;
}

DEFMETHOD (__octave_link_question_dialog__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_question_dialog__ (@var{msg}, @var{title}, @var{btn1}, @var{btn2}, @var{btn3}, @var{default})
Undocumented internal function.
@end deftypefn */)
{
  octave_value retval;

  if (args.length () == 6)
    {
      std::string msg = args(0).xstring_value ("invalid arguments");
      std::string title = args(1).xstring_value ("invalid arguments");
      std::string btn1 = args(2).xstring_value ("invalid arguments");
      std::string btn2 = args(3).xstring_value ("invalid arguments");
      std::string btn3 = args(4).xstring_value ("invalid arguments");
      std::string btndef = args(5).xstring_value ("invalid arguments");

      octave::flush_stdout ();

      octave_link& olnk = interp.get_octave_link ();

      retval = olnk.question_dialog (msg, title, btn1, btn2, btn3, btndef);
    }

  return retval;
}

DEFMETHOD (__octave_link_file_dialog__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_file_dialog__ (@var{filterlist}, @var{title}, @var{filename}, @var{size} @var{multiselect}, @var{pathname})
Undocumented internal function.
@end deftypefn */)
{
  if (args.length () != 6)
    return ovl ();

  octave_value_list retval (3);

  const Array<std::string> flist = args(0).cellstr_value ();
  std::string title = args(1).string_value ();
  std::string filename = args(2).string_value ();
  Matrix pos = args(3).matrix_value ();
  std::string multi_on = args(4).string_value (); // on, off, create
  std::string pathname = args(5).string_value ();

  octave_idx_type nel;

  octave_link::filter_list filter_lst;

  for (octave_idx_type i = 0; i < flist.rows (); i++)
    filter_lst.push_back (std::make_pair (flist(i,0),
                                          (flist.columns () > 1
                                           ? flist(i,1) : "")));

  octave::flush_stdout ();

  octave_link& olnk = interp.get_octave_link ();

  std::list<std::string> items_lst
    = olnk.file_dialog (filter_lst, title, filename, pathname, multi_on);

  nel = items_lst.size ();

  // If 3, then retval is filename, directory, and selected index.
  if (nel <= 3)
    {
      if (items_lst.front ().empty ())
        retval = ovl (octave_value (0.), octave_value (0.), octave_value (0.));
      else
        {
          int idx = 0;
          for (auto& str : items_lst)
            {
              if (idx != 2)
                retval(idx++) = str;
              else
                retval(idx++) = atoi (str.c_str ());
            }
        }
    }
  else
    {
      // Multiple files.
      nel -= 2;
      Cell items (dim_vector (1, nel));

      auto it = items_lst.begin ();

      for (int idx = 0; idx < nel; idx++, it++)
        items.xelem (idx) = *it;

      retval = ovl (items, *it++, atoi (it->c_str ()));
    }

  return retval;
}

DEFMETHOD (__octave_link_list_dialog__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_list_dialog__ (@var{list}, @var{mode}, @var{size}, @var{initial}, @var{name}, @var{prompt}, @var{ok_string}, @var{cancel_string})
Undocumented internal function.
@end deftypefn */)
{
  if (args.length () != 8)
    return ovl ();

  Cell list = args(0).cell_value ();
  const Array<std::string> tlist = list.cellstr_value ();
  octave_idx_type nel = tlist.numel ();
  std::list<std::string> list_lst;
  for (octave_idx_type i = 0; i < nel; i++)
    list_lst.push_back (tlist(i));

  std::string mode = args(1).string_value ();

  Matrix size_matrix = args(2).matrix_value ();
  int width = size_matrix(0);
  int height = size_matrix(1);

  Matrix initial_matrix = args(3).matrix_value ();
  nel = initial_matrix.numel ();
  std::list<int> initial_lst;
  for (octave_idx_type i = 0; i < nel; i++)
    initial_lst.push_back (initial_matrix(i));

  std::string name = args(4).string_value ();
  list = args(5).cell_value ();
  const Array<std::string> plist = list.cellstr_value ();
  nel = plist.numel ();
  std::list<std::string> prompt_lst;
  for (octave_idx_type i = 0; i < nel; i++)
    prompt_lst.push_back (plist(i));
  std::string ok_string = args(6).string_value ();
  std::string cancel_string = args(7).string_value ();

  octave::flush_stdout ();

  octave_link& olnk = interp.get_octave_link ();

  std::pair<std::list<int>, int> result
    = olnk.list_dialog (list_lst, mode, width, height, initial_lst,
                        name, prompt_lst, ok_string, cancel_string);

  std::list<int> items_lst = result.first;
  nel = items_lst.size ();
  Matrix items (dim_vector (1, nel));
  octave_idx_type i = 0;
  for (const auto& int_el : items_lst)
    items.xelem(i++) = int_el;

  return ovl (items, result.second);
}

DEFMETHOD (__octave_link_input_dialog__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_input_dialog__ (@var{prompt}, @var{title}, @var{rowscols}, @var{defaults})
Undocumented internal function.
@end deftypefn */)
{
  if (args.length () != 4)
    return ovl ();

  Cell prompt = args(0).cell_value ();
  Array<std::string> tmp = prompt.cellstr_value ();
  octave_idx_type nel = tmp.numel ();
  std::list<std::string> prompt_lst;
  for (octave_idx_type i = 0; i < nel; i++)
    prompt_lst.push_back (tmp(i));

  std::string title = args(1).string_value ();

  Matrix rc = args(2).matrix_value ();
  nel = rc.rows ();
  std::list<float> nr;
  std::list<float> nc;
  for (octave_idx_type i = 0; i < nel; i++)
    {
      nr.push_back (rc(i,0));
      nc.push_back (rc(i,1));
    }

  Cell defaults = args(3).cell_value ();
  tmp = defaults.cellstr_value ();
  nel = tmp.numel ();
  std::list<std::string> defaults_lst;
  for (octave_idx_type i = 0; i < nel; i++)
    defaults_lst.push_back (tmp(i));

  octave::flush_stdout ();

  octave_link& olnk = interp.get_octave_link ();

  std::list<std::string> items_lst
    = olnk.input_dialog (prompt_lst, title, nr, nc, defaults_lst);

  nel = items_lst.size ();
  Cell items (dim_vector (nel, 1));
  octave_idx_type i = 0;
  for (const auto& str_el : items_lst)
    items.xelem(i++) = str_el;

  return ovl (items);
}


DEFMETHOD (__octave_link_named_icon__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_dialog_icons__ (@var{icon_name})
Undocumented internal function.
@end deftypefn */)
{
  uint8NDArray retval;

  if (args.length () > 0)
    {
      std::string icon_name = args(0).xstring_value ("invalid arguments");

      octave_link& olnk = interp.get_octave_link ();

      retval = olnk.get_named_icon (icon_name);
    }

  return ovl (retval);
}

DEFMETHOD (__octave_link_show_preferences__, interp, , ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_show_preferences__ ()
Undocumented internal function.
@end deftypefn */)
{
  octave_link& olnk = interp.get_octave_link ();

  return ovl (olnk.show_preferences ());
}

DEFMETHOD (__octave_link_gui_preference__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_gui_preference__ ()
Undocumented internal function.
@end deftypefn */)
{
  std::string key;
  std::string value = "";

  if (args.length () >= 1)
    key = args(0).string_value();
  else
    error ("__octave_link_gui_preference__: "
           "first argument must be the preference key");

  if (args.length () >= 2)
    value = args(1).string_value();

  if (octave::application::is_gui_running ())
    {
      octave_link& olnk = interp.get_octave_link ();

      return ovl (olnk.gui_preference (key, value));
    }
  else
    return ovl (value);
}

DEFMETHOD (__octave_link_file_remove__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_file_remove__ ()
Undocumented internal function.
@end deftypefn */)
{
  std::string old_name, new_name;

  if (args.length () == 2)
    {
      old_name = args(0).string_value();
      new_name = args(1).string_value();
    }
  else
    error ("__octave_link_file_remove__: "
           "old and new name expected as arguments");

  octave_link& olnk = interp.get_octave_link ();

  olnk.file_remove (old_name, new_name);

  return ovl ();
}

DEFMETHOD (__octave_link_file_renamed__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_file_renamed__ ()
Undocumented internal function.
@end deftypefn */)
{
  bool load_new;

  if (args.length () == 1)
    load_new = args(0).bool_value();
  else
    error ("__octave_link_file_renamed__: "
           "first argument must be boolean for reload new named file");

  octave_link& olnk = interp.get_octave_link ();

  olnk.file_renamed (load_new);

  return ovl ();
}

DEFMETHOD (openvar, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} openvar (@var{name})
Open the variable @var{name} in the graphical Variable Editor.
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  if (! args(0).is_string ())
    error ("openvar: NAME must be a string");

  std::string name = args(0).string_value ();

  if (! (Fisguirunning ())(0).is_true ())
    warning ("openvar: GUI is not running, can't start Variable Editor");
  else
    {
      octave_value val = interp.varval (name);

      if (val.is_undefined ())
        error ("openvar: '%s' is not a variable", name.c_str ());

      octave_link& olnk = interp.get_octave_link ();

      olnk.edit_variable (name, val);
    }

  return ovl ();
}

/*
%!error openvar ()
%!error openvar ("a", "b")
%!error <NAME must be a string> openvar (1:10)
*/

DEFMETHOD (__octave_link_show_doc__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_show_doc__ (@var{filename})
Undocumented internal function.
@end deftypefn */)
{
  std::string file;

  if (args.length () >= 1)
    file = args(0).string_value();

  octave_link& olnk = interp.get_octave_link ();

  return ovl (olnk.show_doc (file));
}

DEFMETHOD (__octave_link_register_doc__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_register_doc__ (@var{filename})
Undocumented internal function.
@end deftypefn */)
{
  std::string file;

  if (args.length () >= 1)
    file = args(0).string_value();

  octave_link& olnk = interp.get_octave_link ();

  return ovl (olnk.register_doc (file));
}

DEFMETHOD (__octave_link_unregister_doc__, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} __octave_link_unregister_doc__ (@var{filename})
Undocumented internal function.
@end deftypefn */)
{
  std::string file;

  if (args.length () >= 1)
    file = args(0).string_value();

  octave_link& olnk = interp.get_octave_link ();

  return ovl (olnk.unregister_doc (file));
}
