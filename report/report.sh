rm -f *.aux *.bbl *.blg *.log *.out *.pdf
pdflatex --halt-on-error report.tex > /dev/null
pdflatex --halt-on-error report.tex > /dev/null
gs -sDEVICE=pdfwrite -dPrinted=false -dCompatibilityLevel=1.4 -dQUIET -dBATCH -dNOPAUSE -sOutputFile=_report.pdf report.pdf
mv _report.pdf report.pdf
