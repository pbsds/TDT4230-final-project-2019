#/usr/bin/env bash
here="$(pwd)"

echo Building log_combined.md...
(
	cat log_part1_intro.md;          echo;
	cat log_part2_hills.md;          echo;
	cat log_part3_models.md;         echo;
	cat log_part4_optimizations.md;  echo;
	cat log_part5_scene.md;          echo;
	cat log_part6_effect.md;         echo;
	cat log_part7_daylight.md;       echo;
) | sed -e "s/ i / I /g" | sed -e "s/ i'm / I'm /g" > log_combined.md

#ENGINE=pdflatex
#ENGINE=lualatex
ENGINE=xelatex

VARIABLES="$VARIABLES --filter pandoc-imagine"
#VARIABLES="$VARIABLES --variable classoption=twocolumn"
VARIABLES="$VARIABLES --variable papersize=a4paper"
VARIABLES="$VARIABLES --table-of-contents"
VARIABLES="$VARIABLES --number-sections"
#VARIABLES="$VARIABLES --number-offset=0,0"
VARIABLES="$VARIABLES --variable links-as-notes=true"

VARIABLES="$VARIABLES --highlight-style=pygments" # the default
#VARIABLES="$VARIABLES --highlight-style=haddock" # kinda nice for python at least
#VARIABLES="$VARIABLES --highlight-style=tango"
#VARIABLES="$VARIABLES --highlight-style=espresso"
#VARIABLES="$VARIABLES --highlight-style=zenburn"
#VARIABLES="$VARIABLES --highlight-style=kate"
#VARIABLES="$VARIABLES --highlight-style=monochrome"
#VARIABLES="$VARIABLES --highlight-style=breezedark"

ls -1 *.md | grep -v "part" |
( while read source; do
	(
	
	base="$(basename $source)"
	dest="$(echo $base | rev | cut -c4- | rev)_out"
	
	cd "$(dirname $source)"
	
	echo "Converting $source into $(dirname $source)/${dest}.pdf ..."
	pandoc "$base" --pdf-engine="$ENGINE" $VARIABLES -o "$dest.pdf" 
	
	) &
done 
wait )
