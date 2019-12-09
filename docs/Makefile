# Makefile to create PDF documents from LaTeX-Files
# Needed software packages: pdflatex
# License: CC BY-NC-SA

PROJECT=dokumentace
TEX=latex

all:
	$(TEX) $(PROJECT).tex
	$(TEX) $(PROJECT).tex
	dvips -t a4 $(PROJECT).dvi
	ps2pdf $(PROJECT).ps $(PROJECT).pdf
 clean:
	rm -f *.toc *.aux *.log *.dvi *.ps *.lot *.out *.znd *.zdx

cleanall:
	rm -f *.toc *.aux *.log *.dvi *.ps *.lot *.out *.znd *.zdx *.pdf
