# FA4 - A 'grep' like tool

The letters `FA`, stand for `Find All` words, or phrase, in a file or files...

This utility started life in assembler on an Intel 8086 processor, at just 4.77 MHz! I wanted it to be FAST! That was a long time ago ;=)) a different era in PC development...

It progressed through 16-bit Windows, and was converted, well re-written in C, and got the '4' added...

The current is WIN32, and it also compiles in WIN64, when/if that is available...

Once in a repo I cloned it into 64-bit Ubuntu (14.04 -'-bit), and did a unix port.

To me, it is a serious `competitor` to unix venerable grep ;=)) different features, but the same idea - scan a file or files for each instance of a word, or phrase, with lots of options, and report what is found...

Due to the emphasis of `do it quickly` it memory maps the file contents into this app's space, and uses it as a byte buffer of the file...

Enjoy.

Geoff.  
20150828 - 20140808

; eof
