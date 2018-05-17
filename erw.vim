" Vim syntax file
" Language: Erwall
" Maintainer: Erik Wallström
" Latest Revision: 16 May 2018

if exists("b:current_syntax")
	finish
endif

syn keyword Todo TODO FIXME NOTE XXX
syn keyword Keyword type func let mut if elseif else return cast while defer 
	\ unsafe
syn match Type "\u[[:alnum:]_]\+"
syn match Variable "\l[[:alnum:]_]\+"
syn match Number "\d\+"
syn match Float "\d\+\.\d\+"
syn match Boolean "true\|false"
syn match Function "\<[[:lower:]\d_]\+\ze("
syn match Function "\<[[:lower:]\d_]\+\ze\s*:\s*("
syn region String start=+"+ end=+"+
syn region Character start=+'+ end=+'+
syn region Comment start="# " end="$" end=";" contains=Todo
syn region Comment start="#\[" end="#\]" contains=Comment contains=Todo
syn match PreProc "@\a[[:alnum:]_]*"
