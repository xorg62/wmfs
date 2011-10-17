" Vim syntax file
" Language: WMFS Configuration file
" Maintainer: David Delassus <david.jose.delassus@gmail.com>
" Latest Revision: 17 October 2011

if exists("b:current_syntax")
     finish
endif

syn case ignore

" Keywords
syn keyword wmfsBlockTheme font
syn keyword wmfsBlockThemeBars bars_width bars_fg bars_bg
syn keyword wmfsBlockThemeTags tags_normal_fg tags_normal_bg tags_sel_fg tags_sel_bg tags_border_color tags_border_width
syn keyword wmfsBlockThemeClient client_normal_fg client_normal_bg client_sel_fg client_sel_bg frame_bg client_titlebar_width client_border_width

syn keyword wmfsBlockBar position screen elements theme

syn keyword wmfsBlockTag screen name

syn keyword wmfsBlockKey mod key func cmd

syn keyword wmfsTodo contained TODO FIXME XXX NOTE

" Matches
syn match wmfsNumber /\S\@<!\d\+\(\.\d\+\)\?\(\S\@!\|}\@=\)/ nextgroup=wmfsNumber
syn match wmfsNumber '\d\+' contained display
syn match wmfsNumber '[-+]\d\+' contained display

" Regions
syn region wmfsComment start="#" end="$" contains=wmfsTodo
syn region wmfsString start='"' end='"' contains=CONTAINED
syn region wmfsSection start="\[" end="\]"
syn region wmfsList start="{" end="}" contains=wmfsNumber,wmfsString

let b:current_syntax = "wmfs"

hi def link wmfsTodo          Todo
hi def link wmfsComment       Comment
hi def link wmfsSection       Statement
hi def link wmfsList          Statement
hi def link wmfsNumber        Number
hi def link wmfsString        String

hi def link wmfsBlockTheme         Identifier
hi def link wmfsBlockThemeBars     Identifier
hi def link wmfsBlockThemeTags     Identifier
hi def link wmfsBlockThemeClient   Identifier
hi def link wmfsBlockBar           Identifier
hi def link wmfsBlockTag           Identifier
hi def link wmfsBlockKey           Identifier
