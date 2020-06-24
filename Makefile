# Makefile for use with KeywordsTxtGenerator

DOXYGEN := doxygen
DOXYFILE := $(PWD)/keywords_txt_generator.doxy
XSLTPROC := xsltproc
TRANSFORMATION := $(PWD)/doxygen2keywords.xsl

keywords.txt: xml/index.xml
	$(XSLTPROC) $(TRANSFORMATION) $< > $@

xml/index.xml:
	$(DOXYGEN) $(DOXYFILE)

%/xml/index.xml:
	(cd $* ; $(DOXYGEN) $(DOXYFILE))

%/keywords.txt: %/xml/index.xml
	$(XSLTPROC) $(TRANSFORMATION) $< > $@

clean:
	rm -rf xml keywords.txt

.PHONY: clean
