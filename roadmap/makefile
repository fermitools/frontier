all: pdf

pdf: TheNewFroNtier.pdf

SOURCES := TheNewFroNtier.tex \
	analysis.tex \
	architectural.tex \
	deployment.tex \
	development.tex \
	introduction.tex \
	release.tex \
	requirements.tex \
	technology.tex  \
	version.tex

GRAPHICS := architecture.png

TheNewFroNtier.pdf:  ${SOURCES} ${GRAPHICS}
	texify --silent --pdf --run-viewer TheNewFroNtier.tex

clean:
	rm -f *.aux
	rm -f *.toc
	rm -f *.out
	rm -f *.log
	rm -f *~

clobber: clean
	rm -f TheNewFroNtier.pdf
