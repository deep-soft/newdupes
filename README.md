Introduction
-------------------------------------------------------------------------------
jdupes is a program for identifying and taking actions upon duplicate files
such as deleting, hard linking, symlinking, and block-level deduplication
(also known as "dedupe" or "reflink"). It is faster than most other duplicate
scanners. It prioritizes data safety over performance while also giving
expert users access to advanced (and sometimes dangerous) features.

Please consider financially supporting continued development of jdupes using
the links on my home page (Ko-fi, PayPal, SubscribeStar, etc.):

https://www.jodybruchon.com/


Rewrite
-------------------------------------------------------------------------------
This is a rewrite from scratch. All jdupes code has been tossed out and will
be rebuilt from the ground up with a much better underlying architecture and
far better performance on modern hardware. Code repository is currently at:

https://github.com/jbruchon/newdupes

This code is not considered ready for use, nor safe in any way.
