this heap id is generated by sca tools and
those parameters is passed by boot args(mboot parse mmap)

vdec dip mfe xc... will use those heaps
and those heaps has been accessed not by TLB
at meanwhile, miu protection will work for LX range change.

ipa_cma implement with CMA & MIU protection
