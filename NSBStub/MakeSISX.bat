rem MakeSISX

rem Uses the info in pkg file to make a .sis, 
rem then signs it to make a .sisx

tools\makesis NSBStub.pkg

tools\signsis -s -v NSBStub.sis NSBStubSigned.sisx NSBStub.cer NSBStub.key george
del NSBStub.sis