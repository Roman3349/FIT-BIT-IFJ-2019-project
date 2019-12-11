# Makefile to create PDF documents from LaTeX-Files
# Needed software packages: pdflatex
# License: CC BY-NC-SA

PROJECT=dokumentace
TEX=pdflatex

all:
	$(TEX) $(PROJECT).tex
	$(TEX) $(PROJECT).tex
 clean:
	rm -f *.toc *.aux *.log *.dvi *.ps *.lot *.out *.znd *.zdx

cleanall:
	rm -f *.toc *.aux *.log *.dvi *.ps *.lot *.out *.znd *.zdx *.pdf
