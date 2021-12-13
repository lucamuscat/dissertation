content:
	pdflatex -shell-escape -interaction=nonstopmode main_layout.tex && pdflatex -shell-escape -interaction=nonstopmode main_layout.tex

references:
	pdflatex -shell-escape -interaction=nonstopmode main_layout.tex
	bibtex main_layout
	pdflatex -shell-escape -interaction=nonstopmode main_layout.tex
	pdflatex -shell-escape -interaction=nonstopmode main_layout.tex
