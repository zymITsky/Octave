%!test
%! filename = "save-text.var";
%! s.("a-b") = "bad fieldname";
%! unwind_protect
%!   save ("-text", filename, "s");
%!   filevar = load (filename);
%!   assert (filevar.s, s);
%! unwind_protect_cleanup
%!   sts = unlink (filename);
%! end_unwind_protect
