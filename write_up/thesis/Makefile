TARGET_FILE_NAME=dissertation_main_fict
LATEX_PROCESSOR=pdflatex
LATEX_FLAGS=-shell-escape -interaction=nonstopmode --jobname="$(TARGET_FILE_NAME)" $(TARGET_FILE_NAME).tex
PDF_LATEX_COMMAND=$(LATEX_PROCESSOR) $(LATEX_FLAGS)

ALL: content
.PHONY: ALL content references clean

content:
	$(PDF_LATEX_COMMAND) && $(PDF_LATEX_COMMAND)

references:
	$(PDF_LATEX_COMMAND) && \
		bibtex $(TARGET_FILE_NAME) &&  \
		$(PDF_LATEX_COMMAND) && \
		$(PDF_LATEX_COMMAND)

clean:
	./clean.sh