/* Test the REXX functions in Rxmous.DLL */

say 'Assuming you put rxmous.dll somewhere in LIBPATH, should now load.'

rr = rxfuncadd( 'MousInit', 'Rxmous', 'MousInit')
say 'rxfuncadd MousInit rc='rr
call MousInit /* may find it useful to comment out during testing, */
  /* for error when get dlls stuck in memory with same function name */

do forever
  p= ClickPos('D')      /* test / report doublclick */
  if p <> '0 0 0 0' then call charout, ' Pos:"'||p||'" '
  else call charout, '.'
  call syssleep 0.2
/*  call MousClos */
end
exit

