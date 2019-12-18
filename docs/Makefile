# Makefile to create PDF documents from LaTeX-Files
# Needed software packages: pdflatex
# License: CC BY-NC-SA

PROJECT=dokumentace
PRESENTATION=prezentace
TEX=pdflatex

all: docs presentation

docs:
	$(TEX) $(PROJECT).tex
	$(TEX) $(PROJECT).tex

presentation:
	$(TEX) $(PRESENTATION).tex
	$(TEX) $(PRESENTATION).tex

clean:
	rm -f *.toc *.aux *.log *.dvi *.ps *.lot *.out *.znd *.zdx

cleanall:
	rm -f *.toc *.aux *.log *.dvi *.ps *.lot *.out *.znd *.zdx *.pdf
