del selfsigned.key
del selfsigned.cer
makekeys -cert -expdays 9000 -password DefaultPassword -len 1024 -dname "CN=Support CO=CA OU=Development OR=NS Basic Corporation EM=support@nsbasic.com" selfsigned.key selfsigned.cer